#include "emu.h"
#include "ppu.h"
#include "memory.h"
#include "cpu.h"
#include "bus.h"
#include "io.h"

 bool DEBUG = true;

namespace dsemu {

void run() {
    io::init();
    cpu::init();
    ppu::init();

    while(true) {
        cpu::tick();
        ppu::tick();
    }
}

}
