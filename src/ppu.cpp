#include "ppu.h"
#include "cpu.h"
#include "memory.h"

#include <chrono>
#include <thread>

namespace dsemu::ppu {

int currentFrame = 0;
int currentLine = 0;

void init() {
    currentFrame = 0;
    currentLine = 0;
}

int getCurrentLine() {
    return currentLine;
}

void tick() {
    int f = cpu::getTickCount() % (TICKS_PER_FRAME);
    int l = f / 114;

    if (l != currentLine && l < 144) {
        cout << "PPU:> NEW LINE: " << l << endl;
    }

    if (l != currentLine && l == 144) {
        currentFrame++;
        cout << endl << "PPU:> NEW FRAME: " << currentFrame << endl << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        cpu::handleInterrupt(cpu::IVBlank, true);
    }

    currentLine = l;
}


}

