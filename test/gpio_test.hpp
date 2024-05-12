#ifndef __gpio_test_hpp__
#define __gpio_test_hpp__


#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "gpio.hpp"

class GPIOTest : public ::testing::Test
{
    public:
        /**
         * @brief Memory region for GPIO instance will be taken from m_memory_region,
         *        the GPIO instance layout will be done from m_memory_region.
        */
        GPIOTest() : m_memory_region(GPIO::Register::BCM2837_MAX), m_gpio(*new(m_memory_region.data()) GPIO) {}
        virtual ~GPIOTest() override = default;
        
        virtual void SetUp() override;
        virtual void TearDown() override;
        virtual void TestBody() override;

        GPIO& gpio() const {
            return(m_gpio);
        }

    private:
        std::vector<std::uint32_t> m_memory_region;
        GPIO& m_gpio;
        std::unordered_map<std::string, std::string> m_gpio2PinMap;
};


#endif /*__gpio_test_hpp__*/