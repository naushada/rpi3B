#ifndef __interrupt_cpp__
#define __interrupt_cpp__

#include "interrupt.hpp"



void IRQ::enable(irq_number irqNumber) {
    if(irqNumber > 31) {
        m_memory.m_register[RPi3B::InterruptRegisterAddress::Register::Enable_IRQs_1] |= (1U << irqNumber);
    } else {
        m_memory.m_register[RPi3B::InterruptRegisterAddress::Register::Enable_IRQs_2] |= (1U << (irqNumber - 32));
    }
}

void IRQ::disable(irq_number irqNumber) {
    if(irqNumber > 31) {
        m_memory.m_register[RPi3B::InterruptRegisterAddress::Register::Disable_IRQs_1] |= (1U << irqNumber);
        
    } else {
        m_memory.m_register[RPi3B::InterruptRegisterAddress::Register::Disable_IRQs_2] |= (1U << (irqNumber - 32));
    }
}

bool IRQ::isEnabled(irq_number number) {
    if(number > 31) {
        auto& tmpReg = m_memory.m_register[RPi3B::InterruptRegisterAddress::Register::IRQ_Pending1];
        tmpReg &= (1U << number);
        return((tmpReg >> number) & 1U == 1U);
    } else {
        auto& tmpReg = m_memory.m_register[RPi3B::InterruptRegisterAddress::Register::IRQ_Pending2];
        tmpReg &= (1U << number);
        return((tmpReg >> number) & 1U == 1U);
    }
}

void IRQ::install_IRQHandler(std::uint8_t IRQNumber, IVT::pointerToFn cb) {
    IVT& theIVT = *reinterpret_cast<IVT*>(m_ivt[IVT::Number::Reset]);
    theIVT[IVT::Number::IRQ] = cb;
}

void IRQ::install_FIQHandler(std::uint8_t IRQNumber, IVT::pointerToFn cb) {
    IVT& theIVT = *reinterpret_cast<IVT*>(m_ivt[IVT::Number::Reset]);
    theIVT[IVT::Number::FIQ] = cb;
}








#endif /*__interrupt_cpp__*/