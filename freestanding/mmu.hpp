#ifndef __fs_mmu_hpp__
#define __fs_mmu_hpp__

/*
 * Minimal AArch64 MMU bring-up for the BCM2837. Identity map (VA == PA) with a
 * 4 KB granule via TTBR0_EL1:
 *   0x00000000..0x3EFFFFFF  Normal, cacheable, inner-shareable (RAM)
 *   0x3F000000..0x3FFFFFFF  Device-nGnRnE               (BCM peripherals)
 *   0x40000000..0x7FFFFFFF  Device-nGnRnE               (per-core ARM local)
 *
 * "Inner-shareable Normal" is what makes the caches coherent across the four
 * cores — a prerequisite for SMP. Device memory keeps MMIO strongly-ordered.
 */

/// Build the shared translation tables. Call ONCE (core 0), with the MMU off, so
/// the writes land in RAM before any cache is enabled.
void mmu_build_tables();

/// Program MAIR/TCR/TTBR0 and set SCTLR_EL1.{M,C,I}. Call on every core after
/// mmu_build_tables() has completed.
void mmu_enable();

#endif /*__fs_mmu_hpp__*/
