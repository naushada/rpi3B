#ifndef __clock_hpp__
#define __clock_hpp__

#include <cstdint>
#include <cstdio>
#include <atomic>
#include "memory_map.hpp"


class CLOCK {
    public:
        using gpio_number = std::uint32_t;
        using divisor_type = std::uint32_t;

        CLOCK() : m_memory(*new RPi3B::ClockRegistersAddress) {}

        CLOCK(auto region) : m_memory(*new(region) RPi3B::ClockRegistersAddress) {}

        ~CLOCK() = default;

        void CM_GP0DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor, divisor_type value);
        divisor_type CM_GP0DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor) const;
        void CM_GP1DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor, divisor_type value);
        divisor_type CM_GP1DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor) const;
        void CM_GP2DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor, divisor_type value);
        divisor_type CM_GP2DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor) const;
        void write(RPi3B::ClockRegistersAddress::Register reg, RPi3B::ClockRegistersAddress::ClockDivisor divif ,divisor_type value);
        divisor_type read(RPi3B::ClockRegistersAddress::Register, RPi3B::ClockRegistersAddress::ClockDivisor divif) const;

        auto& memory() const {
            return(m_memory);
        }

    private:
        RPi3B::ClockRegistersAddress& m_memory;
};







#endif /*__clock_hpp__*/