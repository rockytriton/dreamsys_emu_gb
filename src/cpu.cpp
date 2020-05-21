#include "cpu.h"
#include "memory.h"
#include "ppu.h"

#include <unistd.h>
#include <iomanip>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>

namespace dsemu {
namespace cpu {

const ushort IV_VBLANK = 0x40;

extern bool interruptsEnabled;

void init_handlers();
void handle_op(OpCode &opCode);

Register regAF;
Register regBC;
Register regDE;
Register regHL;

Register regSP;
ushort regPC = 0;
int remainingTicks = 0;
bool vbRequested = false;
bool vbEnabled = false;

uint64_t totalTicks = 0;

uint64_t getTickCount() {
    return totalTicks;
}

	SDL_Window *window;                    // Declare a pointer

void init() {
    regPC = 0x100;
    *((short *)&regAF) = 0x01B0;
    *((short *)&regBC) = 0x0013;
    *((short *)&regDE) = 0x00D8;
    *((short *)&regHL) = 0x014D;
    *((short *)&regSP) = 0xFFFE;

    init_handlers();

    SDL_Init(SDL_INIT_VIDEO);

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        50,           // initial x position
        50,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL  | SDL_WINDOW_RESIZABLE                // flags - see below
    );
}

int cpuSpeed = 1;

void tick() {
    totalTicks++;

    if (remainingTicks) {
        remainingTicks--;
        return;
    }

    byte b = memory::read(regPC);

    OpCode opCode = opCodes[b];

    cout << Short(regPC) << ": " << Byte(b) << " " << Byte(memory::read(regPC + 1)) << " " << Byte(memory::read(regPC + 2)) << " (" << opCode.name << ") "
            << " - A: " << Byte(regAF.hi) << " F: " << Byte(regAF.lo)
            << " - BC: " << Short(toShort(regBC.hi, regBC.lo))
            << " - DE: " << Short(toShort(regDE.hi, regDE.lo))
            << " - HL: " << Short(toShort(regHL.hi, regHL.lo))
            << endl;

    handle_op(opCode);

    regPC += opCode.length;

    std::this_thread::sleep_for(std::chrono::milliseconds(cpuSpeed));

    if (interruptsEnabled && memory::read(0xFFF0)) {
        cout << endl << "Interrupt ready to handle: " << Byte(memory::read(0xFFF0)) << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    if (interruptsEnabled && vbEnabled && vbRequested) {
        cout << endl << "VBLANK Interrupt ready to handle: " << Byte(memory::read(0xFFF0)) << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    remainingTicks = opCode.cycles - 1;

    SDL_Event e;
    if (SDL_PollEvent(&e) > 0)
    {
        SDL_UpdateWindowSurface(window);
    }
}

void handleInterrupt(byte flag, bool request) {
    if (flag & 1) {
        if (request) vbEnabled = true; else vbRequested = true;

        cout << "V-Blank " << (request ? "requested" : "enabled") << endl;

        if (request && interruptsEnabled) {
            cout << "CPU:> HANDLING VBLANK" << endl;
            ushort *sp = (ushort *)&regSP;
            *sp = (*sp) - 2;
            memory::write(*sp, (regPC + 1) & 0xFF);
            memory::write((*sp) + 1, ((regPC + 1) >> 8) & 0xFF);
            regPC = 0x40;
            cpuSpeed = 500;
        }
    }
}

}
}

