#ifndef __memory_map_hpp__
#define __memory_map_hpp__

#include <cstdint>
#include <cstdio>
#include <atomic>

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

};

struct _InterruptRegisterAddress {

};












#endif /*__memory_map_hpp__*/