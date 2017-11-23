#include <iostream>
#include <fstream>
#include <signal.h>
#include <SDL2/SDL.h>
#include "mos6510.h"
#include "memorycontroller.h"
#include <stdio.h>

bool g_setDebug = false;
void sig_callback(int signum)
{
    g_setDebug = true;
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
        case SDLK_PLUS          : return 40;
        case SDLK_p             : return 41;
        case SDLK_l             : return 42;
        case SDLK_MINUS         : return 43;
        case SDLK_PERIOD        : return 44;
        case SDLK_COLON         : return 45;
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

int main(int argc, char **argv)
{
    if (2 != argc) {
        std::cerr << "Need ROM filename!"
                  << std::endl;
        return -1;
    }

    std::cout << "Hello World, initializing SDL now..." << std::endl;
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

    std::cout << "Booting up the MOS6510 now..."
              << std::endl;

    std::cout << "Using ROM: "
              << argv[1]
              << std::endl;

    std::ifstream romFile(
            argv[1],
            std::ifstream::binary);

    uint8_t rom[16384];
    romFile.read((char *)rom, 0x4000); // Read BASIC/KERNAL in to buffer
    if(0x4000 != romFile.gcount()) {
        std::cerr << "Failed to read full ROM! Only got "
                  << romFile.gcount()
                  << " bytes!"
                  << std::endl;
    }

    printf("ROM[0xA000] = 0x%02X\n", rom[0]);
    printf("ROM[0xA001] = 0x%02X\n", rom[1]);
    std::ifstream cgromFile(
            "characters.901225-01.bin",
            std::ifstream::binary);

    uint8_t cgrom[4096];
    cgromFile.read((char *)cgrom, 0x1000); // Read CGROM in to buffer
    if(0x1000 != cgromFile.gcount()) {
        std::cerr << "Failed to read full CGROM! Only got "
                  << romFile.gcount()
                  << " bytes!"
                  << std::endl;
    }

    signal(SIGTSTP, sig_callback);

    MOS6510::Cpu mos6510(rom, cgrom);
    bool setDebug = false;
    while(1) {
        mos6510.execute(setDebug);
        if(setDebug) {
            g_setDebug = false;
        }
        setDebug = g_setDebug;
        SDL_Event evt;
        while(SDL_PollEvent(&evt)) {
            if(SDL_KEYDOWN == evt.type || SDL_KEYUP == evt.type) {
                SDL_Keycode keycode = evt.key.keysym.sym;
                int ckey = mapKeyToC64(keycode);
                if(SDLK_ESCAPE == keycode) {
                    exit(0);
                } else if(-1 != ckey) {
                    if(SDL_KEYDOWN == evt.type) {
                        mos6510.getMemory().setKeyDown(ckey);
                    } else {
                        mos6510.getMemory().setKeyUp(ckey);
                    }
                }
            }
        }
    }

    SDL_Quit();
    return 0;
}
