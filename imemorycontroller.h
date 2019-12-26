#ifndef INCLUDED_IMEMORY_CONTROLLER_H
#define INCLUDED_IMEMORY_CONTROLLER_H
#include <stdint.h>
#include "vicii.h"
#include "iocontroller.h"

namespace MOS6510 {

class IMemoryController {
public:
    virtual uint8_t read(uint16_t addr) = 0;
    virtual uint16_t readWord(uint16_t addr) = 0;

    virtual void setKeyDown(int key) = 0;
    virtual void setKeyUp(int key) = 0;

    virtual void write(uint16_t addr, uint8_t data, uint16_t pc) = 0;
    virtual void writeWord(uint16_t addr, uint16_t word, uint16_t pc) = 0;
    virtual void registerVIC(VICII *vicPtr) = 0;
    virtual void registerIO(IOController *ioPtr) = 0;

    IMemoryController();
};
} // namespace MOS6510

#endif // INCLUDED_MEMORY_CONTROLLER_H
