#include "uart.hpp"
#include "memory_map.hpp"   // BCM_PERIPH_BASE

/*
 * PL011 UART0 at BCM_PERIPH_BASE + 0x201000. On QEMU raspi3b the first `-serial`
 * (serial0) is wired to this PL011, so `-serial stdio` shows our output. QEMU
 * accepts writes without a full baud setup, but we do a minimal 8N1 init so this
 * also behaves on real silicon (where the mini-UART / clock setup still differ).
 */
namespace {
    constexpr std::uintptr_t kBase = BCM_PERIPH_BASE + 0x201000u;

    enum Reg : std::uintptr_t {
        DR   = 0x00, FR = 0x18, IBRD = 0x24, FBRD = 0x28,
        LCRH = 0x2C, CR = 0x30, IMSC = 0x38, ICR  = 0x44,
    };

    inline volatile std::uint32_t& reg(std::uintptr_t off) {
        return *reinterpret_cast<volatile std::uint32_t*>(kBase + off);
    }

    constexpr std::uint32_t FR_TXFF = 1u << 5;   // transmit FIFO full
}

void uart_init() {
    reg(CR)   = 0;              // disable while configuring
    reg(ICR)  = 0x7FF;          // clear pending interrupts
    reg(LCRH) = (1u << 4)       // FEN: enable FIFOs
              | (3u << 5);      // WLEN = 8 bits
    reg(CR)   = (1u << 0)       // UARTEN
              | (1u << 8)       // TXE
              | (1u << 9);      // RXE
}

void uart_putc(char c) {
    while(reg(FR) & FR_TXFF) { }
    reg(DR) = static_cast<std::uint32_t>(static_cast<unsigned char>(c));
}

void uart_puts(const char* s) {
    for(; *s; ++s) {
        if(*s == '\n') uart_putc('\r');
        uart_putc(*s);
    }
}

void uart_hex(std::uint64_t v) {
    uart_putc('0'); uart_putc('x');
    for(int shift = 60; shift >= 0; shift -= 4) {
        unsigned nib = (v >> shift) & 0xF;
        uart_putc(static_cast<char>(nib < 10 ? '0' + nib : 'a' + (nib - 10)));
    }
}

void uart_dec(std::uint64_t v) {
    char buf[20];
    int i = 0;
    if(v == 0) { uart_putc('0'); return; }
    while(v && i < 20) { buf[i++] = static_cast<char>('0' + (v % 10)); v /= 10; }
    while(i--) uart_putc(buf[i]);
}
