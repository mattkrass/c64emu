#ifndef INCLUDED_IO_CONTROLLER_H
#define INCLUDED_IO_CONTROLLER_H

#include <stdint.h>

namespace MOS6510 {

class MemoryController;

struct IOControllerRegisterFile {
    union {
        struct {
            uint8_t dataPortA;
            uint8_t dataPortB;
            uint8_t dataDirA;
            uint8_t dataDirB;
            uint8_t timerALo;
            uint8_t timerAHi;
            uint8_t timerBLo;
            uint8_t timerBHi;
            uint8_t rtcTenths;
            uint8_t rtcSeconds;
            uint8_t rtcMinutes;
            uint8_t rtcHours;
            uint8_t serialShift;
            uint8_t intCtrlStat;
            uint8_t timerACtrl;
            uint8_t timerBCtrl;
        } reg;
        uint8_t all[16];
    };
};

class IOController {
private:
    IOControllerRegisterFile    m_CIA1Registers;
    IOControllerRegisterFile    m_CIA2Registers;
    MemoryController*           m_memory;
    uint8_t                     m_matrix[8];
    uint8_t                     m_serialByteOut;
    uint8_t                     m_serialBitOut;
    bool                        m_ackPending;

public:
    IOController(MemoryController *memPtr);
    ~IOController();
    
    uint8_t read(uint16_t addr);
    void    write(uint16_t addr, uint8_t data, uint16_t pc);

    void    setKeyDown(int key);
    void    setKeyUp(int key);
    void    serialEvent(uint8_t changed);

    void    init();
    void    execute();
};

}

#endif
