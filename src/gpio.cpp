#ifndef __gpio_cpp__
#define __gpio_cpp__

#include <unordered_map>

#include "gpio.hpp"

static const std::unordered_map<GPIO::gpio_number, GPIO::pin_number> gpioNo2PinNo = {
    {0,   3}, {1,   3}, {2,   3}, {3,   5}, {4,   7}, {5,  29}, {6,  31}, {7,  26}, {8,  24}, {9,  21},
    {10, 19}, {11, 23}, {12, 32}, {13, 33}, {14,  8}, {15, 10}, {16, 36}, {17, 11}, {18, 12}, {19, 35},
    {20, 38}, {21, 40}, {22, 15}, {23, 16}, {24, 18}, {25, 22}, {26, 37}, {27, 13}, {28,  3}, {29,  3},
    {30,  3}, {31,  3}, {32,  3}, {33,  3}, {34,  3}, {35,  3}, {36,  3}, {37,  3}, {38,  3}, {39,  3},
    {40,  3}, {41,  3}, {42,  3}, {43,  3}, {44,  3}, {45,  3}, {46,  3}, {47,  3}, {48,  3}, {49,  3},
    {50,  3}, {51,  3}, {52,  3}, {53,  3}
};

void GPIO::write32(gpio_number gpio_n, std::uint32_t value) {
    m_register[gpio_n/10] = value;
}

void GPIO::write(gpio_number gpio_n, GPIO::Config cfg) {
    /**
     * @brief
     *      First clear those bits and then set to value config.
    */
    m_register[(gpio_n / 10)] &= (~(07U << ((gpio_n % 10) * 3U)));
    m_register[(gpio_n / 10)] |= (cfg << ((gpio_n % 10) * 3U));
}

std::uint32_t GPIO::read32(gpio_number gpio_n) {
    return(m_register[gpio_n/10]);
}

std::uint32_t GPIO::read(gpio_number gpio_n) {

    return((m_register[gpio_n/10]  >> ((gpio_n % 10U) * 3U)) & 0b111);
}

std::uint32_t GPIO::GPLEVn(gpio_number gpio_n) const {
    if(gpio_n < 32) {
        return((m_register[Register::BCM2837_GPLEV0] >> gpio_n) & 1U);
    }
    return ((m_register[Register::BCM2837_GPLEV1] >> (gpio_n - 32) & 1U));
}

void GPIO::GPLEVn(gpio_number gpio_n) {
    if(gpio_n < 32) {
        m_register[Register::BCM2837_GPLEV0] |= (1U << gpio_n);
    }
    m_register[Register::BCM2837_GPLEV1] |= (1U << (gpio_n - 32));
}

void GPIO::output(gpio_number gpio_n) {

    /* @brief:
    *  Each 32bits register can accomodate 10 gpios FSEL, hence division by 10.
    *  gpios (0-9), (10-19), (20-29), (30-39), (40-49) and (50-53) will be 
    *  programmed in register FSEL0, FSEL1, FSEL2, FSEL3, FSEL4 and FSEL5 respectively.
    */
    m_register[(gpio_n / 10)] |= (1U << ((gpio_n % 10) * 3U));
}

void GPIO::input(gpio_number gpio_n) {

    /* @brief:
    *  Each 32bits register can accomodate 10 gpios FSEL, hence division by 10.
    *  gpios (0-9), (10-19), (20-29), (30-39), (40-49) and (50-53) will be 
    *  programmed in register FSEL0, FSEL1, FSEL2, FSEL3, FSEL4 and FSEL5 respectively.
    */		
    m_register[(gpio_n / 10)] &= (~(7U << ((gpio_n % 10U) * 3U)));
}

void GPIO::GPCLRn(gpio_number gpio_n) {
    if(gpio_n < 32) {
        /* sets the respective bit */					
        m_register[Register::BCM2837_GPCLR0] &= (~(1U << gpio_n));
	} else {
        /*set the bits 0 - 21 for GPIO greater than 31 */					
        m_register[Register::BCM2837_GPCLR1] &= (~(1U << (gpio_n - 32)));
	}
}

void GPIO::GPSETn(gpio_number gpio_n) {
  
    if(gpio_n < 32) {
        m_register[Register::BCM2837_GPSET0] |= (1U << gpio_n);
    } else {
        /*set the bits 0 - 21 for GPIO greater than 31 */
        m_register[Register::BCM2837_GPSET1] |= (1U << (gpio_n - 32));
	}
}

std::uint32_t GPIO::GPGETn(gpio_number gpio_n) {
  
    if(gpio_n < 32) {
        return((m_register[Register::BCM2837_GPSET0]  >> gpio_n) & 01U);
    } else {
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((m_register[Register::BCM2837_GPSET1] >> (gpio_n - 32)) & 01U);
	}
}


















#endif /*__gpio_cpp__*/