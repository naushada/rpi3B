#ifndef __clock_test_cpp__
#define __clock_test_cpp__

#include "clock_test.hpp"

void CLOCKTest::SetUp() {
    
}

void CLOCKTest::TearDown() {

}

void CLOCKTest::TestBody() {

}

TEST_F(CLOCKTest, ClockRegister_CM_GP0DIV_Read_DIVI) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI, 35);
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP1DIV_Read_DIVI) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI, 35);
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Read_DIVI) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI, 35);
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP0DIV_Read_DIVF) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF, 492);
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP1DIV_Read_DIVF) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF, 492);
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Read_DIVF) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF, 492);
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP0DIV_Read_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((35 << 12) | 492));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP1DIV_Read_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((35 << 12) | 492));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Read_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((35 << 12) | 492));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP0DIV_Write_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((0xFFF << 12) | 0xFFF));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 0xFFF);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 0xFFF);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP1DIV_Write_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((0xFF2 << 12) | 0xFFD));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 0xFFD);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 0xFF2);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Write_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((0x1 << 12) | 0x1));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 0x1);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 0x1);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Write_BOTH_Var1) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((21 << 12) | 854));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 854);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 21);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Write_BOTH_Var2) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, ((10 << 12) | 939));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 939);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 10);
}




#endif /*__clock_test_cpp__*/