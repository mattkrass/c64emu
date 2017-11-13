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

    m_screenDirty = false;
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
            return 0; // <-- this should be I/O devices, TODO!!!
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

void MemoryController::write(uint16_t addr, uint8_t data)
{
    uint8_t page = addr / 256;
    uint8_t modeFlags = m_sram[0x0001];
    if(page > 207 && page <= 223) {
        if(0 == modeFlags || BankControlSignals::CHAREN == modeFlags) {
            m_sram[addr] = data;
        } else if(checkMask(BankControlSignals::CHAREN, modeFlags)) {
            return; // <-- this should be I/O devices, TODO!!!
        } else { // character rom
            m_sram[addr] = data;
        }
    } else {
        if(page > 3 && page <= 8) { // video memory, mark it dirty
            m_screenDirty = true;
        }

        m_sram[addr] = data;
    }
}

void MemoryController::writeWord(uint16_t addr, uint16_t word)
{
    write(addr,      (word       & 0xFF));
    write(addr + 1, ((word >> 8) & 0xFF));
}

bool MemoryController::resetScreenDirty()
{
    bool tmp = m_screenDirty;
    m_screenDirty = false;
    return tmp;
}

} // namespace MOS6510
