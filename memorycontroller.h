#ifndef INCLUDED_MEMORY_CONTROLLER_H
#define INCLUDED_MEMORY_CONTROLLER_H

namespace MOS6510 {
enum BankControlSignals {
    LORAM   = 0x01,
    HIRAM   = 0x02,
    CHAREN  = 0x04
};

class MemoryController {
private:
    uint8_t     m_sram[65536];
    uint8_t     m_rom[16384];
    uint8_t     m_scanIdx;
    uint8_t     m_matrix[8];
    bool        m_screenDirty;

    bool        checkMask(uint8_t mask, uint8_t value);
public:
    uint8_t     read(uint16_t addr);
    uint16_t    readWord(uint16_t addr);

    void        write(uint16_t addr, uint8_t data);
    void        writeWord(uint16_t addr, uint16_t word);
    bool        resetScreenDirty();
    void        setKeyDown(int key);
    void        setKeyUp(int key);

    MemoryController(uint8_t *rom);
};
} // namespace MOS6510

#endif // INCLUDED_MEMORY_CONTROLLER_H
