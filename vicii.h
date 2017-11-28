#ifndef INCLUDED_VICII_H
#define INCLUDED_VICII_H

#include <SDL2/SDL.h>
#include <stdint.h>

namespace MOS6510 {
const int SCREEN_WIDTH  = 1030;
const int SCREEN_HEIGHT = 585;
const uint32_t BG_COLOR = 0xFF9083EC;
const uint32_t FG_COLOR = 0xFFAAFFEE;

class MemoryController;

struct VICIIRegisterFile {
    union {
        struct {
            uint8_t m0x;
            uint8_t m0y;
            uint8_t m1x;
            uint8_t m1y;
            uint8_t m2x;
            uint8_t m2y;
            uint8_t m3x;
            uint8_t m3y;
            uint8_t m4x;
            uint8_t m4y;
            uint8_t m5x;
            uint8_t m5y;
            uint8_t m6x;
            uint8_t m6y;
            uint8_t m7x;
            uint8_t m7y;
            uint8_t msbx;
            uint8_t ctrl1;
            uint8_t raster;
            uint8_t lpx;
            uint8_t lpy;
            uint8_t spriteEnable;
            uint8_t ctrl2;
            uint8_t spriteYExpansion;
            uint8_t memoryPointers;
            uint8_t interruptFlags;
            uint8_t interruptEnable;
            uint8_t spriteDataPriority;
            uint8_t spriteMulticolor;
            uint8_t spriteXExpansion;
            uint8_t spriteSpriteCollision;
            uint8_t spriteDataCollision;
            uint8_t borderColor;
            uint8_t backgroundColor0;
            uint8_t backgroundColor1;
            uint8_t backgroundColor2;
            uint8_t backgroundColor3;
            uint8_t spriteMulticolor0;
            uint8_t spriteMulticolor1;
            uint8_t spriteColor0;
            uint8_t spriteColor1;
            uint8_t spriteColor2;
            uint8_t spriteColor3;
            uint8_t spriteColor4;
            uint8_t spriteColor5;
            uint8_t spriteColor6;
            uint8_t spriteColor7;
        } reg;
        uint8_t all[47];
    };
};

class VICII {
private:
    VICIIRegisterFile   m_registers;
    MemoryController*   m_memory;
    uint16_t            m_rasterTrigger;
    uint8_t             m_xCycle;
    SDL_Window *        m_window;
    SDL_Surface *       m_surface;
    uint8_t*            m_cgromPtr;

    uint16_t getRasterLine();
    void setRasterLine(uint16_t rasterLine);

public:
    VICII(MemoryController *memPtr, uint8_t *cgromPtr);
    ~VICII();
    
    uint8_t read(uint8_t addr);
    void write(uint8_t addr, uint8_t data);

    void init();
    void execute();
};

}

#endif
