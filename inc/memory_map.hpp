#ifndef __memory_map_hpp__
#define __memory_map_hpp__

#include <cstdint>
#include <cstdio>
#include <atomic>

namespace RPi3B {
    struct GPIORegistersAddress {

        enum Config: std::uint32_t {
            InputMode = 0b000,
            OutputMode = 0b001,
            AlternateFunction0 = 0b100,
            AlternateFunction1 = 0b101,
            AlternateFunction2 = 0b110,
            AlternateFunction3 = 0b111,
            AlternateFunction4 = 0b011,
            AlternateFunction5 = 0b010
        };

        enum PullUpDownConfig: std::uint32_t {
            Off = 0b00,
            EnablePullDown = 0b01,
            EnablePullUp = 0b10,
            Reserved = 0b11
        };

        enum Register: std::uint32_t {
            /* GPIO Function Selection offset */
            BCM2837_GPFSEL0,
            BCM2837_GPFSEL1,
            BCM2837_GPFSEL2,
            BCM2837_GPFSEL3,
            BCM2837_GPFSEL4,
            BCM2837_GPFSEL5,
            BCM2837_GPFSEL_RESERVED,
            /* GPIO SET Address offset */
            BCM2837_GPSET0,
            BCM2837_GPSET1,
            BCM2837_GPSET_RESERVED,
            /* Offset for clearing the GPIO */
            BCM2837_GPCLR0,
            BCM2837_GPCLR1,
            BCM2837_GPCLR_RESERVED,
            /* Offset for GPIO LEV0/1 */
            BCM2837_GPLEV0,
            BCM2837_GPLEV1,
            BCM2837_GPLEV_RESERVED,
            BCM2837_GPEDS0,
            BCM2837_GPEDS1,
            BCM2837_GPEDS_RESERVED,
            BCM2837_GPREN0,
            BCM2837_GPREN1,
            BCM2837_GPREN_RESERVED,
            BCM2837_GPFEN0,
            BCM2837_GPFEN1,
            BCM2837_GPFEN_RESERVED,
            BCM2837_GPHEN0,
            BCM2837_GPHEN1,
            BCM2837_GPHEN_RESERVED,
            BCM2837_GPLEN0,
            BCM2837_GPLEN1,
            BCM2837_GPLEN_RESERVED,
            BCM2837_GPAREN0,
            BCM2837_GPAREN1,
            BCM2837_GPAREN_RESERVED,
            BCM2837_GPAFEN0,
            BCM2837_GPAFEN1,
            BCM2837_GPAFEN_RESERVED,
            BCM2837_GPPUD,
            BCM2837_GPPUDCLK0,
            BCM2837_GPPUDCLK1,
            BCM2837_RESERVED,
            BCM2837_TEST1,
            BCM2837_TEST2,
            BCM2837_TEST3,
            BCM2837_TEST4,
            BCM2837_MAX
        };
    
        using device_register = volatile std::atomic<std::uint32_t>; //@brief This ensures that this is a thread safe
        GPIORegistersAddress() {}
        ~GPIORegistersAddress() {}

        /** 
         * @brief Compiler give preference to this new operator over global new operator and invoke this new operator. 
         * @param nBytes Number of bytes to be allocated
         * @param region is the memory region to be return if present.
         * @return pointer to void
         */
        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                return reinterpret_cast<void *>((0x3F000000) + (0x00200000));
            }
            return(region);
        }

        /** 
         * @brief
         * address for m_register will be 
         * [0x3F200000, 0x3F200004, 0x3F200008, 0x3F20000C, 0x3F200010, 0x3F200014, 0x3F200018, 0x3F20001C  ... 0x3F2000B0] 
         **/
        device_register m_register[Register::BCM2837_MAX];
    };

    struct ClockRegistersAddress {

        enum AddressOffset : std::uint32_t {
            CM_GP0CTL,
            CM_GP0DIV,
            CM_GP1CTL,
            CM_GP1DIV,
            CM_GP2CTL,
            CM_GP2DIV,
            CM_GPn_MAX
        };

        enum ClockSource: std::uint32_t {
            Ground = 0,
            Oscillator = 1,
            Testdebug0 = 2,
            Testdebug1 = 3,
            PLLA = 4,
            PLLC = 5,
            PLLD = 6,
            HDMI_Auxiliary = 7
            /**
             * @brief
                    8-15 = GND
                    To avoid lock-ups and glitches do not change this
                    control while BUSY=1 and do not change this control
                    at the same time as asserting ENAB
            */
        };

        enum ClockMASH : std::uint32_t {
            Integer_division = 0,
            Stage1_MASH = 1,
            Stage2_MASH = 2,
            Stage3_MASH = 3
            /**
             * @brief
             *      To avoid lock-ups and glitches do not change this
                    control while BUSY=1 and do not change this control
                    at the same time as asserting ENAB.
            */
        };

        using device_register = volatile std::atomic<std::uint32_t>; //@brief This ensures that this is a thread safe
        ClockRegistersAddress() {}
        ~ClockRegistersAddress() {}

        /** 
         * @brief Compiler give preference to this new operator over global new operator and invoke this new operator. 
         * @param nBytes Number of bytes to be allocated
         * @param region is the memory region to be return if present.
         * @return pointer to void
         */
        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                return reinterpret_cast<void *>((0x3F000000) + (0x00100000));
            }
            return(region);
        }

        /** 
         * @brief
         * address for m_register will be 
         * [0x3F100000, 0x3F100004, 0x3F100008, 0x3F10000C, 0x3F100010, 0x3F100014, 0x3F100018, 0x3F10001C  ... 0x3F1000B0] 
         **/
        device_register m_register[AddressOffset::CM_GPn_MAX];
    };

    struct InterruptRegisterAddress {
        enum Register : std::uint32_t {
            IRQ_Basic_Pending,
            IRQ_Pending1,
            IRQ_Pending2,
            FIQ_Control,
            Enable_IRQs_1,
            Enable_IRQs_2,
            Enable_Basic_IRQs,
            Disable_IRQs_1,
            Disable_IRQs_2,
            Disable_Basic_IRQs,
            IRQs_ALL_MAX
        };

        
        using device_register = volatile std::atomic<std::uint32_t>; //@brief This ensures that this is a thread safe
        InterruptRegisterAddress() {}
        ~InterruptRegisterAddress() {}

        /** 
         * @brief Compiler give preference to this new operator over global new operator and invoke this new operator. 
         * @param nBytes Number of bytes to be allocated
         * @param region is the memory region to be return if present.
         * @return pointer to void
         */
        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                return reinterpret_cast<void *>(0x3F00B200);
            }
            return(region);
        }

        /** 
         * @brief
         * address for m_register will be 
         * [0x3F00B200, 0x3F00B204, 0x3F00B208, 0x3F00B20C, 0x3F00B210, 0x3F00B214, 0x3F00B218, 0x3F00B21C  ... 0x3F00B220, 0x3F00B224] 
         **/
        device_register m_register[Register::IRQs_ALL_MAX];

    };
}











#endif /*__memory_map_hpp__*/