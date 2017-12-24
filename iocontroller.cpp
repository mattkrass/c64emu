#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "iocontroller.h"
#include "memorycontroller.h"

namespace MOS6510 {

IOController::IOController(MemoryController *memPtr)
    : m_memory(memPtr)
{
    assert(m_memory);
    init();
    m_memory->registerIO(this);
}

IOController::~IOController()
{

}

uint8_t IOController::read(uint16_t addr)
{
    uint8_t data = 0xFF;
    if(0xFF < addr) { // first CIA chip
        data = m_CIA2Registers.all[(addr % 0x10)];
    } else if(0x0001 == addr) {
        uint8_t mask = 0x01;
        for(size_t i = 0; i < 8; ++i) {
            if(mask != (m_CIA1Registers.reg.dataPortA & mask)) {
                data &= m_matrix[i];
            }
            mask <<= 1;
        }
    } else {
        data = m_CIA1Registers.all[(addr % 0x10)];
    }

    return data;
}

void IOController::write(uint16_t addr, uint8_t data, uint16_t pc)
{
    uint8_t tmp;
    uint8_t changed = 0;
    switch(addr) {
        case 0x00:
            tmp = (data & m_CIA1Registers.reg.dataDirA);
            tmp |= (m_CIA1Registers.reg.dataPortA & ~m_CIA1Registers.reg.dataDirA);
            m_CIA1Registers.reg.dataPortA = tmp;
            break;
        case 0x01:
            tmp = (data & m_CIA1Registers.reg.dataDirB);
            tmp |= (m_CIA1Registers.reg.dataPortB & ~m_CIA1Registers.reg.dataDirB);
            m_CIA1Registers.reg.dataPortB = tmp;
            break;
        case 0x02:
            m_CIA1Registers.reg.dataDirA = data;
            break;
        case 0x03:
            m_CIA1Registers.reg.dataDirB = data;
            break;
        case 0x100:
            tmp = (data & m_CIA2Registers.reg.dataDirA);
            tmp |= (m_CIA2Registers.reg.dataPortA & ~m_CIA2Registers.reg.dataDirA);
            changed = m_CIA2Registers.reg.dataPortA ^ tmp;
            m_CIA2Registers.reg.dataPortA = tmp;
            //printf("Data Port A = 0x%02X, changed = 0x%02X at PC = 0x%04X\n", tmp, changed, pc);
            if(0 != (changed & 0x38)) {
                serialEvent(changed);
            }
            break;
        case 0x101:
            tmp = (data & m_CIA2Registers.reg.dataDirB);
            tmp |= (m_CIA2Registers.reg.dataPortB & ~m_CIA2Registers.reg.dataDirB);
            changed = m_CIA2Registers.reg.dataPortB ^ tmp;
            m_CIA2Registers.reg.dataPortB = tmp;
            printf("Data Port B = 0x%02X, changed = 0x%02X at PC = 0x%04X\n", tmp, changed, pc);
            break;
        case 0x102:
            m_CIA2Registers.reg.dataDirA = data;
            printf("Data Dir  A = 0x%02X at PC = 0x%04X\n", data, pc);
            break;
        case 0x103:
            m_CIA2Registers.reg.dataDirB = data;
            printf("Data Dir  B = 0x%02X at PC = 0x%04X\n", data, pc);
            break;
    };
}

void IOController::execute()
{

}

void IOController::init()
{
    for(size_t i = 0; i < 8; ++i) {
        m_matrix[i] = 0xFF;
    }
    
    m_serialBitOut = 0;
    m_serialByteOut = 0;
}

void IOController::setKeyDown(int key)
{
    m_matrix[(key / 8)] &= ~(1 << (key % 8));
}

void IOController::setKeyUp(int key)
{
    m_matrix[(key / 8)] |= (1 << (key % 8));
}

void IOController::serialEvent(uint8_t changed)
{
    bool atn = (m_CIA2Registers.reg.dataPortA & 0x08);
    bool clk = (m_CIA2Registers.reg.dataPortA & 0x10);
    bool din = (m_CIA2Registers.reg.dataPortA & 0x80);
    bool dot = (m_CIA2Registers.reg.dataPortA & 0x20);

    if(0x08 & changed) {
        printf("ATN is now %s\n", atn ? "low" : "high");
        if(atn) {
            m_ackPending = true;
        }
    }
    printf("CLK is %s, ", clk ? "low" : "high");
    printf("DIN is %s, ", din ? "low" : "high");
    printf("DOT is %s\n", dot ? "low" : "high");

    if(!clk) {
        m_serialByteOut >>= 1;
        if(!dot) {
            m_serialByteOut |= 0x80;
        }

        printf("Fresh bit: %s, byte: 0x%02X\n", dot ? "0" : "1", m_serialByteOut);
        
        if(8 == ++m_serialBitOut) { // complete byte in buffer
            printf("Got byte: 0x%02X\n", m_serialByteOut);
            m_ackPending = true;
            m_serialBitOut = 0;
            m_serialByteOut = 0;
            m_CIA2Registers.reg.dataPortA &= 0x7F;
        }
    }

    if(m_ackPending) { // waiting for bus
        m_CIA2Registers.reg.dataPortA |= 0x80;
        printf("Bus ACK.\n\n");
        m_ackPending = false;
    }
}

}
