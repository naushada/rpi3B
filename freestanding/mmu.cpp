#include "mmu.hpp"
#include <cstdint>

/*
 * 39-bit VA (T0SZ=25), 4 KB granule → translation starts at level 1 (each L1
 * entry maps 1 GB). We use:
 *   L1[0] -> a level-2 table (512 × 2 MB) covering 0..1 GB with mixed attrs;
 *   L1[1] -> a 1 GB block, Device, covering 1..2 GB (the ARM-local block lives
 *            at 0x40000000).
 */
namespace {
    constexpr std::uint64_t k2MB = 0x200000ull;
    constexpr std::uint64_t k1GB = 0x40000000ull;

    // MAIR indices.
    constexpr std::uint64_t ATTR_DEVICE = 0;   // Device-nGnRnE (MAIR attr 0x00)
    constexpr std::uint64_t ATTR_NORMAL = 1;   // Normal WB      (MAIR attr 0xFF)

    // Descriptor bits.
    constexpr std::uint64_t DESC_BLOCK = 0b01; // block (L1/L2)
    constexpr std::uint64_t DESC_TABLE = 0b11; // table pointer
    constexpr std::uint64_t AF         = 1ull << 10;          // Access Flag
    constexpr std::uint64_t SH_INNER   = 0b11ull << 8;        // inner shareable
    constexpr std::uint64_t ATTRIDX(std::uint64_t i) { return i << 2; }

    // Peripheral window starts at 0x3F000000 → L2 index 0x3F000000 / 2MB = 504.
    constexpr unsigned kPeriphIndex = static_cast<unsigned>(0x3F000000ull / k2MB);

    alignas(4096) std::uint64_t l1[512];
    alignas(4096) std::uint64_t l2[512];

    std::uint64_t normal_block(std::uint64_t pa) {
        return pa | AF | SH_INNER | ATTRIDX(ATTR_NORMAL) | DESC_BLOCK;
    }
    std::uint64_t device_block(std::uint64_t pa) {
        return pa | AF | ATTRIDX(ATTR_DEVICE) | DESC_BLOCK;   // SH ignored for Device
    }
}

void mmu_build_tables() {
    for(unsigned i = 0; i < 512; ++i) {
        std::uint64_t pa = static_cast<std::uint64_t>(i) * k2MB;
        l2[i] = (i < kPeriphIndex) ? normal_block(pa) : device_block(pa);
    }

    l1[0] = reinterpret_cast<std::uint64_t>(&l2[0]) | DESC_TABLE;  // 0..1 GB
    l1[1] = device_block(k1GB);                                   // 1..2 GB (ARM local)
    for(unsigned i = 2; i < 512; ++i) l1[i] = 0;
}

void mmu_enable() {
    // MAIR: attr0 = Device-nGnRnE (0x00), attr1 = Normal WB inner/outer (0xFF).
    const std::uint64_t mair = (0xFFull << 8) | (0x00ull << 0);

    // TCR_EL1: T0SZ=25, 4 KB granule, WB-WA walks, inner-shareable, 40-bit PA,
    // TTBR1 walks disabled (EPD1).
    const std::uint64_t tcr =
          (25ull << 0)      // T0SZ  → 39-bit VA
        | (0b01ull << 8)    // IRGN0 = WB WA
        | (0b01ull << 10)   // ORGN0 = WB WA
        | (0b11ull << 12)   // SH0   = inner shareable
        | (0b00ull << 14)   // TG0   = 4 KB
        | (1ull    << 23)   // EPD1  = disable TTBR1 walks
        | (0b010ull << 32); // IPS   = 40-bit PA

    std::uint64_t ttbr0 = reinterpret_cast<std::uint64_t>(&l1[0]);

    __asm__ volatile(
        "msr mair_el1,  %0\n"
        "msr tcr_el1,   %1\n"
        "msr ttbr0_el1, %2\n"
        "dsb ish\n"
        "isb\n"
        "tlbi vmalle1\n"
        "dsb ish\n"
        "isb\n"
        :: "r"(mair), "r"(tcr), "r"(ttbr0) : "memory");

    std::uint64_t sctlr;
    __asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
    sctlr |= (1ull << 0)    // M: MMU enable
          |  (1ull << 2)    // C: data cache
          |  (1ull << 12);  // I: instruction cache
    __asm__ volatile(
        "msr sctlr_el1, %0\n"
        "isb\n"
        :: "r"(sctlr) : "memory");
}
