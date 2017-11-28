#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "vicii.h"
#include "memorycontroller.h"

namespace MOS6510 {

const uint32_t colors[] =
    { 0xFF000000,
      0xFFFFFFFF,
      0xFF880000,
      0xFFAAFFEE,
      0xFFCC44CC,
      0xFF00CC55,
      0xFF0000AA,
      0xFFEEEE77,
      0xFFDD88EE,
      0xFF664400,
      0xFFFF7777,
      0xFF333333,
      0xFF777777,
      0xFFAAFF66,
      0xFF0088FF,
      0xFFBBBBBB };

uint16_t VICII::getRasterLine()
{
    uint16_t raster = (0x80 == (0x80 & m_registers.reg.ctrl1)) ? 0x100 : 0;
    raster |= m_registers.reg.raster;
    return raster;
}

void VICII::setRasterLine(uint16_t rasterLine)
{
    m_registers.reg.raster = rasterLine & 0xFF;
    if(0x100 & rasterLine) {
        m_registers.reg.ctrl1 |= 0x80;
    } else {
        m_registers.reg.ctrl1 &= 0x7F;
    }
}

VICII::VICII(MemoryController *memPtr, uint8_t *cgromPtr)
    : m_memory(memPtr)
    , m_cgromPtr(cgromPtr)
{
    assert(m_memory);
    init();
    m_memory->registerVic(this);
}

VICII::~VICII()
{
    SDL_DestroyWindow(m_window);
}

uint8_t VICII::read(uint8_t addr)
{
    if(47 > addr) {
        return m_registers.all[addr];
    } else if(63 < addr) {
        return read(addr - 64);
    }

    return 0xFF;
}

void VICII::write(uint8_t addr, uint8_t data)
{
    if(47 > addr) {
        m_registers.all[addr] = data;
    } else if(63 < addr) {
        write(addr - 64, data);
    }
}

int mapKeyToC64(SDL_Keycode key)
{
    switch(key) {
        case SDLK_BACKSPACE     : return 0;
        case SDLK_RETURN        : return 1;
        case SDLK_RIGHT         : return 2;
        case SDLK_F7            : return 3;
        case SDLK_F1            : return 4;
        case SDLK_F3            : return 5;
        case SDLK_F5            : return 6;
        case SDLK_DOWN          : return 7;
        case SDLK_3             : return 8;
        case SDLK_w             : return 9;
        case SDLK_a             : return 10;
        case SDLK_4             : return 11;
        case SDLK_z             : return 12;
        case SDLK_s             : return 13;
        case SDLK_e             : return 14;
        case SDLK_LSHIFT        : return 15;
        case SDLK_5             : return 16;
        case SDLK_r             : return 17;
        case SDLK_d             : return 18;
        case SDLK_6             : return 19;
        case SDLK_c             : return 20;
        case SDLK_f             : return 21;
        case SDLK_t             : return 22;
        case SDLK_x             : return 23;
        case SDLK_7             : return 24;
        case SDLK_y             : return 25;
        case SDLK_g             : return 26;
        case SDLK_8             : return 27;
        case SDLK_b             : return 28;
        case SDLK_h             : return 29;
        case SDLK_u             : return 30;
        case SDLK_v             : return 31;
        case SDLK_9             : return 32;
        case SDLK_i             : return 33;
        case SDLK_j             : return 34;
        case SDLK_0             : return 35;
        case SDLK_m             : return 36;
        case SDLK_k             : return 37;
        case SDLK_o             : return 38;
        case SDLK_n             : return 39;
        case SDLK_LEFTBRACKET   : return 40;
        case SDLK_p             : return 41;
        case SDLK_l             : return 42;
        case SDLK_MINUS         : return 43;
        case SDLK_PERIOD        : return 44;
        case SDLK_QUOTE         : return 45;
        case SDLK_AT            : return 46;
        case SDLK_COMMA         : return 47;
        case SDLK_BACKSLASH     : return 48;
        case SDLK_ASTERISK      : return 49;
        case SDLK_SEMICOLON     : return 50;
        case SDLK_HOME          : return 51;
        case SDLK_RSHIFT        : return 52;
        case SDLK_EQUALS        : return 53;
        case SDLK_CARET         : return 54;
        case SDLK_SLASH         : return 55;
        case SDLK_1             : return 56;
        case SDLK_LEFT          : return 57;
        case SDLK_LCTRL         : return 58;
        case SDLK_2             : return 59;
        case SDLK_SPACE         : return 60;
        case SDLK_RCTRL         : return 61;
        case SDLK_q             : return 62;
        case SDLK_TAB           : return 63;
    };

    return -1;
}

void VICII::execute()
{
    uint16_t raster = getRasterLine();
    uint16_t xCoord = m_xCycle++ * 8;
    uint32_t* pixelPtr = ((uint32_t*)m_surface->pixels);
    uint32_t color = 0;
    const uint32_t bdColor = colors[m_registers.reg.borderColor];
    const uint32_t bgColor = colors[m_registers.reg.backgroundColor0];
    for(int ix = 0; ix < 8; ++ix) {
        if(13 < raster && 249 > raster) { // within the non-blanked portion
            uint16_t x = xCoord + ix;
            if(49 < x && 462 > x) {
                if(30 < raster && 230 > raster) { // non-border
                    if(95 < x && 416 > x) {
                        uint8_t sx = (x - 96) / 8;
                        uint8_t sy = (raster - 30) / 8;
                        uint8_t sl = (raster - 30) % 8;
                        uint16_t sxy = 0x0400 + ((40 * sy) + sx);
                        uint16_t cxy = 0xD800 + ((40 * sy) + sx);
                        uint8_t ch = m_memory->read(sxy);
                        uint8_t pixels = m_cgromPtr[(ch * 8) + sl];
                        uint8_t mask = (1 << (7 - ((x - 96) % 8)));
                        if(mask == (mask & pixels)) {
                            color = colors[m_memory->read(cxy)];
                        } else {
                            color = bgColor;
                        }
                    } else {
                        color = bdColor;
                    }
                } else {
                    color = bdColor;
                }

                uint32_t idx = (x - 50) + ((raster - 14) * 412);
                pixelPtr[idx] = color;
            }
        }
    }

    if(64 <= m_xCycle) {
        // next raster line
        setRasterLine(++raster);
        m_xCycle = 0;
    }

    if(262 < raster) { // crossed end of screen, transfer SDL buffer to window now
        SDL_Event evt;
        while(SDL_PollEvent(&evt)) {
            if(SDL_KEYDOWN == evt.type || SDL_KEYUP == evt.type) {
                SDL_Keycode keycode = evt.key.keysym.sym;
                int ckey = mapKeyToC64(keycode);
                if(SDLK_ESCAPE == keycode) {
                    exit(0);
                } else if(-1 != ckey) {
                    if(SDL_KEYDOWN == evt.type) {
                        m_memory->setKeyDown(ckey);
                    } else {
                        m_memory->setKeyUp(ckey);
                    }
                }
            }
        }
        setRasterLine(0);
        SDL_Rect tgt; tgt.x = 0; tgt.y = 0; tgt.w = SCREEN_WIDTH, tgt.h = SCREEN_HEIGHT;
        SDL_BlitScaled(m_surface, 0, SDL_GetWindowSurface(m_window), &tgt);
        SDL_UpdateWindowSurface(m_window);
    }
}

void VICII::init()
{
    int rc = SDL_Init(SDL_INIT_VIDEO);
    if (0 > rc) {
        std::cerr << "Failed to start SDL, rc = " << rc << std::endl;
        std::cerr << "SDL_Error = " << SDL_GetError() << std::endl;
        exit(rc);
    }

    if(SDL_IsTextInputActive()) {
        printf("Stopping text input for performance reasons!\n");
        SDL_StopTextInput();
    }

    m_window = SDL_CreateWindow("C64",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
    m_surface = SDL_CreateRGBSurface(0, 412, 234, 32, 0, 0, 0, 0);

    // set default color registers
    //m_memory.write(646, 14);
    m_registers.reg.backgroundColor0 = 6;
    m_registers.reg.borderColor = 14;
}

}
