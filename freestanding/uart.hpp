#ifndef __fs_uart_hpp__
#define __fs_uart_hpp__

#include <cstdint>

/* Minimal PL011 (UART0) console for QEMU `-M raspi3b -serial ...` and real Pi
 * hardware. Enough to print from kmain / the exception handlers. */
void uart_init();
void uart_putc(char c);
void uart_puts(const char* s);
void uart_hex(std::uint64_t v);   ///< 0x-prefixed 64-bit hex
void uart_dec(std::uint64_t v);   ///< unsigned decimal

/* Once SMP is live, wrap a multi-call line in uart_lock()/uart_unlock() so
 * output from the four cores stays legible. No-op until uart_enable_lock()
 * (called after the MMU is on — the spinlock uses cacheable exclusives). */
void uart_enable_lock();
void uart_lock();
void uart_unlock();

#endif /*__fs_uart_hpp__*/
