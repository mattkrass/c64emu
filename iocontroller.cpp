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
    if(SerialState::EOI == m_serialState) {
        if(9 < m_serialBitOut) {
            --m_serialBitOut;
        } else if(9 == m_serialBitOut) {
            if(0x80 & m_CIA2Registers.reg.dataPortA) {
                printf("Changing data port A2 from 0x%02X to 0x%02X\n",
                        m_CIA2Registers.reg.dataPortA, (m_CIA2Registers.reg.dataPortA & 0x7F));
                m_CIA2Registers.reg.dataPortA &= 0x7F;
                m_serialBitOut = 14;
            } else {
                printf("Changing data port A2 from 0x%02X to 0x%02X\n",
                        m_CIA2Registers.reg.dataPortA, (m_CIA2Registers.reg.dataPortA | 0x80));
                m_CIA2Registers.reg.dataPortA |= 0x80;
                m_serialBitOut = 0;
                m_serialState = SerialState::LISTEN;
            }
        }
    }
}

void IOController::init()
{
    for(size_t i = 0; i < 8; ++i) {
        m_matrix[i] = 0xFF;
    }
    
    m_serialBitOut = 0;
    m_serialByteOut = 0;
    m_serialState = SerialState::IDLE;
}

void IOController::setKeyDown(int key)
{
    m_matrix[(key / 8)] &= ~(1 << (key % 8));
}

void IOController::setKeyUp(int key)
{
    m_matrix[(key / 8)] |= (1 << (key % 8));
}

static const char* getState(const SerialState state)
{
    switch(state) {
        case SerialState::IDLE:                     return "IDLE";
        case SerialState::WAIT_FOR_COMMAND:         return "WAIT_FOR_COMMAND";
        case SerialState::WAIT_FOR_SECONDARY_ADDR:  return "WAIT_FOR_SECONDARY_ADDR";
        case SerialState::EOI:                      return "EOI";
        case SerialState::LISTEN:                   return "LISTEN";
        case SerialState::TALK:                     return "TALK";
    }

    return "Unknown";
}

void IOController::serialEvent(uint8_t changed)
{
    bool atn = (m_CIA2Registers.reg.dataPortA & 0x08);
    bool clk = (m_CIA2Registers.reg.dataPortA & 0x10);
    bool din = (m_CIA2Registers.reg.dataPortA & 0x80);
    bool dot = (m_CIA2Registers.reg.dataPortA & 0x20);
    SerialState oldState = m_serialState;

    switch(m_serialState) {
        case SerialState::IDLE: {
            if(atn && !clk) {
                m_CIA2Registers.reg.dataPortA |= 0x80;
                din = true;
                printf("0 Bus ack.\n");
                m_serialState = SerialState::WAIT_FOR_COMMAND;
            }
        } break;

        case SerialState::WAIT_FOR_COMMAND: {
            if((0x10 & changed) && !clk) {
                printf("Clock up, bit = %d\n", dot);
                m_serialByteOut >>= 1;
                m_serialByteOut |= dot ? 0x00: 0x80;
                ++m_serialBitOut;
                if(8 == m_serialBitOut) {
                    printf("Byte: 0x%02X\n", m_serialByteOut);
                    if(0x20 & m_serialByteOut) { // listen
                        m_serialState = SerialState::WAIT_FOR_SECONDARY_ADDR;
                        m_CIA2Registers.reg.dataPortA &= 0x7F;
                        din = false;
                    }

                    m_primaryAddress = m_serialByteOut & 0x1F;
                }
            }
        } break;

        case SerialState::WAIT_FOR_SECONDARY_ADDR: {
            if((0x10 & changed) && !clk) {
                if(8 == m_serialBitOut) {
                    m_CIA2Registers.reg.dataPortA |= 0x80;
                    din = true;
                    printf("1 Bus ack.\n");
                    m_serialBitOut = 0;
                } else {
                    printf("Clock up, bit = %d\n", !dot);
                    m_serialByteOut >>= 1;
                    m_serialByteOut |= dot ? 0x00: 0x80;
                    ++m_serialBitOut;
                    if(8 == m_serialBitOut) {
                        printf("Byte: 0x%02X\n", m_serialByteOut);
                        m_secondaryAddress = m_serialByteOut;
                        m_serialState = SerialState::EOI;
                        m_CIA2Registers.reg.dataPortA &= 0x7F;
                        din = false;
                    }
                }
            }
        } break;

        case SerialState::EOI: {
            if((0x10 & changed) && !clk) {
                if(8 == m_serialBitOut) {
                    m_CIA2Registers.reg.dataPortA |= 0x80;
                    din = true;
                    printf("2 Bus ack.\n");
                    m_serialBitOut = 14;
                }
            }
        } break;

        case SerialState::LISTEN: {
            if((0x10 & changed) && !clk) {
                if(8 == m_serialBitOut) {
                    m_CIA2Registers.reg.dataPortA |= 0x80;
                    din = true;
                    printf("3 Bus ack.\n");
                    m_serialBitOut = 0;
                } else {
                    m_CIA2Registers.reg.dataPortA |= 0x80;
                    din = true;
                    printf("Clock up, bit = %d\n", !dot);
                    m_serialByteOut >>= 1;
                    m_serialByteOut |= dot ? 0x00: 0x80;
                    ++m_serialBitOut;
                    if(8 == m_serialBitOut) {
                        printf("Byte: 0x%02X\n", m_serialByteOut);
                        m_CIA2Registers.reg.dataPortA &= 0x7F;
                        din = false;
                        if(atn) { // cmd
                            if(0x3F == m_serialByteOut) { // UNLISTEN
                                m_serialBitOut = 0;
                                m_serialState = SerialState::IDLE;
                                if(0xF0 == m_secondaryAddress) {
                                    char *filename = (char*)calloc(
                                            m_serialQueue.size() + 1,
                                            sizeof(char));
                                    uint8_t idx = 0;
                                    while(m_serialQueue.size()) {
                                        filename[idx++] = m_serialQueue.front();
                                        m_serialQueue.pop_front();
                                    }

                                    printf("Device %d requested to open file '%s'\n",
                                            m_primaryAddress,
                                            filename);
                                    free(filename);
                                }
                            }
                        } else {
                            m_serialQueue.push_back(m_serialByteOut);
                        }
                    }
                }
            }
        } break;

        case SerialState::TALK: {
        } break;

    }


    printf("ATN is %s, ", atn ? "low" : "high");
    printf("CLK is %s, ", clk ? "low" : "high");
    printf("DIN is %s, ", din ? "low" : "high");
    printf("DOT is %s, ", dot ? "low" : "high");
    printf("STATE is %s", getState(m_serialState));
    if(oldState != m_serialState) {
        printf(" (was %s)", getState(oldState));
    }
    printf("\n-----------------------\n");
}

#if 0
void IOController::serialEvent(uint8_t changed)
{
    bool atn = (m_CIA2Registers.reg.dataPortA & 0x08);

    if(0x08 & changed) {
        m_ackPending = atn;
        if(!atn) {
            m_CIA2Registers.reg.dataPortA |= 0x80;
        }
    }

    bool clk = (m_CIA2Registers.reg.dataPortA & 0x10);
    bool din = (m_CIA2Registers.reg.dataPortA & 0x80);
    bool dot = (m_CIA2Registers.reg.dataPortA & 0x20);

    printf("ATN is %s, ", atn ? "low" : "high");
    printf("CLK is %s, ", clk ? "low" : "high");
    printf("DIN is %s, ", din ? "low" : "high");
    printf("DOT is %s\n", dot ? "low" : "high");

    if((0x10 & changed) && !clk) { // rising edge
        if(!din && m_ackPending) {
            m_CIA2Registers.reg.dataPortA |= 0x80;
            m_ackPending = false;
            printf("Bus ack.\n");
        } else {
            printf("Got bit %d: %d\n", m_serialBitOut++, !dot);
            m_serialByteOut >>= 1;
            if(!dot) {
                m_serialByteOut |= 0x80;
            }

            if(8 == m_serialBitOut) {
                printf("Received byte: 0x%02X\n", m_serialByteOut);
                m_CIA2Registers.reg.dataPortA &= 0x7F;
                m_ackPending = true;
                m_serialBitOut = 0;
            }
        }
    }
    printf("-----------------------\n");
}
#endif

}
