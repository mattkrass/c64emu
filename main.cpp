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

    uint8_t sram[65536];
    for(size_t i; i < 65536; ++i) {
        sram[i] = 0;
    }

    std::cout << "Memory 0xD012: "
              << ((int)sram[0xd012])
              << std::endl;
    
    std::cout << "Using ROM: "
              << argv[1]
              << std::endl;

    std::ifstream romFile(
            argv[1],
            std::ifstream::binary);
    romFile.read((char *)sram, 0x2000); // Read BASIC in to RAM
    if(0x2000 != romFile.gcount()) {
        std::cerr << "Failed to read BASIC ROM! Only got "
                  << romFile.gcount()
                  << " bytes!"
                  << std::endl;
    }
    std::cout << "Memory 0xD012: "
              << ((int)sram[0xd012])
              << std::endl;

    romFile.read((char *)&sram[0xE000], 0x2000); // Read KERNAL in to RAM
    if(0x2000 != romFile.gcount()) {
        std::cerr << "Failed to read KERNAL ROM! Only got "
                  << romFile.gcount()
                  << " bytes!"
                  << std::endl;
    }
    std::cout << "Memory 0xD012: "
              << ((int)sram[0xd012])
              << std::endl;

    MOS6510::Cpu mos6510(sram);
    for(int i = 0; i < 80000; ++i) {
        mos6510.execute();
    }

    return 0;
}
