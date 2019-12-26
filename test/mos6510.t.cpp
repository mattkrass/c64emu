#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#include "mos6510.h"

using MOS6510::Cpu;
using MOS6510::IMemoryController;

class MockMemController : public IMemoryController
{
    uint8_t read(uint16_t addr) = 0;
    uint16_t readWord(uint16_t addr) = 0;

    void setKeyDown(int key) = 0;
    void setKeyUp(int key) = 0;

    void write(uint16_t addr, uint8_t data, uint16_t pc) = 0;
    void writeWord(uint16_t addr, uint16_t word, uint16_t pc) = 0;
    void registerVIC(VICII *vicPtr) = 0;
    void registerIO(IOController *ioPtr) = 0;
};

TEST(DummyFixture, DummyTest)
{
    // GIVEN
    Cpu cpu;
    /*
void Cpu::adc()
{
    uint16_t result = ((uint16_t)m_accumulator) + m_operand + m_status.bits.carryFlag;
    m_status.bits.carryFlag = (result > 0xFF);
    m_accumulator = result & 0xFF;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}
*/
}
