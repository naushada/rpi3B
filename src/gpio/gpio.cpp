#ifndef __gpio_cpp__
#define __gpio_cpp__

#include <unordered_map>
#include <algorithm>

#include "gpio.hpp"

static const std::unordered_map<GPIO::gpio_number, GPIO::pin_number> gpioNo2PinNo = {
    /**
     * @brief pin_number value 0 means this pin is either, GROUND, 3.3V, 5V or DNC - Don't Connect
    */
    {0,   0}, {1,  28}, {2,   3}, {3,   5}, {4,   7}, {5,  29}, {6,  31}, {7,  26}, {8,  24}, {9,  21},
    {10, 19}, {11, 23}, {12, 32}, {13, 33}, {14,  8}, {15, 10}, {16, 36}, {17, 11}, {18, 12}, {19, 35},
    {20, 38}, {21, 40}, {22, 15}, {23, 16}, {24, 18}, {25, 22}, {26, 37}, {27, 13}, {28,  0}, {29,  0},
    {30,  0}, {31,  0}, {32,  0}, {33,  0}, {34,  0}, {35,  0}, {36,  0}, {37,  0}, {38,  0}, {39,  0},
    {40,  0}, {41,  0}, {42,  0}, {43,  0}, {44,  0}, {45,  0}, {46,  0}, {47,  0}, {48,  0}, {49,  0},
    {50,  0}, {51,  0}, {52,  0}, {53,  0}
};

void GPIO::write32(gpio_number gpio_n, std::uint32_t value) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        memory().m_register[pin_no/10] = value;
    }
}

void GPIO::write(gpio_number gpio_n, GPIORegistersAddress::Config cfg) {
    /**
     * @brief
     *      First clear those bits and then set to value config.
    */
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});
    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        memory().m_register[(pin_no / 10)] &= (~(07U << ((pin_no % 10) * 3U)));
        memory().m_register[(pin_no / 10)] |= (cfg << ((pin_no % 10) * 3U));
    }
}

std::uint32_t GPIO::read32(gpio_number gpio_n) {

    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});
    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;
        return(memory().m_register[pin_no/10]);
    }
    return(0);
}

std::uint32_t GPIO::read(gpio_number gpio_n) {

    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});
    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;
        return((memory().m_register[pin_no/10]  >> ((pin_no % 10U) * 3U)) & 0b111);
    }
    return(0);
}

std::uint32_t GPIO::GPLEVn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEV0] >> pin_no) & 1U);
        }
        return ((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEV1] >> (pin_no - 32) & 1U));
    }

    return(0);
}

void GPIO::GPLEVn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEV0] |= (1U << pin_no);
        }
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEV1] |= (1U << (pin_no - 32));
    }
}

void GPIO::output(gpio_number gpio_n) {

    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;
        /* @brief:
         *  Each 32bits register can accomodate 10 gpios FSEL, hence division by 10.
         *  gpios (0-9), (10-19), (20-29), (30-39), (40-49) and (50-53) will be 
         *  programmed in register FSEL0, FSEL1, FSEL2, FSEL3, FSEL4 and FSEL5 respectively.
         */
        memory().m_register[(pin_no / 10)] |= (1U << ((pin_no % 10) * 3U));
    }
}

void GPIO::input(gpio_number gpio_n) {

    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;
        /* @brief:
         *  Each 32bits register can accomodate 10 gpios FSEL, hence division by 10.
         *  gpios (0-9), (10-19), (20-29), (30-39), (40-49) and (50-53) will be 
         *  programmed in register FSEL0, FSEL1, FSEL2, FSEL3, FSEL4 and FSEL5 respectively.
         */
        memory().m_register[(gpio_n / 10)] &= (~(7U << ((gpio_n % 10U) * 3U)));
    }
}

void GPIO::GPCLRn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            /* sets the respective bit */					
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPCLR0] &= (~(1U << pin_no));
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */					
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPCLR1] &= (~(1U << (pin_no - 32)));
    }
}

void GPIO::GPSETn(gpio_number gpio_n) {
  
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPSET0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPSET1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPGETn(gpio_number gpio_n) {

    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPSET0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPSET1] >> (pin_no - 32)) & 01U);
    }
    return(0);
}

void GPIO::GPEDSn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPEDS0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPEDS1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPEDSn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPEDS0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPEDS1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPRENn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPREN0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPREN1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPRENn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPREN0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPREN1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPFENn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPFEN0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPFEN1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPFENn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPFEN0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPFEN1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPHENn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPHEN0] |= (1U << pin_no);
        } else {
            /*set the bits 0 - 21 for GPIO greater than 31 */
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPHEN1] |= (1U << (pin_no - 32));
        }
    }
}

std::uint32_t GPIO::GPHENn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPHEN0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPHEN1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPLENn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEN0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEN1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPLENn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEN0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPLEN1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPARENn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAREN0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAREN1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPARENn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAREN0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAREN1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPAFENn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAFEN0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAFEN1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPAFENn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAFEN0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPAFEN1] >> (pin_no - 32)) & 01U);
    }

    return(0);
}

void GPIO::GPPUD(gpio_number gpio_n, GPIORegistersAddress::PullUpDownConfig cfg) {
    memory().m_register[GPIORegistersAddress::Register::BCM2837_GPPUD] |= (3U & cfg);
}

std::uint32_t GPIO::GPPUD(gpio_number gpio_n) const {
    return(memory().m_register[GPIORegistersAddress::Register::BCM2837_GPPUD] & 3U);
}

void GPIO::GPPUDCLKn(gpio_number gpio_n) {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            memory().m_register[GPIORegistersAddress::Register::BCM2837_GPPUDCLK0] |= (1U << pin_no);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        memory().m_register[GPIORegistersAddress::Register::BCM2837_GPPUDCLK1] |= (1U << (pin_no - 32));
    }
}

std::uint32_t GPIO::GPPUDCLKn(gpio_number gpio_n) const {
    auto it = std::find_if(gpioNo2PinNo.begin(), gpioNo2PinNo.end(), [&](const auto& ent) -> bool {return(gpio_n == ent.first);});

    if(it != gpioNo2PinNo.end()) {
        auto pin_no = it->second;

        if(pin_no < 32) {
            return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPPUDCLK0]  >> pin_no) & 01U);
        }
        /*set the bits 0 - 21 for GPIO greater than 31 */
        return((memory().m_register[GPIORegistersAddress::Register::BCM2837_GPPUDCLK1] >> (pin_no - 32)) & 01U);
    }
    return(0);
}



#endif /*__gpio_cpp__*/