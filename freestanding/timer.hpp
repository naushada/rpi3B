#ifndef __fs_timer_hpp__
#define __fs_timer_hpp__

#include <cstdint>

/* ARMv8-A generic-timer accessors (EL1 uses the physical non-secure timer,
 * CNTP_*). Reachable from EL1 because boot.S set CNTHCTL_EL2.EL1PC{T,}EN. */

inline std::uint64_t timer_freq() {
    std::uint64_t v;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(v));
    return v;
}

/// Fire an interrupt after `ticks` counter ticks; writing TVAL re-arms and
/// deasserts a currently-pending timer interrupt.
inline void timer_set_tval(std::uint64_t ticks) {
    __asm__ volatile("msr cntp_tval_el0, %0" : : "r"(ticks));
}

/// CNTP_CTL_EL0 = ENABLE(1), IMASK(0): counting and interrupt unmasked.
inline void timer_enable() {
    std::uint64_t ctl = 1;
    __asm__ volatile("msr cntp_ctl_el0, %0" : : "r"(ctl));
}

inline void irq_unmask() { __asm__ volatile("msr daifclr, #2"); }

inline std::uint64_t current_el() {
    std::uint64_t v;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(v));
    return v >> 2;
}

#endif /*__fs_timer_hpp__*/
