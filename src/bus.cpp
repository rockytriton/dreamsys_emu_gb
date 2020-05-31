#include "bus.h"
#include "cart.h"
#include "memory.h"
#include "ppu.h"
#include "io.h"
#include "cpu.h"

#include <unistd.h>

namespace dsemu::bus {

    byte bankNumber = 1;

    byte read(ushort address) {
        if (address < 0x8000) {
            return cart::read(address);
        } else if (address < 0xA000) {
            //cout << "READ TO VIDEO RAM: " << Short(address) << endl;
            //sleep(2);
            return memory::read(address);
        } 
        else if (address < 0xFE00) {

            return memory::read(address);
        } else if (address < 0xFEA0) {

            return ppu::readOAM(address - 0xFE00);
        } else if (address < 0xFEFF) {
            //cout << "WTF THIS ISn'T USEABLE" << endl;
            return 0;
        } else if (address < 0xFF80) {
            return io::read(address);
        } else if (address < 0xFFFF) {
            //cout << "READING HRAM: " << Short(address) << " = " << Byte(memory::read(address)) << endl;

            if (address == 0xFFC0) {
                //sleep(4);
            }
            return memory::read(address);
        } else {
            return cpu::getInterruptsEnableFlag();
        }
    }

    void write(ushort address, byte b) {

        if (address < 0x8000) {
            //cart::write(address, b);
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

                sleep(1);
            } else if (cart::g_header.cartType == 1 ) {
                cout << "OTHER: " << Short(address) << endl;
                sleep(10);
            }

        } else if (address < 0xA000) {
            //if (DEBUG) cout << "WRITING TO VIDEO RAM: " << Short(address) << endl;
            //sleep(2);
	
            memory::write(address, b);
        } 
        else if (address < 0xFE00) {

            if (address == 0xc01a) {
                //cout << "BUS WRITINT TO C01A = " << Byte(b) << endl;
                //sleepMs(2000);
            }

            memory::write(address, b);

            if (address == 0xc01a) {
                //cout << "WROTE TO C01A = " << Byte(memory::read(address)) << endl;
                //sleepMs(2000);
            }

            //sleep(1);

        } else if (address < 0xFEA0) {
            ppu::writeOAM(address - 0xFE00, b);

        } else if (address < 0xFEFF) {
            //cout << "WTF THIS ISn'T USEABLE" << endl;
        } else if (address < 0xFF80) {
            io::write(address, b);
        } else if (address < 0xFFFF) {
            //cout << "WRITING HRAM: " << Short(address) << " = " << Byte(b) << endl;

            if (address == 0xFFC0) {
                //sleep(4);
            }

            memory::write(address, b);
        } else {
            cpu::setInterruptsEnableFlag(b);
        }
    }

}

