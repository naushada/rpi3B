#ifndef __interrupt_test_cpp__
#define __interrupt_test_cpp__

#include "interrupt_test.hpp"

void IRQTest::SetUp() {
    
}

void IRQTest::TearDown() {

}

void IRQTest::TestBody() {

}

TEST_F(IRQTest, Irq) {
    /*
     * Refer 7.5 Section of BCM2837-ARM-Peripherals.pdf
     * IRQ Number  Name
     *   48         smi
         49         gpio_int[0]
         50         gpio_int[1]
         51         gpio_int[2]
         52         gpio_int[3]
         53         i2c_int
         54         spi_int
         55         pcm_int
         56
         57         uart_int
     */
    ::printf("0x%X\n", irq());
    //::printf("0x%X\n", &irq()[IVT::Reset]);
    //::printf("0x%X\n", &irq()[IVT::UndefinedInstruction]);
    //::printf("0x%X\n", &irq()[IVT::SWI]);
}







#endif /*__interrupt_test_cpp__*/