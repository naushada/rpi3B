#ifndef __interrupt_hpp__
#define __interrupt_hpp__

#include <cstdint>
#include <cstdio>
#include <atomic>

#include "memory_map.hpp"

class IVT {
    public:
        using pointerToFn = void (*)();
        /// @brief Address of below will be 0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C
        enum Number {
            Reset,
            UndefinedInstruction,
            SWI,
            InstructionAbort,
            DataAbort,
            Hypervisor,
            IRQ,
            FIQ,
            End
        };

        IVT() {}
        ~IVT() {}

        pointerToFn &operator[](Number irq) {
            return(m_IRQTable[irq]);
        }

    private:
        pointerToFn m_IRQTable[End];
};

class IRQ {
    public:
        using irq_number = std::uint8_t;

        //using IVT::IVT;

        IRQ() : m_memory(*new RPi3B::InterruptRegisterAddress) {}

        IRQ(auto region) : m_memory(*new(region) RPi3B::InterruptRegisterAddress) {}

        ~IRQ() = default;
        void enable(irq_number number);
        void disable(irq_number number);
        bool isEnabled(irq_number number);
        

    private:
        RPi3B::InterruptRegisterAddress& m_memory;
};








#endif /*__interrupt_hpp__*/