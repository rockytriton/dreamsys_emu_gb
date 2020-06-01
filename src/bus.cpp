#include "bus.h"
#include "cart.h"
#include "memory.h"
#include "ppu.h"
#include "io.h"
#include "cpu.h"

#include <unistd.h>
#include <cstring>

namespace dsemu::bus {

    byte bankNumber = 1;

    byte read(ushort address) {
        if (address < 0x8000) {
            return memory::read(address);
        } else if (address < 0xA000) {
            return memory::read(address);
        } 
        else if (address < 0xFE00) {

            return memory::read(address);
        } else if (address < 0xFEA0) {

            return ppu::readOAM(address - 0xFE00);
        } else if (address < 0xFEFF) {
            return 0;
        } else if (address < 0xFF80) {
            return io::read(address);
        } else if (address < 0xFFFF) {
            return memory::read(address);
        } else {
            return cpu::getInterruptsEnableFlag();
        }
    }

    void write(ushort address, byte b) {

        if (address < 0x8000) {
            if (cart::g_header.cartType == 0) {
                cout << "BAD WRITE" << endl;
                sleep(1);
                return;
            }
            
            if (address < 0x2000) {
                //disable sram...
                return;
            }

            if (cart::g_header.cartType == 1 && address >= 0x6000) {
                cout << "Memory Model Select: " << Byte(b & 1) << endl;
                sleep(10);
            } else if (cart::g_header.cartType == 1 && address >= 0x2000 && address <= 0x3FFF) {
                cout << "Rom Bank Select: " << Byte(b & 0x1F) << endl;

                byte bank = b & 0x1F;
                if(bank == 0 || bank == 0x20 || bank == 0x40 || bank == 0x60)
                    bank++;

                memcpy(&memory::ram[0x4000], cart::g_romData + (bank * 0x4000), 0x4000);

                cout << "Copied starting at: " << (int)(bank * 0x4000) << " - size: " << cart::g_romSize << endl;

                if ((bank * 0x4000) > cart::g_romSize) {
                    cout << "TOO BIG" << endl;
                    sleep(10);
                }

                //sleep(1);
            } else if (cart::g_header.cartType == 1 ) {
                cout << "OTHER: " << Short(address) << endl;
                sleep(10);
            }

        } else if (address < 0xA000) {
            if (address == 0xDD00 || address == 0xDD01) {
                cout << "WRITING TO " << Short(address) << " = " << Byte(b) << endl;
                sleep(2);
            }
            memory::write(address, b);
        } 
        else if (address < 0xFE00) {
            memory::write(address, b);
        } else if (address < 0xFEA0) {
            ppu::writeOAM(address - 0xFE00, b);

        } else if (address < 0xFEFF) {
        } else if (address < 0xFF80) {
            io::write(address, b);
        } else if (address < 0xFFFF) {
            memory::write(address, b);
        } else {
            cpu::setInterruptsEnableFlag(b);
        }
    }

}

