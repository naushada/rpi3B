#ifndef __memory_map_hpp__
#define __memory_map_hpp__

#include <cstdint>
#include <cstdio>

/**
 * @brief Peripheral physical base for the *bare-metal* default placement
 *        addresses below (the `operator new` fallbacks used when no region
 *        pointer is supplied). Defaults to BCM2836/7 (Pi 2/3/Zero 2 W); override
 *        at compile time to retarget a freestanding build:
 *
 *          -DBCM_PERIPH_BASE=0x20000000   // BCM2835: Pi 1, Pi Zero, Pi Zero W
 *          -DBCM_PERIPH_BASE=0x3F000000   // BCM2836/7: Pi 2, Pi 3, Pi Zero 2 W
 *          -DBCM_PERIPH_BASE=0xFE000000   // BCM2711: Pi 4, Pi 400, CM4
 *
 * The booted-Linux path (inc/mmio.hpp) ignores this and auto-detects the base
 * at runtime, so a single Linux binary already targets every supported board.
*/
#ifndef BCM_PERIPH_BASE
#define BCM_PERIPH_BASE 0x3F000000U
#endif

/**
 * @brief Physical base of the per-core **ARM local peripherals** block (the
 *        "QA7" register file: per-core timer/mailbox interrupt control + IRQ/FIQ
 *        source). This is a SEPARATE address space from BCM_PERIPH_BASE — it is
 *        NOT in the BCM283x peripheral window and does not move with it:
 *
 *          0x40000000   // BCM2836/7: Pi 2 / Pi 3 / Pi Zero 2 W   (this default)
 *          0xFF800000   // BCM2711:   Pi 4 / Pi 400 / CM4
 *
 * The block does not exist on BCM2835 (Pi 1 / Zero), which is single-core with
 * no local interrupt routing. See ARMv8-A on the Pi 3: the AArch64 generic-timer
 * IRQ (CNTPNSIRQ) and the legacy controller's combined output are both routed to
 * a core through this block (see inc/interrupt.hpp dispatch()).
*/
#ifndef BCM_LOCAL_BASE
#define BCM_LOCAL_BASE 0x40000000U
#endif

namespace BCM2837 {

    /**
     * @brief One 32-bit memory-mapped I/O register.
     *
     * Plain `volatile` storage — NOT `std::atomic`. For a word-aligned MMIO
     * register a volatile load/store is the correct primitive; atomics add no
     * value over the peripheral bus (see docs/DRIVER_REVIEW.md §2.1). Two
     * properties matter for the real-hardware path:
     *
     *  1. **Trivially default-constructible.** No user-declared constructor, so
     *     placement-new'ing a register block over *live* MMIO does NOT run any
     *     element constructor and therefore does NOT clobber the registers.
     *     (C++20 `std::atomic`'s default constructor value-initialises to 0,
     *     which would zero every register on attach.)
     *  2. **No deprecated volatile compound-assignment.** The compound operators
     *     below do an explicit volatile load / modify / store, so callers can
     *     keep writing `reg |= bit;` without tripping C++20's `-Wdeprecated-
     *     volatile` on `volatile`-qualified `|=`/`&=`.
     *
     * Layout-compatible with `uint32_t` (standard-layout, size/align 4), so the
     * unit tests can still overlay a `std::vector<std::uint32_t>` region.
    */
    struct mmio_reg {
        volatile std::uint32_t value;

        operator std::uint32_t() const { return value; }     ///< volatile read
        std::uint32_t load() const { return value; }
        void store(std::uint32_t v) { value = v; }

        mmio_reg& operator=(std::uint32_t v)  { value = v; return *this; }
        mmio_reg& operator|=(std::uint32_t v) { value = value | v; return *this; }
        mmio_reg& operator&=(std::uint32_t v) { value = value & v; return *this; }
        mmio_reg& operator^=(std::uint32_t v) { value = value ^ v; return *this; }
    };

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
    
        using device_register = mmio_reg; //@brief One MMIO register (see BCM2837::mmio_reg)
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
                /**
                 * @brief
                 *      Physical addresses range from 0x3F000000 to 0x3FFFFFFF for peripherals. The
                        bus addresses for peripherals are set up to map onto the peripheral bus address range
                        starting at 0x7E000000. Thus a peripheral advertised here at bus address 0x7Ennnnnn is
                        available at physical address 0x3Fnnnnnn.
                */
                return reinterpret_cast<void *>((BCM_PERIPH_BASE) + (0x00200000U));
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

        enum Register : std::uint32_t {
            CM_GP0CTL,
            CM_GP0DIV,
            CM_GP1CTL,
            CM_GP1DIV,
            CM_GP2CTL,
            CM_GP2DIV,
            CM_GPn_MAX
        };
        
        enum CM_GPnCTL_Type: std::uint32_t {
            SRC,
            ENAB,
            KILL,
            Unused,
            BUSY,
            FLIP,
            MASH,
            ALL
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

        enum ClockDivisor: std::uint32_t {
            /**
             * @brief
             *      Fractional part of divisor
                    To avoid lock-ups and glitches do not change this
                    control while BUSY=1.
            */
            DIVF,
            /**
             * @brief
             *      Integer part of divisor
                    This value has a minimum limit determined by the
                    MASH setting. See text for details. To avoid lock-ups
                    and glitches do not change this control while BUSY=1.
            */
            DIVI,
            /**
             * @brief
             *      Use BOTH_VALUE to write both Integer & Fractional part
            */
            BOTH_VALUE
        };

        using device_register = mmio_reg; //@brief One MMIO register (see BCM2837::mmio_reg)
        ClockRegistersAddress() {
            /*
            for(auto idx = 0; idx < Register::CM_GPn_MAX; ++idx)
                std::printf("Clock Register is 0x%X\n", &m_register[idx]);
            std::printf("\n");
            */
        }
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
                return reinterpret_cast<void *>((BCM_PERIPH_BASE) + (0x00100000U));
            }
            return(region);
        }

        /** 
         * @brief
         * address for m_register will be 
         * [0x3F100000, 0x3F100004, 0x3F100008, 0x3F10000C, 0x3F100010, 0x3F100014, 0x3F100018, 0x3F10001C  ... 0x3F1000B0] 
         **/
        device_register m_register[Register::CM_GPn_MAX];
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

        
        using device_register = mmio_reg; //@brief One MMIO register (see BCM2837::mmio_reg)
        InterruptRegisterAddress() {
            for(auto idx = 0; idx < Register::IRQs_ALL_MAX; ++idx)
                std::printf("Interrupt Register is 0x%X\n", &m_register[idx]);
            std::printf("\n");
        }
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
                return reinterpret_cast<void *>((BCM_PERIPH_BASE) + (0x0000B200U));
            }
            return(region);
        }

        device_register* operator[](Register offset) {
            return(&m_register[offset]);
        }

        /** 
         * @brief
         * address for m_register will be 
         * [0x3F00B200, 0x3F00B204, 0x3F00B208, 0x3F00B20C, 0x3F00B210, 0x3F00B214, 0x3F00B218, 0x3F00B21C  ... 0x3F00B220, 0x3F00B224] 
         **/
        device_register m_register[Register::IRQs_ALL_MAX];

    };

    /**
     * @brief
     *      Per-core **ARM local peripherals** ("QA7") register block at
     *      BCM_LOCAL_BASE (0x40000000 on BCM2836/7). This is where the AArch64
     *      generic-timer interrupts and the legacy interrupt controller's
     *      combined "GPU IRQ" line are routed to a specific core — the piece the
     *      classic AArch32 IVT model omitted entirely. Registers are word-indexed
     *      by their datasheet byte offset / 4 (the repo convention). The four
     *      per-core variants are contiguous, so register N for core c is
     *      `CoreN_base + c` (e.g. CORE_IRQ_SOURCE0 + 2 == core 2's IRQ source).
     *
     *      Reference: "Quad-A7 control" (BCM2836 ARM-local peripherals) §4.
    */
    struct ArmLocalRegisterAddress {
        enum Register : std::uint32_t {
            CONTROL              = 0x00 / 4,  /* 0x00: control (timer source etc.) */
            CORE_TIMER_PRESCALER = 0x08 / 4,  /* 0x08: core timer prescaler        */
            GPU_INT_ROUTING      = 0x0C / 4,  /* 0x0C: route GPU IRQ/FIQ to a core  */
            PMU_INT_ROUTING_SET  = 0x10 / 4,  /* 0x10: PMU interrupt routing set     */
            PMU_INT_ROUTING_CLR  = 0x14 / 4,  /* 0x14: PMU interrupt routing clear   */
            CORE_TIMER_LS        = 0x1C / 4,  /* 0x1C: 64-bit core timer, low word   */
            CORE_TIMER_MS        = 0x20 / 4,  /* 0x20: 64-bit core timer, high word  */
            LOCAL_INT_ROUTING    = 0x24 / 4,  /* 0x24: local timer IRQ/FIQ routing   */
            LOCAL_TIMER_CONTROL  = 0x34 / 4,  /* 0x34: local timer control & status  */
            LOCAL_TIMER_WRITE    = 0x38 / 4,  /* 0x38: local timer IRQ clear/reload  */
            CORE_TIMER_IRQCNTL0  = 0x40 / 4,  /* 0x40..0x4C: per-core timer IRQ ctrl */
            CORE_MBOX_IRQCNTL0   = 0x50 / 4,  /* 0x50..0x5C: per-core mailbox IRQ ctl*/
            CORE_IRQ_SOURCE0     = 0x60 / 4,  /* 0x60..0x6C: per-core IRQ source     */
            CORE_FIQ_SOURCE0     = 0x70 / 4,  /* 0x70..0x7C: per-core FIQ source     */
            ARM_LOCAL_MAX        = 0x80 / 4   /* one word past CORE_FIQ_SOURCE3      */
        };

        /**
         * @brief Bit positions shared by CORE_TIMER_IRQCNTLn (enables) and
         *        CORE_IRQ_SOURCEn / CORE_FIQ_SOURCEn (pending decode). The low
         *        nibble is the four generic-timer lines; in CORE_TIMER_IRQCNTLn
         *        bits 0-3 are the IRQ enables and bits 4-7 the FIQ enables.
         *        CNTPNSIRQ (bit 1) is the physical non-secure timer an AArch64
         *        EL1 kernel uses for its tick.
        */
        enum Source : std::uint32_t {
            CNTPSIRQ  = 0,   /* physical secure timer        */
            CNTPNSIRQ = 1,   /* physical non-secure timer    */
            CNTHPIRQ  = 2,   /* hypervisor (EL2) timer       */
            CNTVIRQ   = 3,   /* virtual timer                */
            MAILBOX0  = 4,
            MAILBOX1  = 5,
            MAILBOX2  = 6,
            MAILBOX3  = 7,
            GPU       = 8,   /* combined legacy-controller output (one core only) */
            PMU       = 9,
            AXI_QUIET = 10,  /* core 0 only                   */
            LOCALTIMER = 11
        };

        using device_register = mmio_reg;
        ArmLocalRegisterAddress() {}
        ~ArmLocalRegisterAddress() {}

        /**
         * @brief Placement-new seam: overlay on the live QA7 block at
         *        BCM_LOCAL_BASE on hardware, or on a caller-supplied buffer in
         *        tests. Mirrors InterruptRegisterAddress::operator new.
        */
        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                return reinterpret_cast<void *>(BCM_LOCAL_BASE);
            }
            return(region);
        }

        device_register* operator[](Register offset) {
            return(&m_register[offset]);
        }

        device_register m_register[Register::ARM_LOCAL_MAX];
    };

    /**
     * @brief
     *      Broadcom Serial Controller (BSC) register block — the BCM2837 I2C
     *      master. Refer Section 3 "BSC" of BCM2837-ARM-Peripherals.pdf.
     *      The Pi exposes BSC1 on the 40-pin header (GPIO2=SDA1, GPIO3=SCL1,
     *      ALT0). BSC0 is at 0x3F205000, BSC1 at 0x3F804000 (bus 0x7E804000),
     *      BSC2 (0x3F805000) is reserved for HDMI. Registers are spaced 4 bytes
     *      apart, indexed directly by datasheet word offset (no pin mapping).
    */
    struct BSCRegistersAddress {

        enum Register : std::uint32_t {
            C,      /* 0x00 Control                */
            S,      /* 0x04 Status                 */
            DLEN,   /* 0x08 Data Length            */
            A,      /* 0x0C Slave Address          */
            FIFO,   /* 0x10 Data FIFO              */
            DIV,    /* 0x14 Clock Divider          */
            DEL,    /* 0x18 Data Delay             */
            CLKT,   /* 0x1C Clock Stretch Timeout  */
            BSC_MAX
        };

        /// @brief Fields of the Control register (C).
        enum Control : std::uint32_t {
            READ,   /* bit0     : 1=Read transfer, 0=Write transfer */
            CLEAR,  /* bits[5:4]: write 0b01/0b10 to clear the FIFO  */
            ST,     /* bit7     : Start a new transfer               */
            INTD,   /* bit8     : Interrupt on DONE                  */
            INTT,   /* bit9     : Interrupt on TX                    */
            INTR,   /* bit10    : Interrupt on RX                    */
            I2CEN,  /* bit15    : BSC controller enable              */
            C_ALL
        };

        /// @brief Fields of the Status register (S). DONE/ERR/CLKT are W1C.
        enum Status : std::uint32_t {
            TA,     /* bit0  : Transfer Active            */
            DONE,   /* bit1  : Transfer Done (W1C)        */
            TXW,    /* bit2  : FIFO needs Writing         */
            RXR,    /* bit3  : FIFO needs Reading         */
            TXD,    /* bit4  : FIFO can accept data       */
            RXD,    /* bit5  : FIFO contains data         */
            TXE,    /* bit6  : FIFO empty                 */
            RXF,    /* bit7  : FIFO full                  */
            ERR,    /* bit8  : ACK error (W1C)            */
            CLKT_TO,/* bit9  : Clock stretch timeout (W1C)*/
            S_ALL
        };

        using device_register = mmio_reg;
        BSCRegistersAddress() {}
        ~BSCRegistersAddress() {}

        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                /* BSC1 physical base (bus 0x7E804000 -> phys 0x3F804000). */
                return reinterpret_cast<void *>(0x3F804000U);
            }
            return(region);
        }

        device_register m_register[Register::BSC_MAX];
    };

    /**
     * @brief
     *      SPI0 master register block. Refer Section 10 "SPI" of
     *      BCM2837-ARM-Peripherals.pdf. SPI0 is at bus 0x7E204000 ->
     *      phys 0x3F204000 and is exposed on GPIO7..11 (ALT0):
     *      GPIO8=CE0, GPIO7=CE1, GPIO9=MISO, GPIO10=MOSI, GPIO11=SCLK.
    */
    struct SPIRegistersAddress {

        enum Register : std::uint32_t {
            CS,     /* 0x00 Control and Status     */
            FIFO,   /* 0x04 TX/RX FIFO             */
            CLK,    /* 0x08 Clock divider          */
            DLEN,   /* 0x0C Data length (DMA)      */
            LTOH,   /* 0x10 LoSSI output hold delay*/
            DC,     /* 0x14 DMA DREQ controls      */
            SPI_MAX
        };

        /// @brief Fields of the CS (control/status) register.
        enum ControlStatus : std::uint32_t {
            CS_LINE,  /* bits[1:0] : Chip Select (which CE line)            */
            CPHA,     /* bit2      : Clock Phase                            */
            CPOL,     /* bit3      : Clock Polarity                         */
            CLEAR,    /* bits[5:4] : Clear TX (0b01) / RX (0b10) FIFO       */
            CSPOL,    /* bit6      : Chip Select Polarity                   */
            TA,       /* bit7      : Transfer Active                        */
            DMAEN,    /* bit8      : DMA enable                             */
            INTD,     /* bit9      : Interrupt on DONE                      */
            INTR,     /* bit10     : Interrupt on RXR                       */
            ADCS,     /* bit11     : Auto Deassert Chip Select (DMA)        */
            REN,      /* bit12     : Read Enable (bidirectional mode)       */
            LEN,      /* bit13     : LoSSI enable                           */
            LMONO,    /* bit14     : unused (LoSSI)                         */
            TE_EN,    /* bit15     : unused                                 */
            DONE,     /* bit16     : Transfer Done (RO)                     */
            RXD,      /* bit17     : RX FIFO contains data (RO)             */
            TXD,      /* bit18     : TX FIFO can accept data (RO)           */
            RXR,      /* bit19     : RX FIFO needs Reading (RO)             */
            RXF,      /* bit20     : RX FIFO full (RO)                      */
            CSPOL0,   /* bit21     : Chip Select 0 Polarity                 */
            CSPOL1,   /* bit22     : Chip Select 1 Polarity                 */
            CSPOL2,   /* bit23     : Chip Select 2 Polarity                 */
            DMA_LEN,  /* bit24     : Enable DMA in LoSSI mode               */
            LEN_LONG, /* bit25     : Enable Long data word in LoSSI DMA     */
            CS_ALL
        };

        using device_register = mmio_reg;
        SPIRegistersAddress() {}
        ~SPIRegistersAddress() {}

        void *operator new(std::size_t nBytes, void *region=nullptr) {
            (void)nBytes;
            if(nullptr == region) {
                /* SPI0 physical base (bus 0x7E204000 -> phys base + 0x204000). */
                return reinterpret_cast<void *>((BCM_PERIPH_BASE) + (0x00204000U));
            }
            return(region);
        }

        device_register m_register[Register::SPI_MAX];
    };
}











#endif /*__memory_map_hpp__*/