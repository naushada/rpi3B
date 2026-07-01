/*
 * C exception handlers invoked from vectors.S. The IRQ path delegates straight
 * into the AArch64 model: IRQ::dispatch() reads the per-core ARM-local source
 * and the legacy controller, then calls the registered handler.
 */
#include "uart.hpp"
#include "runtime.hpp"
#include "interrupt.hpp"

namespace {
    IRQ* g_active = nullptr;
}

void set_active_irq(IRQ* irq) { g_active = irq; }

extern "C" void irq_handler_c() {
    // Dispatch for the core that actually took the IRQ: each core has its own
    // CORE_IRQ_SOURCE / generic timer, so dispatch(core) reads the right one.
    if(g_active) g_active->dispatch(core_id());
}

extern "C" void bad_exception_c(unsigned long kind,
                                unsigned long esr,
                                unsigned long elr) {
    static const char* const name[] = { "SYNC", "IRQ", "FIQ", "SERROR" };
    uart_puts("\n[EXCEPTION] ");
    uart_puts(name[kind & 3]);
    uart_puts("  ESR="); uart_hex(esr);
    uart_puts("  ELR="); uart_hex(elr);
    uart_puts("\n[HALTED]\n");
    for(;;) __asm__ volatile("wfe");
}
