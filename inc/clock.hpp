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

        CLOCK() : m_memory(*new ClockRegistersAddress) {}

        CLOCK(auto region) : m_memory(*new(region) ClockRegistersAddress) {}

        ~CLOCK() = default;

    private:
        ClockRegistersAddress& m_memory;
};







#endif /*__clock_hpp__*/