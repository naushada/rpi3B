/*
 * Minimal freestanding libc: just enough for the driver code and kmain. The
 * BCM2837 register-block constructors call std::printf to log register
 * addresses; here that routes to the UART, so those logs become a boot sign of
 * life. memset/memcpy/memmove/memcmp back any the compiler emits.
 */
#include <cstdint>
#include <cstddef>
#include <cstdarg>

#include "uart.hpp"

extern "C" {

void* memset(void* d, int c, std::size_t n) {
    auto* p = static_cast<unsigned char*>(d);
    while(n--) *p++ = static_cast<unsigned char>(c);
    return d;
}

void* memcpy(void* d, const void* s, std::size_t n) {
    auto* dp = static_cast<unsigned char*>(d);
    auto* sp = static_cast<const unsigned char*>(s);
    while(n--) *dp++ = *sp++;
    return d;
}

void* memmove(void* d, const void* s, std::size_t n) {
    auto* dp = static_cast<unsigned char*>(d);
    auto* sp = static_cast<const unsigned char*>(s);
    if(dp < sp) { while(n--) *dp++ = *sp++; }
    else        { dp += n; sp += n; while(n--) *--dp = *--sp; }
    return d;
}

int memcmp(const void* a, const void* b, std::size_t n) {
    auto* pa = static_cast<const unsigned char*>(a);
    auto* pb = static_cast<const unsigned char*>(b);
    for(; n--; ++pa, ++pb) if(*pa != *pb) return *pa - *pb;
    return 0;
}

static void print_uint(unsigned long long v, unsigned base, bool upper) {
    char buf[24];
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    if(v == 0) { uart_putc('0'); return; }
    while(v && i < 24) { buf[i++] = digits[v % base]; v /= base; }
    while(i--) uart_putc(buf[i]);
}

/* Supports %c %s %d/%i %u %x %X %p %lu %lx %% — enough for the driver + kmain. */
int printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for(; *fmt; ++fmt) {
        if(*fmt != '%') { uart_putc(*fmt); continue; }
        ++fmt;
        int lng = 0;
        while(*fmt == 'l') { ++lng; ++fmt; }
        switch(*fmt) {
            case 'c': uart_putc(static_cast<char>(va_arg(ap, int))); break;
            case 's': uart_puts(va_arg(ap, const char*)); break;
            case 'd': case 'i': {
                long long v = lng ? va_arg(ap, long) : va_arg(ap, int);
                if(v < 0) { uart_putc('-'); v = -v; }
                print_uint(static_cast<unsigned long long>(v), 10, false);
                break;
            }
            case 'u': print_uint(lng ? va_arg(ap, unsigned long)
                                     : va_arg(ap, unsigned int), 10, false); break;
            case 'x': print_uint(lng ? va_arg(ap, unsigned long)
                                     : va_arg(ap, unsigned int), 16, false); break;
            case 'X': print_uint(lng ? va_arg(ap, unsigned long)
                                     : va_arg(ap, unsigned int), 16, true);  break;
            case 'p': { uart_putc('0'); uart_putc('x');
                        print_uint(reinterpret_cast<unsigned long>(va_arg(ap, void*)), 16, false);
                        break; }
            case '%': uart_putc('%'); break;
            default:  uart_putc('%'); uart_putc(*fmt); break;
        }
    }
    va_end(ap);
    return 0;
}

/* A pure-virtual call would be a bug; make it a visible halt rather than a jump
 * through a null slot. */
void __cxa_pure_virtual() {
    uart_puts("\n[panic] __cxa_pure_virtual\n");
    for(;;) __asm__ volatile("wfe");
}

} // extern "C"
