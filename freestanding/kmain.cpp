/*
 * kmain — core 0 init for the AArch64 IVT / IRQ / MMU / SMP demo on (QEMU-
 * emulated) BCM2837:
 *
 *   1. init the PL011 console;
 *   2. enable the MMU (RAM cacheable inner-shareable, MMIO device) — the
 *      inner-shareable attribute is what makes the four cores cache-coherent;
 *   3. construct IRQ (overlays the real legacy controller @0x3F00B200 and the
 *      per-core ARM-local block @0x40000000) and install VBAR_EL1;
 *   4. register the physical-timer handler once (shared dispatch table);
 *   5. release cores 1..3 (they run ksecondary in smp.cpp);
 *   6. arm core 0's own timer and WFI.
 *
 * Each core takes its own timer IRQ through the 0x280 vector → irq_handler_c →
 * IRQ::dispatch(core) → the registered handler.
 */
#include <cstdint>

#include "uart.hpp"
#include "timer.hpp"
#include "mmu.hpp"
#include "runtime.hpp"
#include "interrupt.hpp"
#include "memory_map.hpp"

extern "C" void kmain() {
    using Src = BCM2837::ArmLocalRegisterAddress::Source;

    uart_init();
    uart_puts("\n=== bcm2837 bare-metal: AArch64 IVT / IRQ / MMU / SMP ===\n");
    uart_puts("core 0 running at EL"); uart_dec(current_el()); uart_putc('\n');

    // MMU on: build the shared tables while caches are off (so they reach RAM),
    // then enable. Do this before any shared state is published to secondaries.
    mmu_build_tables();
    mmu_enable();
    uart_puts("MMU enabled (RAM cacheable inner-shareable, MMIO device)\n");

    IRQ irq;                                  // overlays real MMIO
    set_active_irq(&irq);
    irq.install_vector_table(vector_table);   // core 0 VBAR_EL1 (model API)
    uart_puts("VBAR_EL1 -> vector_table @ ");
    uart_hex(reinterpret_cast<std::uint64_t>(vector_table));
    uart_putc('\n');

    // Register the physical non-secure timer line once; the shared dispatch
    // table is read by every core's dispatch(core).
    irq.register_handler(IVT::kLocalBase + Src::CNTPNSIRQ, &timer_isr);
    uart_puts("CNTFRQ="); uart_dec(timer_freq());
    uart_puts("  (~5 IRQs/s per core)\n");

    // From here, output can come from any core — take the console lock.
    uart_enable_lock();

    // Wake cores 1..3 (spin-table). They set up their own MMU/VBAR/timer.
    smp_release_secondaries();

    // Core 0's own timer.
    core_timer_start(0);
    uart_lock();
    uart_puts("[core 0] online\n");
    uart_unlock();

    for(;;) __asm__ volatile("wfi");
}
