#ifndef __interrupt_test_hpp__
#define __interrupt_test_hpp__


#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstring>

#include "interrupt.hpp"

class IRQTest : public ::testing::Test
{
    public:
        /**
         * @brief Memory region for IRQ instance will be taken from m_memory_region,
         *        the IRQ instance layout will be done from m_memory_region.
        */
        IRQTest() : m_memory_region(IRQ::region_words()),
                    m_irq(IRQ(m_memory_region.data())) {
            // The IRQ driver overlays THREE blocks on one region:
            // [interrupt registers][IVT (16 AArch64 vectors + dispatch table)]
            // [ARM-local registers]. IRQ::region_words() sizes the buffer to
            // cover all three — otherwise install_*Handler / dispatch write past
            // the end (a bus error). Zero after the overlay: the register/IVT
            // structs have trivial ctors (so they don't clobber live MMIO), which
            // lets -O2/-O3 drop the vector's value-init as a dead store; zeroing
            // here is observed by the driver's volatile reads and is not elided.
            std::memset(m_memory_region.data(), 0,
                        m_memory_region.size() * sizeof(std::uint32_t));
        }

        virtual ~IRQTest() override = default;

        virtual void SetUp() override;
        virtual void TearDown() override;
        virtual void TestBody() override;

        // All IRQ copies share the overlaid region (references), so registers,
        // the IVT and the dispatch table are common state across irq() calls.
        IRQ irq() const {
            return(m_irq);
        }

        /// Raw overlaid buffer, for scripting pending/source bits in dispatch
        /// tests. The ARM-local block starts at IRQ::kLocalWordOffset words.
        std::uint32_t* region() { return m_memory_region.data(); }
        void zero_region() {
            std::memset(m_memory_region.data(), 0,
                        m_memory_region.size() * sizeof(std::uint32_t));
        }

    private:
        std::vector<std::uint32_t> m_memory_region;
        IRQ m_irq;
};









#endif /*__interrupt_test_hpp__*/