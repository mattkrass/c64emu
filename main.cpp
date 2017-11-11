#include <iostream>
#include <fstream>
#include "mos6510.h"

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
    romFile.read((char *)rom, 0x4000); // Read BASIC in to RAM
    if(0x4000 != romFile.gcount()) {
        std::cerr << "Failed to read full ROM! Only got "
                  << romFile.gcount()
                  << " bytes!"
                  << std::endl;
    }

    printf("ROM[0xA000] = 0x%02X\n", rom[0]);
    printf("ROM[0xA001] = 0x%02X\n", rom[1]);
    MOS6510::Cpu mos6510(rom);
    mos6510.addBreakpoint(0xE5CD);
    mos6510.addBreakpoint(0xBDCD);
    while(1) {
        mos6510.execute();
    }

    return 0;
}
