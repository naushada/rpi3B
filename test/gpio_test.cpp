#ifndef __gpio_test_cpp__
#define __gpio_test_cpp__

#include "gpio_test.hpp"
#include <vector>

void GPIOTest::SetUp() {

    m_gpio2PinMap = {
        {"GPIO_Number_1", "GPIO_PIN_"},
        {"GPIO_Number_2", "GPIO_PIN_3"},
        {"GPIO_Number_3", "GPIO_PIN_5"},
        {"GPIO_Number_4", "GPIO_PIN_7"},
        {"GPIO_Number_5", "GPIO_PIN_29"},
        {"GPIO_Number_6", "GPIO_PIN_31"},
        {"GPIO_Number_7", "GPIO_PIN_26"},
        {"GPIO_Number_8", "GPIO_PIN_24"},
        {"GPIO_Number_9", "GPIO_PIN_21"},
        {"GPIO_Number_10", "GPIO_PIN_19"},
        {"GPIO_Number_11", "GPIO_PIN_23"},
        {"GPIO_Number_12", "GPIO_PIN_32"},
        {"GPIO_Number_13", "GPIO_PIN_33"},
        {"GPIO_Number_14", "GPIO_PIN_8"},
        {"GPIO_Number_15", "GPIO_PIN_10"},
        {"GPIO_Number_16", "GPIO_PIN_36"},
        {"GPIO_Number_17", "GPIO_PIN_11"},
        {"GPIO_Number_18", "GPIO_PIN_12"},
        {"GPIO_Number_19", "GPIO_PIN_35"},
        {"GPIO_Number_20", "GPIO_PIN_38"},
        {"GPIO_Number_21", "GPIO_PIN_40"},
        {"GPIO_Number_22", "GPIO_PIN_15"},
        {"GPIO_Number_23", "GPIO_PIN_16"},
        {"GPIO_Number_24", "GPIO_PIN_18"},
        {"GPIO_Number_25", "GPIO_PIN_22"},
        {"GPIO_Number_26", "GPIO_PIN_37"},
        {"GPIO_Number_27", "GPIO_PIN_13"},
        {"GPIO_Number_28", "GPIO_PIN_"},
        {"GPIO_Number_29", "GPIO_PIN_"},
        {"GPIO_Number_30", "GPIO_PIN_"},
        {"GPIO_Number_31", "GPIO_PIN_"},
        {"GPIO_Number_32", "GPIO_PIN_"},
        {"GPIO_Number_33", "GPIO_PIN_"},
        {"GPIO_Number_34", "GPIO_PIN_"},
        {"GPIO_Number_35", "GPIO_PIN_"},
        {"GPIO_Number_36", "GPIO_PIN_"},
        {"GPIO_Number_37", "GPIO_PIN_"},
        {"GPIO_Number_38", "GPIO_PIN_"},
        {"GPIO_Number_39", "GPIO_PIN_"},
        {"GPIO_Number_40", "GPIO_PIN_"},
        {"GPIO_Number_41", "GPIO_PIN_"},
        {"GPIO_Number_42", "GPIO_PIN_"},
        {"GPIO_Number_43", "GPIO_PIN_"},
        {"GPIO_Number_44", "GPIO_PIN_"},
        {"GPIO_Number_45", "GPIO_PIN_"},
        {"GPIO_Number_46", "GPIO_PIN_"},
        {"GPIO_Number_47", "GPIO_PIN_"},
        {"GPIO_Number_48", "GPIO_PIN_"},
        {"GPIO_Number_49", "GPIO_PIN_"},
        {"GPIO_Number_50", "GPIO_PIN_"},
    };
}

void GPIOTest::TearDown() {

}

void GPIOTest::TestBody() {

}

TEST_F(GPIOTest, GPIO_Number_12_InputMode) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::InputMode);
    auto res = gpio().read(gpio_n);
    //std::cout << "gpio in Result:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::InputMode);
}

TEST_F(GPIOTest, GPIO_Number_12_OutputMode) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::OutputMode);
    auto res = gpio().read(gpio_n);
    //std::cout << "gpio out Result:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::OutputMode);
}

TEST_F(GPIOTest, GPIO_Number_12_AlternateFunction0) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction0);
    auto res = gpio().read(gpio_n);
    //std::cout << "AlternateFunction0:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction0);
}

TEST_F(GPIOTest, GPIO_Number_12_AlternateFunction1) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction1);
    auto res = gpio().read(gpio_n);
    //std::cout << "AlternateFunction1:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction1);
}

TEST_F(GPIOTest, GPIO_Number_12_AlternateFunction2) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction2);
    auto res = gpio().read(gpio_n);
    //std::cout << "AlternateFunction2:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction2);
}

TEST_F(GPIOTest, GPIO_Number_12_AlternateFunction3) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction3);
    auto res = gpio().read(gpio_n);
    //std::cout << "AlternateFunction3:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction3);
}

TEST_F(GPIOTest, GPIO_Number_12_AlternateFunction4) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction4);
    auto res = gpio().read(gpio_n);
    //std::cout << "AlternateFunction4:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction4);
}

TEST_F(GPIOTest, GPIO_Number_12_AlternateFunction5) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 32
    */
    GPIO::gpio_number gpio_n = 12;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction5);
    auto res = gpio().read(gpio_n);
    //std::cout << "AlternateFunction5:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction5);
}

TEST_F(GPIOTest, GPIO_Number_2to9_OutputMode) {

    GPIO::gpio_number gpio_n = 2;
    for(; gpio_n < 10; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }

    gpio_n = 2;
    for(; gpio_n < 10; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::OutputMode);    
    }
}

TEST_F(GPIOTest, GPIO_Number_10to19_OutputMode) {

    GPIO::gpio_number gpio_n = 10;
    for(; gpio_n < 20; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }

    gpio_n = 10;
    for(; gpio_n < 20; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }
}

TEST_F(GPIOTest, GPIO_Number_20to27_OutputMode) {

    GPIO::gpio_number gpio_n = 20;
    for(; gpio_n < 28; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }

    gpio_n = 20;
    for(; gpio_n < 28; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }
}

#if 0
TEST_F(GPIOTest, GPIO_Number_30to39_OutputMode) {

    GPIO::gpio_number gpio_n = 30;
    for(; gpio_n < 40; ++gpio_n) {
        gpio().write(gpio_n, GPIORegistersAddress::Config::OutputMode);
    }

    gpio_n = 30;
    auto res = gpio().read32(gpio_n);
    //std::cout << "GPIO 30 to 39: 0x" << std::hex << res << std::dec << std::endl;

    EXPECT_EQ(res, 0x9249249);
}

TEST_F(GPIOTest, GPIO_Number_40to49_OutputMode) {

    GPIO::gpio_number gpio_n = 40;
    for(; gpio_n < 50; ++gpio_n) {
        gpio().write(gpio_n, GPIORegistersAddress::Config::OutputMode);
    }

    gpio_n = 40;
    auto res = gpio().read32(gpio_n);
    //std::cout << "GPIO 40 to 49: 0x" << std::hex << res << std::dec << std::endl;

    EXPECT_EQ(res, 0x9249249);
}

TEST_F(GPIOTest, GPIO_Number_50to53_OutputMode) {

    GPIO::gpio_number gpio_n = 50;
    for(; gpio_n < 54; ++gpio_n) {
        gpio().write(gpio_n, GPIORegistersAddress::Config::OutputMode);
    }

    gpio_n = 50;
    auto res = gpio().read32(gpio_n);
    //std::cout << "GPIO 50 to 53: 0x" << std::hex << res << std::dec << std::endl;

    EXPECT_EQ(res, 0x249);
}
#endif

TEST_F(GPIOTest, GPIO_Number_2to9_AlternateFunction0) {

    GPIO::gpio_number gpio_n = 2;
    for(; gpio_n < 10; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction0);
    }

    gpio_n = 2;
    for(; gpio_n < 10; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction0);
    }
}

TEST_F(GPIOTest, GPIO_Number_10to19_AlternateFunction2) {

    GPIO::gpio_number gpio_n = 10;
    for(; gpio_n < 20; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction2);
    }

    gpio_n = 10;
    for(; gpio_n < 20; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction2);
    }
}

TEST_F(GPIOTest, GPIO_Number_20to27_AlternateFunction5) {

    GPIO::gpio_number gpio_n = 20;
    for(; gpio_n < 28; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction5);
    }

    gpio_n = 20;
    for(; gpio_n < 28; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction5);
    }
}

TEST_F(GPIOTest, GPIO_Number_2to19_InputMode) {

    GPIO::gpio_number gpio_n = 2;
    for(; gpio_n < 20; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::InputMode);
    }

    gpio_n = 2;
    for(; gpio_n < 20; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::InputMode);
    }
}

TEST_F(GPIOTest, GPIO_Number_2to19_OutputMode_AlternateFunction3) {

    GPIO::gpio_number gpio_n = 2;
    for(; gpio_n < 10; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }

    for(; gpio_n < 20; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction3);
    }

    gpio_n = 2;
    for(; gpio_n < 10; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::OutputMode);
    }

    for(; gpio_n < 20; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction3);
    }
}

TEST_F(GPIOTest, GPIO_Number_20to27_InputMode_AlternateFunction4) {

    GPIO::gpio_number gpio_n = 20;
    for(; gpio_n < 25; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::InputMode);
    }

    for(; gpio_n < 28; ++gpio_n) {
        gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::AlternateFunction4);
    }

    gpio_n = 20;
    for(; gpio_n < 25; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::InputMode);    
    }

    for(; gpio_n < 28; ++gpio_n) {
        auto res = gpio().read(gpio_n);
        EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::AlternateFunction4);    
    }
}

TEST_F(GPIOTest, GPIO_Number_0to31_GPSETn) {

    GPIO::gpio_number gpio_n = 0;
    for(; gpio_n < 32; ++gpio_n) {
        gpio().GPSETn(gpio_n);
    }

    gpio_n = 20;
    auto res = gpio().GPGETn(gpio_n);
    //std::cout << "GPIO 0 to 31: 0x" << std::hex << res << std::dec << std::endl;

    EXPECT_EQ(res, 01);
}

TEST_F(GPIOTest, GPIO_Number_32to53_GPSETn) {

    GPIO::gpio_number gpio_n = 32;
    for(; gpio_n < 54; ++gpio_n) {
        gpio().GPSETn(gpio_n);
    }

    gpio_n = 40;
    auto res = gpio().GPGETn(gpio_n);
    //std::cout << "GPIO 32 to 53: 0x" << std::hex << res << std::dec << std::endl;

    EXPECT_EQ(res, 01);
}

TEST_F(GPIOTest, GPIO_Number_14_InputMode) {

    /**
     * GPIONumber 12 is mapped to GPIO PIN 8
    */
    GPIO::gpio_number gpio_n = 14;
    gpio().write(gpio_n, RPi3B::GPIORegistersAddress::Config::InputMode);
    auto res = gpio().read(gpio_n);
    //std::cout << "gpio in Result:" << std::to_string(res) << std::endl;
    EXPECT_EQ(res, RPi3B::GPIORegistersAddress::Config::InputMode);
}

#endif /*__gpio_test_cpp__*/