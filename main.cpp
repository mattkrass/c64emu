#include <iostream>
#include <fstream>
#include <signal.h>
#include "mos6510.h"

bool g_setDebug = false;
void sig_callback(int signum)
{
    g_setDebug = true;
}

int main(int argc, char **argv)
{
    std::cout << "Hello World, booting up the MOS6510 now..."
              << std::endl;

    if (2 != argc) {
        std::cerr << "Need ROM filename!"
                  << std::endl;
        return -1;
    }

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
    signal(SIGTSTP, sig_callback);

    MOS6510::Cpu mos6510(rom);
    bool setDebug = true;
    while(1) {
        mos6510.execute(setDebug);
        if(setDebug) {
            g_setDebug = false;
        }
        setDebug = g_setDebug;
    }

    return 0;
}
