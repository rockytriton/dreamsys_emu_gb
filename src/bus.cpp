#include "bus.h"
#include "cart.h"
#include "memory.h"
#include "ppu.h"
#include "io.h"
#include "cpu.h"

#include <unistd.h>

namespace dsemu::bus {

    byte read(ushort address) {
        if (address < 0x8000) {
            return cart::read(address);
        } else if (address < 0xA000) {
            cout << "READ TO VIDEO RAM: " << Short(address) << endl;
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
            cout << "READING HRAM: " << Short(address) << " = " << Byte(memory::read(address)) << endl;
            return memory::read(address);
        } else {
            return cpu::getInterruptsEnableFlag();
        }
    }

    void write(ushort address, byte b) {
        if (address < 0x8000) {
            //cart::write(address, b);
        } else if (address < 0xA000) {
            if (DEBUG) cout << "WRITING TO VIDEO RAM: " << Short(address) << endl;
            //sleep(2);
            memory::write(address, b);
        } 
        else if (address < 0xFE00) {

            memory::write(address, b);


            //sleep(1);

        } else if (address < 0xFEA0) {
            ppu::writeOAM(address - 0xFE00, b);

        } else if (address < 0xFEFF) {
            //cout << "WTF THIS ISn'T USEABLE" << endl;
        } else if (address < 0xFF80) {
            io::write(address, b);
        } else if (address < 0xFFFF) {
            cout << "WRITING HRAM: " << Short(address) << " = " << Byte(b) << endl;
            memory::write(address, b);
        } else {
            cpu::setInterruptsEnableFlag(b);
        }
    }

}

