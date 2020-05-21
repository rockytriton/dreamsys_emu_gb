#include "cpu.h"
#include "memory.h"
#include "cart.h"
#include "emu.h"

#include <cstring>
#include <iomanip>

using namespace dsemu::cpu;
using namespace dsemu::memory;

int main(int argc, char **argv) {
    cout << "Starting main.." << endl;

    std::memset(ram, 0, 0xFFFF);

    dsemu::cart::load((const char *)argv[1]);

    std::memcpy(ram, dsemu::cart::g_romData, 0x8000);



/*
    ram[0] = 0x01;
    ram[1] = 0xC0;
    ram[2] = 0xDE;
    ram[3] = 0x3E;
    ram[4] = 0x55;
    ram[5] = 0x02;
    ram[6] = 0x3E;
    ram[7] = 0x99;
    ram[8] = 0x02;
    ram[9] = 0xFF;
*/
    //cout << endl << "BEFORE RUN, C0DE = " << std::hex << std::setfill('0') << std::setw(2) << (int)ram[0xC0DE] << endl << endl;

    dsemu::run();

    //cout << endl << "AFTER RUN, C0DE  = " << std::hex << std::setfill('0') << std::setw(2) << (int)ram[0xC0DE] << endl << endl;


/*

    for (int i=0xC0DE; i<0xCFFF; i++) {
        if ((i % 64) == 0) {
            cout << endl;
            cout << std::hex << std::setfill('0') << std::setw(4) << i << " - ";
        }

        cout << std::hex << std::setfill('0') << std::setw(2) << (int)ram[i] << " ";
    }

    Register reg;
    reg.hi = 0xC0;
    reg.lo = 0xDE;

    ushort *p = (ushort *)&reg;

    regAF.lo = 1;

    cout << "Reg: " << endl 
         << "Hi: " << std::hex << (int)reg.hi << " Lo:" << (int)reg.lo << " = " << (*p) << endl;
*/
    return 0;
}
