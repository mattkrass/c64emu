#include <stdexcept>
#include <cstring>
#include <cassert>
#include "memorycontroller.h"

namespace MOS6510 {
MemoryController::MemoryController(uint8_t *rom)
{
    assert(0 != rom);
    memcpy(&m_rom, rom, 16384);
    for(size_t i = 0; i < 65536; ++i) {
        m_sram[i] = 1;
    }
}

bool MemoryController::checkMask(uint8_t mask, uint8_t value)
{
    return (value & mask) == mask;
}

uint8_t MemoryController::read(uint16_t addr)
{
    uint8_t page = addr / 256;
    uint8_t modeFlags = m_sram[0x0001];
    if(page <= 15) {
        return m_sram[addr];
    } else if(page > 15 && page <= 127) {
        return m_sram[addr];
    } else if(page > 127 && page <= 159) {
        return m_sram[addr];
    } else if(page > 159 && page <= 191) {
        if(checkMask((BankControlSignals::LORAM | BankControlSignals::HIRAM), modeFlags)) {
            return m_rom[addr - 0xA000];
        } else {
            return m_sram[addr];
        }
    } else if(page > 191 && page <= 207) {
        return m_sram[addr];
    } else if(page > 207 && page <= 223) {
        if(0 == modeFlags || BankControlSignals::CHAREN == modeFlags) {
            return m_sram[addr];
        } else if(checkMask(BankControlSignals::CHAREN, modeFlags)) {
            if(page > 207 && page <= 211) { // VIC registers
                if(addr <= 0xD02E) {
                    return m_vicPtr->read(addr - 0xD000);
                } else if(addr > 0xD02E && addr < 0xD040) {
                    return 0xFF;
                } else if(addr < 0xD3FF) {
                    return m_vicPtr->read(addr - 0xD400);
                }
            } else if((220 == page) || (221 == page)) {
                return m_ioPtr->read(addr - 0xDC00);
            }

            return m_sram[addr]; // <-- this should be I/O devices, TODO!!!
        } else { // character rom
            return 0xFF; // <-- this is totally bogus but I'm tired, should be CHAR ROM
        }
    } else if(page > 223 && page <= 255) {
        if(checkMask(BankControlSignals::HIRAM, modeFlags)) {
            return m_rom[(addr - 0xE000) + 0x2000];
        } else {
            return m_sram[addr];
        }
    }
    
    // if we get here, we are probably in trouble, exception?
    throw std::runtime_error("Addressed invalid memory for read.");
    return 0;
}

uint16_t MemoryController::readWord(uint16_t addr)
{
    uint16_t word = read(addr + 1) << 8;
    word |= read(addr);
    return word;
}

void MemoryController::write(uint16_t addr, uint8_t data, uint16_t pc)
{
    uint8_t page = addr / 256;
    uint8_t modeFlags = m_sram[0x0001];
    if((page > 211 && page <= 215) || (page > 221 && page <= 223)) {
        if(0 == modeFlags || BankControlSignals::CHAREN == modeFlags) {
            m_sram[addr] = data;
        } else if(checkMask(BankControlSignals::CHAREN, modeFlags)) {
            if(0xDC00 == addr) {
            }
        } else { // character rom
            m_sram[addr] = data;
        }
    } else if(page > 207 && page <= 211) {
        if(addr > 0xD02E && addr < 0xD040) {
            return;
        } else if (addr >= 0xD040) {
            m_vicPtr->write(addr - 0xD040, data);
        } else {
            m_vicPtr->write(addr - 0xD000, data);
        }
    } else if((220 == page) || (221 == page)) {
        m_ioPtr->write(addr - 0xDC00, data, pc);
    } else {
        m_sram[addr] = data;
    }
}

void MemoryController::writeWord(uint16_t addr, uint16_t word, uint16_t pc)
{
    write(addr,      (word       & 0xFF), pc);
    write(addr + 1, ((word >> 8) & 0xFF), pc);
}

void MemoryController::setKeyDown(int key)
{
    m_ioPtr->setKeyDown(key);
}

void MemoryController::setKeyUp(int key)
{
    m_ioPtr->setKeyUp(key);
}

void MemoryController::registerVIC(VICII *vicPtr)
{
    m_vicPtr = vicPtr;
}

void MemoryController::registerIO(IOController *ioPtr)
{
    m_ioPtr = ioPtr;
}

} // namespace MOS6510
