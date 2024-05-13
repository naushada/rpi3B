#ifndef __clock_hpp__
#define __clock_hpp__

#include <cstdint>
#include <cstdio>
#include <atomic>
#include "memory_map.hpp"


class CLOCK {
    public:
        using gpio_number = std::uint32_t;
        using pin_number = std::uint32_t;

        CLOCK() : m_memory(*new RPi3B::ClockRegistersAddress) {}

        CLOCK(auto region) : m_memory(*new(region) RPi3B::ClockRegistersAddress) {}

        ~CLOCK() = default;

    private:
        RPi3B::ClockRegistersAddress& m_memory;
};







#endif /*__clock_hpp__*/