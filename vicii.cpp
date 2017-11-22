#include "vicii.h"

namespace MOS6510 {

uint16_t VICII::getRasterLine()
{
    uint16_t raster = (0x80 & m_registers.reg.raster) ? 0x100 : 0;
    raster |= m_registers.reg.raster;
    return raster;
}

void VICII::setRasterLine(uint16_t rasterLine)
{
    m_registers.reg.raster = rasterLine & 0xFF;
    if(0x100 & rasterLine) {
        m_registers.reg.raster |= 0x80;
    } else {
        m_registers.reg.raster &= 0x7F;
    }
}

VICII::VICII(MemoryController *memPtr)
    : m_memory(memPtr)
{
    assert(m_memory);
}

VICII::~VICII()
{

}

uint8_t read(uint8_t addr)
{
    if(47 > addr) {
        return m_registers.all[addr];
    } else if(63 < addr) {
        return read(addr - 64);
    }

    return 0xFF;
}

void write(uint8_t addr, uint8_t data)
{
    if(47 > addr) {
        m_registers.all[addr] = data;
    } else if(63 < addr) {
        write(addr - 64, data);
    }
}

void VICII::execute()
{
    uint16_t raster = getRasterLine();
    uint16_t xCoord = m_xCycle * 8;
    for(int ix = 0; ix < 8; ++ix) {
        if(13 < raster && 263 > raster) { // within the non-blanked portion
            uint16_t x = xCoord + ix;
            if(49 < x && 460 > x) {
                if(30 < raster && 232 > raster) { // non-border
                    if(95 < x && 417 > x) {

                    } else {

                    }
                } else { // border

                }
            }
        }
    }

    if(262 < raster) { // crossed end of screen, transfer SDL buffer to window now

        raster = 0;
    }

    setRasterLine(raster);
}

}
