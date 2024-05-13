#ifndef __clock_cpp__
#define __clock_cpp__

#include "clock.hpp"

void CLOCK::CM_GP0DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor, CLOCK::divisor_type value) {

    switch (divisor)
    {
        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVI:
        {
            /// SET bits 12...23 ---- Integer Divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP0DIV] |= ((~(1U << 12) & value) << 12);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVF:
        {
            /// SET bits 0...11 --- Float divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP0DIV] |= (~(1U << 12) & value);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE:
        {
            /// SET bits 0...23 --- both Integer & Float divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP0DIV] |= (~(1U << 24) & value);
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
}

CLOCK::divisor_type CLOCK::CM_GP0DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor) const {
    divisor_type value = 0;

    switch (divisor)
    {
        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVI:
        {
            /// SET bits 12...23 ---- Integer Divisor
            value = (memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP0DIV] >> 12) & (~(1U << 12) & value);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVF:
        {
            /// SET bits 0...11 --- Float divisor
            value = memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP0DIV] & (~(1U << 12));
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE:
        {
            /// SET bits 0...23 --- both Integer & Float divisor
            value = memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP0DIV] & (~(1U << 24));
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
    return(value);
}

void CLOCK::CM_GP1DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor, CLOCK::divisor_type value) {
    switch (divisor)
    {
        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVI:
        {
            /// SET bits 12...23 ---- Integer Divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP1DIV] |= ((~(1U << 12) & value) << 12);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVF:
        {
            /// SET bits 0...11 --- Float divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP1DIV] |= (~(1U << 12) & value);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE:
        {
            /// SET bits 0...23 --- both Integer & Float divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP1DIV] |= (~(1U << 24) & value);
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
}

CLOCK::divisor_type CLOCK::CM_GP1DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor) const {
    divisor_type value = 0;

    switch (divisor)
    {
        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVI:
        {
            /// SET bits 12...23 ---- Integer Divisor
            value = (memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP1DIV] >> 12) & (~(1U << 12) & value);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVF:
        {
            /// SET bits 0...11 --- Float divisor
            value = memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP1DIV] & (~(1U << 12));
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE:
        {
            /// SET bits 0...23 --- both Integer & Float divisor
            value = memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP1DIV] & (~(1U << 24));
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
    return(value);
}

void CLOCK::CM_GP2DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor, CLOCK::divisor_type value) {
    switch (divisor)
    {
        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVI:
        {
            /// SET bits 12...23 ---- Integer Divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP2DIV] |= ((~(1U << 12) & value) << 12);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVF:
        {
            /// SET bits 0...11 --- Float divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP2DIV] |= (~(1U << 12) & value);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE:
        {
            /// SET bits 0...23 --- both Integer & Float divisor
            memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP2DIV] |= (~(1U << 24) & value);
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
}

CLOCK::divisor_type CLOCK::CM_GP2DIV(RPi3B::ClockRegistersAddress::ClockDivisor divisor) const {
    divisor_type value = 0;

    switch (divisor)
    {
        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVI:
        {
            /// SET bits 12...23 ---- Integer Divisor
            value = (memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP2DIV] >> 12) & (~(1U << 12) & value);
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::DIVF:
        {
            /// SET bits 0...11 --- Float divisor
            value = memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP2DIV] & (~(1U << 12));
        }
        break;

        case RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE:
        {
            /// SET bits 0...23 --- both Integer & Float divisor
            value = memory().m_register[RPi3B::ClockRegistersAddress::Register::CM_GP2DIV] & (~(1U << 24));
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
    return(value);
}

void CLOCK::write(RPi3B::ClockRegistersAddress::Register reg, RPi3B::ClockRegistersAddress::ClockDivisor divif, CLOCK::divisor_type value) {

    switch (divif)
    {
        case RPi3B::ClockRegistersAddress::Register::CM_GP0DIV:
        {
            CM_GP0DIV(divif, value);
        }
        break;

        case RPi3B::ClockRegistersAddress::Register::CM_GP1DIV:
        {
            CM_GP1DIV(divif, value);
        }
        break;

        case RPi3B::ClockRegistersAddress::Register::CM_GP2DIV:
        {
            CM_GP2DIV(divif, value);
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
}

CLOCK::divisor_type CLOCK::read(RPi3B::ClockRegistersAddress::Register, RPi3B::ClockRegistersAddress::ClockDivisor divif) const {

    CLOCK::divisor_type value = 0;
    switch (divif)
    {
        case RPi3B::ClockRegistersAddress::Register::CM_GP0DIV:
        {
            value = CM_GP0DIV(divif);
        }
        break;

        case RPi3B::ClockRegistersAddress::Register::CM_GP1DIV:
        {
            value = CM_GP1DIV(divif);
        }
        break;

        case RPi3B::ClockRegistersAddress::Register::CM_GP2DIV:
        {
            value = CM_GP2DIV(divif);
        }
        break;
    
        default:
            /// Error Handling
            break;
    }
    return(value);
}

#endif /*__clock_cpp__*/