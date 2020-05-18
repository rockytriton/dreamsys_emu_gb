#include "memory.h"

namespace dsemu::memory {

    byte ram[0xFFFF];

    byte read(ushort address) {
        return ram[address];
    }

    void write(ushort address, byte value) {
        ram[address] = value;
    }

}

