/*
 * kmain — the freestanding demo that exercises the AArch64 IVT/IRQ model on
 * real (QEMU-emulated) BCM2837 hardware:
 *
 *   1. init the PL011 console;
 *   2. construct IRQ (overlays the real legacy controller @0x3F00B200 and the
 *      per-core ARM-local block @0x40000000);
 *   3. install the vector table into VBAR_EL1;
 *   4. register a handler for the physical-timer line (CNTPNSIRQ) and route it
 *      to core 0 in the ARM-local block;
 *   5. program the generic timer, unmask IRQs, and WFI.
 *
 * Each timer IRQ is taken through the 0x280 (Current EL, SPx, IRQ) vector →
 * irq_handler_c → IRQ::dispatch(0) → the registered handler below.
 */
#include <cstdint>

#include "uart.hpp"
#include "timer.hpp"
#include "runtime.hpp"
#include "interrupt.hpp"
#include "memory_map.hpp"

namespace {
    volatile std::uint64_t g_ticks    = 0;
    std::uint64_t          g_interval = 0;

    void timer_isr() {
        ++g_ticks;
        uart_puts("[irq] timer tick #");
        uart_dec(g_ticks);
        uart_putc('\n');
        timer_set_tval(g_interval);   // re-arm (also deasserts the line)
    }
}

extern "C" void kmain() {
    using Local = BCM2837::ArmLocalRegisterAddress;
    using Src   = BCM2837::ArmLocalRegisterAddress::Source;

    uart_init();
    uart_puts("\n=== bcm2837 bare-metal: AArch64 IVT / IRQ demo ===\n");
    uart_puts("running at EL"); uart_dec(current_el()); uart_putc('\n');

    IRQ irq;                                  // overlays real MMIO
    set_active_irq(&irq);

    irq.install_vector_table(vector_table);   // msr VBAR_EL1 (INTERRUPT_BAREMETAL)
    uart_puts("VBAR_EL1 -> vector_table @ ");
    uart_hex(reinterpret_cast<std::uint64_t>(vector_table));
    uart_putc('\n');

    // Register the physical non-secure timer line and route it to core 0.
    irq.register_handler(IVT::kLocalBase + Src::CNTPNSIRQ, &timer_isr);
    auto& local = *new Local;                 // overlays 0x40000000
    local.m_register[Local::Register::CORE_TIMER_IRQCNTL0] |= (1u << Src::CNTPNSIRQ);

    // Fire ~5x/second.
    g_interval = timer_freq() / 5;
    uart_puts("CNTFRQ="); uart_dec(timer_freq());
    uart_puts("  interval="); uart_dec(g_interval); uart_putc('\n');

    timer_set_tval(g_interval);
    timer_enable();
    irq_unmask();

    uart_puts("interrupts unmasked; waiting for timer IRQs...\n");
    for(;;) __asm__ volatile("wfi");
}
