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
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, (35 << 11 | 492));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP0DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP1DIV_Read_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, (35 << 11 | 492));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP1DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}

TEST_F(CLOCKTest, ClockRegister_CM_GP2DIV_Read_BOTH) {
    clock().write(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::BOTH_VALUE, (35 << 11 | 492));
    auto res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVF);
    EXPECT_EQ(res, 492);
    res = clock().read(RPi3B::ClockRegistersAddress::Register::CM_GP2DIV, RPi3B::ClockRegistersAddress::ClockDivisor::DIVI);
    EXPECT_EQ(res, 35);
}







#endif /*__clock_test_cpp__*/