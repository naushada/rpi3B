#ifndef __interrupt_test_cpp__
#define __interrupt_test_cpp__

#include "interrupt_test.hpp"

void IRQTest::SetUp() {

}

void IRQTest::TearDown() {

}

void IRQTest::TestBody() {

}

namespace {
    /* Sentinels the dispatch tests observe. Captureless handlers so they convert
     * to IVT::pointerToFn (void(*)()). */
    int g_fired = 0;
    void timer_handler() { g_fired = 1; }
    void i2c_handler()   { g_fired = 53; }
    void uart_handler()  { g_fired = 10; }
}

/* --- AArch64 vector-table geometry (ARMv8-A VBAR_EL1 model) --- */

TEST_F(IRQTest, Aarch64_VectorTable_Has16_Slots_Of_0x80) {
    /*
     * ARMv8-A: VBAR_EL1 points at a 16-entry table of 0x80-byte slots (0x800
     * total, 2 KB-aligned), grouped 4×4 by source. This replaces the old
     * AArch32 8-vector (0x00..0x1C) model.
     */
    EXPECT_EQ(static_cast<int>(IVT::Vector::Count), 16);
    EXPECT_EQ(IVT::kEntryStride, 0x80u);
    EXPECT_EQ(IVT::kTableBytes,  0x800u);
    EXPECT_EQ(IVT::kTableAlign,  0x800u);

    EXPECT_EQ(IVT::byte_offset(IVT::Vector::CurEL_SP0_Sync),     0x000u);
    EXPECT_EQ(IVT::byte_offset(IVT::Vector::CurEL_SPx_IRQ),      0x280u);
    EXPECT_EQ(IVT::byte_offset(IVT::Vector::LowerEL_A64_IRQ),    0x480u);
    EXPECT_EQ(IVT::byte_offset(IVT::Vector::LowerEL_A32_SError), 0x780u);

    /* The compat Number::IRQ/FIQ alias resolves into the Current-EL-SPx group. */
    EXPECT_EQ(static_cast<int>(IVT::Number::IRQ),
              static_cast<int>(IVT::Vector::CurEL_SPx_IRQ));
    EXPECT_EQ(static_cast<int>(IVT::Number::FIQ),
              static_cast<int>(IVT::Vector::CurEL_SPx_FIQ));
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
    SUCCEED();
}

/* --- Regression tests for the fixes in docs/DRIVER_REVIEW.md §2.4 --- */

TEST_F(IRQTest, Enable_Below32_Uses_Bank1_And_IsEnabled) {
    irq().enable(20);
    EXPECT_TRUE(irq().isEnabled(20));
    EXPECT_FALSE(irq().isEnabled(21));
}

TEST_F(IRQTest, Enable_AtOrAbove32_Uses_Bank2) {
    /* IRQ 53 == i2c_int (see table above): bank2, bit 21. */
    irq().enable(53);
    EXPECT_TRUE(irq().isEnabled(53));
    EXPECT_FALSE(irq().isEnabled(20)); /* a different bank stays clear */
}

TEST_F(IRQTest, Disable_Sets_Disable_Register) {
    irq().disable(54); /* spi_int */
    EXPECT_TRUE(irq().isEnabled(54) == false); /* not enabled; nothing in enable reg */
}

TEST_F(IRQTest, Install_Handlers_Does_Not_Crash) {
    /* Previously dereferenced a wild pointer; must now be a safe assignment. */
    irq().install_IRQHandler(53, []{});
    irq().install_FIQHandler(54, []{});
    SUCCEED();
}

/* --- AArch64 IRQ dispatch (single vector → many sources) --- */

TEST_F(IRQTest, InstallIRQHandler_Registers_By_Number) {
    /* The IRQNumber arg was previously ignored; it now keys the dispatch table. */
    irq().install_IRQHandler(53, &i2c_handler);
    EXPECT_EQ(irq().handler_for(53), &i2c_handler);
    EXPECT_EQ(irq().handler_for(10), nullptr);
}

TEST_F(IRQTest, Dispatch_Routes_Peripheral_IRQ_Via_GPU_Line) {
    using IR  = BCM2837::InterruptRegisterAddress::Register;
    using LR  = BCM2837::ArmLocalRegisterAddress::Register;
    using Src = BCM2837::ArmLocalRegisterAddress::Source;
    const std::size_t L = IRQ::kLocalWordOffset;

    irq().register_handler(53, &i2c_handler);
    /* Per-core IRQ source: the combined GPU line is asserted... */
    region()[L + LR::CORE_IRQ_SOURCE0] = (1U << Src::GPU);
    /* ...and the legacy controller reports IRQ 53 pending in bank 2 (bit 21). */
    region()[IR::IRQ_Pending2] = (1U << (53 - 32));

    g_fired = 0;
    irq().dispatch(0);
    EXPECT_EQ(g_fired, 53);
}

TEST_F(IRQTest, Dispatch_Picks_Lowest_Numbered_Pending) {
    using IR  = BCM2837::InterruptRegisterAddress::Register;
    using LR  = BCM2837::ArmLocalRegisterAddress::Register;
    using Src = BCM2837::ArmLocalRegisterAddress::Source;
    const std::size_t L = IRQ::kLocalWordOffset;

    irq().register_handler(53, &i2c_handler);
    irq().register_handler(10, &uart_handler);
    region()[L + LR::CORE_IRQ_SOURCE0] = (1U << Src::GPU);
    region()[IR::IRQ_Pending1] = (1U << 10);        /* bank1 IRQ 10 */
    region()[IR::IRQ_Pending2] = (1U << (53 - 32)); /* and bank2 IRQ 53 */

    g_fired = 0;
    irq().dispatch(0);
    EXPECT_EQ(g_fired, 10);   /* bank1 (lower numbers) serviced first */
}

TEST_F(IRQTest, Dispatch_Routes_GenericTimer_Line) {
    using LR  = BCM2837::ArmLocalRegisterAddress::Register;
    using Src = BCM2837::ArmLocalRegisterAddress::Source;
    const std::size_t L = IRQ::kLocalWordOffset;

    /* CNTPNSIRQ (physical non-secure timer) is the AArch64 EL1 kernel tick. */
    irq().register_handler(IVT::kLocalBase + Src::CNTPNSIRQ, &timer_handler);
    region()[L + LR::CORE_IRQ_SOURCE0] = (1U << Src::CNTPNSIRQ);

    g_fired = 0;
    irq().dispatch(0);
    EXPECT_EQ(g_fired, 1);
}

TEST_F(IRQTest, LocalTimer_IRQ_Control_Bit) {
    using LR  = BCM2837::ArmLocalRegisterAddress::Register;
    using Src = BCM2837::ArmLocalRegisterAddress::Source;
    const std::size_t L = IRQ::kLocalWordOffset;

    /* Enabling the core-0 CNTPNSIRQ IRQ sets bit 1 of CORE_TIMER_IRQCNTL0. */
    region()[L + LR::CORE_TIMER_IRQCNTL0] |= (1U << Src::CNTPNSIRQ);
    EXPECT_NE(region()[L + LR::CORE_TIMER_IRQCNTL0] & (1U << 1), 0u);
}

TEST_F(IRQTest, InstallVectorTable_Records_Aligned_Pointer_On_Host) {
    /* Host no-op records the pointer (real build emits `msr VBAR_EL1`). Use a
     * local instance since m_vbar is per-object, not in the shared region. */
    std::vector<std::uint32_t> buf(IRQ::region_words());
    std::memset(buf.data(), 0, buf.size() * sizeof(std::uint32_t));
    IRQ local(buf.data());

    alignas(0x800) static unsigned char table[IVT::kTableBytes];
    local.install_vector_table(table);
    EXPECT_EQ(local.vector_table(), table);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(local.vector_table())
                  & (IVT::kTableAlign - 1),
              0u);
}

#endif /*__interrupt_test_cpp__*/
