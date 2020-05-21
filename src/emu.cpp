#include "emu.h"
#include "ppu.h"
#include "memory.h"
#include "cpu.h"

namespace dsemu {

void run() {
    cpu::init();
    ppu::init();

    while(true) {
        cpu::tick();
        ppu::tick();
    }
}

}
