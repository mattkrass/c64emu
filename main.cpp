#include <iostream>
#include <fstream>
#include <signal.h>
#include <SDL.h>
#include "mos6510.h"

bool g_setDebug = false;
void sig_callback(int signum)
{
    g_setDebug = true;
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
    bool setDebug = true;
    while(1) {
        mos6510.execute(setDebug);
        if(setDebug) {
            g_setDebug = false;
        }
        setDebug = g_setDebug;
    }

    SDL_Quit();
    return 0;
}
