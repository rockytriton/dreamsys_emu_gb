#include "ppu.h"
#include "cpu.h"
#include "memory.h"

#include <chrono>
#include <thread>

namespace dsemu::ppu {

ScrollInfo scrollInfo;
int currentFrame = 0;
byte currentLine = 0;

byte oamRAM[160];

void init() {
    currentFrame = 0;
    currentLine = 0;
}

byte getCurrentLine() {
    return currentLine;
}
byte readOAM(ushort address) {
    return oamRAM[address];
}

void writeOAM(ushort address, byte b) {
    oamRAM[address] = b;
}

void tick() {
    int f = cpu::getTickCount() % (TICKS_PER_FRAME);
    int l = f / 114;

    if (l != currentLine && l < 144) {
        if (DEBUG) cout << "PPU:> NEW LINE: " << l << " FRAME: " << currentFrame << endl;
    }

    if (l != currentLine && l == 145) {
        currentFrame++;
        if (DEBUG) cout << endl << "PPU:> NEW FRAME: " << currentFrame << endl << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        cpu::handleInterrupt(cpu::IVBlank, true);
    }

    currentLine = l;
}


}

