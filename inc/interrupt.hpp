#ifndef __interrupt_hpp__
#define __interrupt_hpp__

#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <atomic>

#include "memory_map.hpp"

/**
 * @file interrupt.hpp
 * @brief AArch64 (ARMv8-A) interrupt-vector + IRQ-dispatch model for the
 *        BCM2837 (Raspberry Pi 3B). Host-testable like the rest of the repo:
 *        placement-`new` overlays the controller register blocks and the vector
 *        table either on live MMIO or on a caller buffer for gtest.
 *
 *  AArch64 exception model (vs the classic AArch32 one this code used to model):
 *  exceptions are taken through VBAR_EL1, which points at a **16-entry** table of
 *  **0x80-byte** slots (0x800 total, 2 KB-aligned), grouped 4×4 by source —
 *  {Current EL w/ SP0} {Current EL w/ SPx} {Lower EL AArch64} {Lower EL AArch32},
 *  each group being {Synchronous, IRQ, FIQ, SError}. Each slot holds up to 32
 *  instructions (normally a `b <handler>`), NOT a function pointer — but for a
 *  host-side *model* we store the handler pointer the asm stub would branch to.
 *
 *  The single architectural IRQ vector fans out to many sources. Real dispatch
 *  reads the per-core ARM-local IRQ source (BCM_LOCAL_BASE) and, for the combined
 *  "GPU IRQ" line, the legacy controller's pending registers, then calls the
 *  per-IRQ-number handler registered below. See IRQ::dispatch().
 *
 *  DEFERRED (runtime phase, documented in docs/aarch64-interrupt-model.md): the
 *  freestanding asm vector stub, _start, linker script placing `.vectors`, and
 *  the EL2→EL1 drop the Pi's armstub requires. install_vector_table() emits the
 *  real `msr VBAR_EL1` only under -DINTERRUPT_BAREMETAL; on the host it is a
 *  no-op that records the pointer so tests can read it back.
 */

class IVT {
    public:
        using pointerToFn = void (*)();

        /// The 16 AArch64 exception vectors, in architectural order. The slot
        /// for vector v lives at byte offset `v * 0x80` from VBAR.
        enum class Vector : std::uint8_t {
            CurEL_SP0_Sync,   CurEL_SP0_IRQ,   CurEL_SP0_FIQ,   CurEL_SP0_SError,
            CurEL_SPx_Sync,   CurEL_SPx_IRQ,   CurEL_SPx_FIQ,   CurEL_SPx_SError,
            LowerEL_A64_Sync, LowerEL_A64_IRQ, LowerEL_A64_FIQ, LowerEL_A64_SError,
            LowerEL_A32_Sync, LowerEL_A32_IRQ, LowerEL_A32_FIQ, LowerEL_A32_SError,
            Count
        };

        /// AArch64 vector-table geometry (ARMv8-A): 16 entries × 0x80 bytes,
        /// 0x800 total, VBAR must be 2 KB-aligned.
        static constexpr unsigned kEntryStride = 0x80;
        static constexpr unsigned kTableBytes  = 0x800;
        static constexpr unsigned kTableAlign  = 0x800;

        /// Max software IRQ sources demuxed below the single architectural IRQ
        /// vector: legacy-controller IRQs 0..63 plus a small local-source range
        /// (timer lines) starting at kLocalBase.
        static constexpr std::size_t kMaxHandlers = 72;
        static constexpr std::size_t kLocalBase   = 64;   ///< CNTPNSIRQ → 64+1

        /// Compatibility alias for callers written against the single
        /// architectural IRQ/FIQ slot. A kernel runs handlers at the current EL
        /// on SPx, so these resolve to the Current-EL-SPx group.
        enum Number : std::uint8_t {
            IRQ = static_cast<std::uint8_t>(Vector::CurEL_SPx_IRQ),
            FIQ = static_cast<std::uint8_t>(Vector::CurEL_SPx_FIQ)
        };

        IVT() {}
        ~IVT() {}

        /**
         * @brief Placement-new seam. With no region (the freestanding default)
         *        the vector/dispatch table is backed by static storage in the
         *        image's .bss — a real kernel keeps this software table in RAM
         *        (the *architectural* table lives at VBAR, set separately by
         *        IRQ::install_vector_table). Tests always pass a buffer. A kernel
         *        has a single IVT, so sharing the static backing is intentional.
         * @param nBytes Number of bytes to be allocated
         * @param region is the memory region to be returned if present.
         */
        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                alignas(IVT) static unsigned char storage[sizeof(IVT)];
                return static_cast<void *>(storage);
            }
            return(region);
        }

        /// Architectural vector slot (the handler the asm stub branches to).
        pointerToFn &operator[](Vector v) {
            return(m_slots[static_cast<std::uint8_t>(v)]);
        }
        /// Compatibility overload for the single IRQ/FIQ slot.
        pointerToFn &operator[](Number n) {
            return(m_slots[static_cast<std::uint8_t>(n)]);
        }

        /// Byte offset of a vector's slot within the table — where the asm
        /// `b <handler>` lives. Used by the deferred freestanding stub and tests.
        static constexpr unsigned byte_offset(Vector v) {
            return(static_cast<unsigned>(v) * kEntryStride);
        }

        /* ---- software dispatch table (NOT architectural; the demux the single
         *      IRQ vector branches into — see IRQ::dispatch) ---- */
        pointerToFn handler(std::size_t irq) const {
            return(irq < kMaxHandlers ? m_handlers[irq] : nullptr);
        }
        void set_handler(std::size_t irq, pointerToFn cb) {
            if(irq < kMaxHandlers) m_handlers[irq] = cb;
        }

    private:
        pointerToFn m_slots[static_cast<std::size_t>(Vector::Count)];
        pointerToFn m_handlers[kMaxHandlers];
};

class IVT_Table {
    public:
        IVT_Table() : m_ivt(*new IVT) {}
        template<typename Region>
        IVT_Table(Region region) : m_ivt(*new(region) IVT) {}


    private:
        IVT& m_ivt;
};

class IRQ {
    public:
        using irq_number = std::uint8_t;

        IRQ() : m_memory(*new BCM2837::InterruptRegisterAddress),
                m_ivt(*new IVT),
                m_local(*new BCM2837::ArmLocalRegisterAddress) {}

        /**
         * @brief Overlay the three software/hardware blocks on one contiguous
         *        region (tests): [interrupt registers][IVT][ARM-local registers].
         *        `Region` is a word pointer (uint32_t*), so the offsets below are
         *        in words. Use region_words() to size the buffer.
         */
        template<typename Region>
        IRQ(Region region)
            : m_memory(*new(region) BCM2837::InterruptRegisterAddress),
              m_ivt(*new(region + kIvtWordOffset) IVT),
              m_local(*new(region + kLocalWordOffset) BCM2837::ArmLocalRegisterAddress) {}

        ~IRQ() = default;

        /* ---- legacy controller enable/disable (banks 1/2) ---- */
        void enable(irq_number number);
        void disable(irq_number number);
        bool isEnabled(irq_number number);

        /* ---- AArch64 dispatch: one architectural IRQ vector → many sources --- */
        /// Register a handler for a peripheral IRQ number (0..63) or a local
        /// source (kLocalBase + line). dispatch() routes to it.
        void register_handler(irq_number number, IVT::pointerToFn cb);
        IVT::pointerToFn handler_for(irq_number number) const;
        /// Body the architectural IRQ vector branches to: read the per-core IRQ
        /// source, then the legacy pending registers for the combined GPU line,
        /// and call the lowest-numbered pending source's registered handler.
        void dispatch(unsigned core = 0);

        /* ---- existing API (kept source-compatible for i2c_irq) ---- */
        /// Register `cb` for IRQ `IRQNumber` AND point the architectural IRQ
        /// vector at the dispatch trampoline (so the previously-ignored number is
        /// now meaningful — see docs/DRIVER_REVIEW.md §2.4).
        void install_IRQHandler(std::uint8_t IRQNumber, IVT::pointerToFn cb);
        /// FIQ is single-source: install `cb` directly in the FIQ vector slot.
        void install_FIQHandler(std::uint8_t IRQNumber, IVT::pointerToFn cb);

        /* ---- VBAR_EL1 install (freestanding-gated; host no-op records it) ---- */
        void install_vector_table(const void* table);
        const void* vector_table() const { return m_vbar; }

        /// Region word offsets / total size for placement (used by tests).
        static constexpr std::size_t kIvtWordOffset =
            BCM2837::InterruptRegisterAddress::Register::IRQs_ALL_MAX;
        static constexpr std::size_t kLocalWordOffset =
            kIvtWordOffset + (sizeof(IVT) + sizeof(std::uint32_t) - 1) / sizeof(std::uint32_t);
        static constexpr std::size_t region_words() {
            return kLocalWordOffset
                 + BCM2837::ArmLocalRegisterAddress::Register::ARM_LOCAL_MAX;
        }

    private:
        /// Trampoline the architectural IRQ slot points at; calls s_active->dispatch().
        static void dispatch_trampoline();
        static IRQ* s_active;

        BCM2837::InterruptRegisterAddress&    m_memory;
        IVT&                                  m_ivt;
        BCM2837::ArmLocalRegisterAddress&     m_local;
        const void*                           m_vbar = nullptr;
};

#endif /*__interrupt_hpp__*/
