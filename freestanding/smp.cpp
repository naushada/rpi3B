/*
 * smp.cpp — SMP bring-up + the per-core timer-IRQ demo.
 *
 * QEMU raspi3b (like a real Pi 3) holds cores 1..3 in the armstub spin-table:
 * each polls a release mailbox and, when it becomes non-zero, branches there.
 * Core 0 writes the image entry (_start) into the three mailboxes and SEVs; each
 * secondary re-enters boot.S, drops to EL1, and lands in ksecondary().
 *
 * Every core then arms *its own* physical timer. When it fires, the IRQ is taken
 * on that core, and irq_handler_c() calls IRQ::dispatch(core) — reading that
 * core's CORE_IRQ_SOURCE — so the same registered handler services all four.
 */
#include <cstdint>

#include "uart.hpp"
#include "timer.hpp"
#include "mmu.hpp"
#include "runtime.hpp"
#include "interrupt.hpp"
#include "memory_map.hpp"

/* The image entry (boot.S). Secondaries branch here when released. */
extern "C" char _start[];

namespace {
    using Local = BCM2837::ArmLocalRegisterAddress;
    using Src   = BCM2837::ArmLocalRegisterAddress::Source;

    volatile std::uint64_t g_ticks[4] = { 0, 0, 0, 0 };

    /// ~5 IRQs/second (CNTFRQ is identical on every core).
    inline std::uint64_t interval() { return timer_freq() / 5; }

    /* Pi 3 aarch64 spin-table release addresses for cores 1/2/3. */
    constexpr std::uintptr_t kReleaseAddr[3] = { 0xe0, 0xe8, 0xf0 };
}

void timer_isr() {
    unsigned c = core_id();
    uart_lock();
    uart_puts("[core "); uart_dec(c);
    uart_puts("] timer tick #"); uart_dec(++g_ticks[c]);
    uart_putc('\n');
    uart_unlock();
    timer_set_tval(interval());   // re-arm this core's timer (deasserts the line)
}

void core_timer_start(unsigned core) {
    // Route this core's physical non-secure timer IRQ to itself.
    auto& local = *new Local;     // overlays 0x40000000
    local.m_register[Local::Register::CORE_TIMER_IRQCNTL0 + core] |= (1u << Src::CNTPNSIRQ);
    timer_set_tval(interval());
    timer_enable();
    irq_unmask();
}

void smp_release_secondaries() {
    const std::uint64_t entry = reinterpret_cast<std::uint64_t>(&_start[0]);
    for(std::uintptr_t addr : kReleaseAddr) {
        auto* slot = reinterpret_cast<volatile std::uint64_t*>(addr);
        *slot = entry;
        // Core 0 has its caches on; the secondaries poll with the MMU OFF, so
        // clean the line to the point of coherency before waking them.
        __asm__ volatile("dc cvac, %0" : : "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy\n sev\n" ::: "memory");
}

extern "C" void ksecondary() {
    unsigned core = core_id();
    mmu_enable();                       // shared tables built by core 0
    vbar_el1_set(vector_table);         // this core's VBAR_EL1
    core_timer_start(core);

    uart_lock();
    uart_puts("[core "); uart_dec(core); uart_puts("] online\n");
    uart_unlock();

    for(;;) __asm__ volatile("wfi");
}
