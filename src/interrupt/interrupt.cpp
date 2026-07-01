#ifndef __interrupt_cpp__
#define __interrupt_cpp__

#include "interrupt.hpp"

namespace {
    /// Index of the lowest set bit, or -1 if none. Used to pick the
    /// lowest-numbered pending IRQ from a legacy pending register.
    inline int lowest_set_bit(std::uint32_t v) {
        if(v == 0U) return -1;
        int n = 0;
        while((v & 1U) == 0U) { v >>= 1; ++n; }
        return n;
    }
}

IRQ* IRQ::s_active = nullptr;

/**
 * @brief
 *      IRQs 0..31 are controlled by the *_IRQs_1 register (bit = irq number);
 *      IRQs 32..63 by the *_IRQs_2 register (bit = irq number - 32). The earlier
 *      code had the register selection inverted and used out-of-range shifts
 *      (see docs/DRIVER_REVIEW.md §2.4).
*/
void IRQ::enable(irq_number irqNumber) {
    if(irqNumber < 32) {
        m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::Enable_IRQs_1] |= (1U << irqNumber);
    } else {
        m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::Enable_IRQs_2] |= (1U << (irqNumber - 32));
    }
}

void IRQ::disable(irq_number irqNumber) {
    if(irqNumber < 32) {
        m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::Disable_IRQs_1] |= (1U << irqNumber);
    } else {
        m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::Disable_IRQs_2] |= (1U << (irqNumber - 32));
    }
}

/**
 * @brief Report whether the interrupt was enabled, by reading back the bit we
 *        set in the Enable_IRQs register. This is side-effect free (the previous
 *        version mutated the read-only pending register during the query) and
 *        has the &/== precedence fixed.
*/
bool IRQ::isEnabled(irq_number number) {
    if(number < 32) {
        return(((m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::Enable_IRQs_1] >> number) & 1U) == 1U);
    }
    return(((m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::Enable_IRQs_2] >> (number - 32)) & 1U) == 1U);
}

/* ---- AArch64 dispatch ---- */

void IRQ::register_handler(irq_number number, IVT::pointerToFn cb) {
    m_ivt.set_handler(number, cb);
}

IVT::pointerToFn IRQ::handler_for(irq_number number) const {
    return(m_ivt.handler(number));
}

/**
 * @brief The work the single architectural IRQ vector delegates to. AArch64 on
 *        the Pi 3 routes interrupts to a core through the ARM-local block: the
 *        four generic-timer lines arrive as their own source bits, and every
 *        peripheral IRQ from the legacy controller is OR'd into a single "GPU"
 *        bit. So: read the per-core source; service a timer line if present,
 *        else decode the legacy pending registers and dispatch the
 *        lowest-numbered pending peripheral IRQ.
*/
void IRQ::dispatch(unsigned core) {
    using Local = BCM2837::ArmLocalRegisterAddress::Register;
    using Src   = BCM2837::ArmLocalRegisterAddress::Source;

    const std::uint32_t source = m_local.m_register[Local::CORE_IRQ_SOURCE0 + core];

    /* Generic-timer lines (CNTPS/CNTPNS/CNTHP/CNTV) → local handler slots. */
    for(std::uint32_t line = Src::CNTPSIRQ; line <= Src::CNTVIRQ; ++line) {
        if(source & (1U << line)) {
            IVT::pointerToFn cb = m_ivt.handler(IVT::kLocalBase + line);
            if(cb) cb();
            return;
        }
    }

    /* Combined peripheral line: decode the legacy controller's pending banks. */
    if(source & (1U << Src::GPU)) {
        const std::uint32_t p1 = m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::IRQ_Pending1];
        int bit = lowest_set_bit(p1);
        if(bit >= 0) {
            IVT::pointerToFn cb = m_ivt.handler(static_cast<std::size_t>(bit));
            if(cb) cb();
            return;
        }
        const std::uint32_t p2 = m_memory.m_register[BCM2837::InterruptRegisterAddress::Register::IRQ_Pending2];
        bit = lowest_set_bit(p2);
        if(bit >= 0) {
            IVT::pointerToFn cb = m_ivt.handler(static_cast<std::size_t>(32 + bit));
            if(cb) cb();
            return;
        }
    }
}

void IRQ::dispatch_trampoline() {
    if(s_active != nullptr) s_active->dispatch(0);
}

/**
 * @brief Install the per-IRQ handler AND point the architectural IRQ vector at
 *        the dispatch trampoline. Unlike the previous version, the IRQNumber is
 *        now meaningful (it keys the dispatch table); the previous code ignored
 *        it and reinterpret_cast a function-pointer slot to an IVT* (a wild
 *        pointer — see docs/DRIVER_REVIEW.md §2.4).
*/
void IRQ::install_IRQHandler(std::uint8_t IRQNumber, IVT::pointerToFn cb) {
    m_ivt.set_handler(IRQNumber, cb);
    s_active = this;
    m_ivt[IVT::Number::IRQ] = &IRQ::dispatch_trampoline;
}

void IRQ::install_FIQHandler(std::uint8_t IRQNumber, IVT::pointerToFn cb) {
    (void)IRQNumber;
    m_ivt[IVT::Number::FIQ] = cb;
}

/**
 * @brief Point VBAR_EL1 at the (2 KB-aligned) vector table. Freestanding only:
 *        the `msr` is emitted under -DINTERRUPT_BAREMETAL; on the host this is a
 *        no-op that records the pointer so tests can read it back. The EL2→EL1
 *        drop the Pi's armstub needs is a runtime-phase concern (see
 *        docs/aarch64-interrupt-model.md).
*/
void IRQ::install_vector_table(const void* table) {
    m_vbar = table;
#ifdef INTERRUPT_BAREMETAL
    asm volatile("msr vbar_el1, %0\n\tisb" : : "r"(table) : "memory");
#endif
}

#endif /*__interrupt_cpp__*/
