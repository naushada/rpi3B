#ifndef __interrupt_hpp__
#define __interrupt_hpp__

#include <cstdint>
#include <cstdio>
#include <atomic>

#include "memory_map.hpp"

class IRQ {
    public:
        using gpio_number = std::uint32_t;
        using pin_number = std::uint32_t;

        IRQ() : m_memory(*new RPi3B::InterruptRegisterAddress) {}

        IRQ(auto region) : m_memory(*new(region) RPi3B::InterruptRegisterAddress) {}

        ~IRQ() = default;

    private:
        RPi3B::InterruptRegisterAddress& m_memory;
};








#endif /*__interrupt_hpp__*/