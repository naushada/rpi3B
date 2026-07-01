#ifndef __fs_runtime_hpp__
#define __fs_runtime_hpp__

class IRQ;

/// Point the C exception handlers at the IRQ controller kmain owns, so
/// irq_handler_c() can drive IRQ::dispatch(). Called once from kmain.
void set_active_irq(IRQ* irq);

/// Symbol exported by vectors.S — the 2 KB-aligned AArch64 vector table.
extern "C" unsigned char vector_table[];

/// C exception entry points invoked from vectors.S.
extern "C" void irq_handler_c();
extern "C" void bad_exception_c(unsigned long kind,
                                unsigned long esr,
                                unsigned long elr);

/// Secondary-core entry (cores 1..3), branched to from boot.S.
extern "C" void ksecondary();

/// This core's id from MPIDR_EL1 (0..3).
inline unsigned core_id() {
    unsigned long v;
    __asm__ volatile("mrs %0, mpidr_el1" : "=r"(v));
    return static_cast<unsigned>(v & 0xFF);
}

/// Set this core's VBAR_EL1 (each core has its own). Core 0 uses the model API
/// IRQ::install_vector_table; secondaries use this directly.
inline void vbar_el1_set(const void* table) {
    __asm__ volatile("msr vbar_el1, %0\n isb" : : "r"(table) : "memory");
}

/// The timer ISR (registered once by core 0; runs on whichever core took the
/// IRQ — it reads MPIDR to label its output and re-arm its own timer).
void timer_isr();
/// Route + start this core's physical-timer IRQ (CORE_TIMER_IRQCNTL[core]).
void core_timer_start(unsigned core);
/// Release cores 1..3 from the armstub spin-table (Pi 3 mailboxes 0xe0/e8/f0).
void smp_release_secondaries();

#endif /*__fs_runtime_hpp__*/
