#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "bus.h"

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
byte intEnableFlag = 0;
byte intRequestFlag = 0;

uint64_t totalTicks = 0;

void push(ushort s) {
    ushort *sp = (ushort *)&regSP;
    *sp = (*sp) - 2;
    bus::write(*sp, s & 0xFF);
    bus::write((*sp) + 1, (s >> 8) & 0xFF);
}

void push(byte b) {
    ushort *sp = (ushort *)&regSP;
    *sp = (*sp) - 1;
    bus::write(*sp, b);
}

ushort spop() {
    ushort *sp = (ushort *)&regSP;
    byte lo = bus::read(*sp);
    *sp = (*sp) + 1;
    byte hi = bus::read(*sp);
    *sp = (*sp) + 1;

    return toShort(lo, hi);
}

byte pop() {
    ushort *sp = (ushort *)&regSP;
    byte lo = bus::read(*sp);
    *sp = (*sp) + 1;

    return lo;
}

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

int cpuSpeed = 0; //5;

void tick() {
    totalTicks++;

    if (remainingTicks) {
        remainingTicks--;
        return;
    }

    if (regAF.hi == 0x92) {
        //cpuSpeed = 500;
    }

    byte b = bus::read(regPC);

    OpCode opCode = opCodes[b];

    if (DEBUG) cout << Int64(totalTicks) << ": " << Short(regPC) << ": " << Byte(b) << " " << Byte(bus::read(regPC + 1)) << " " << Byte(bus::read(regPC + 2)) << " (" << opCode.name << ") "
            << " - A: " << Byte(regAF.hi) << " F: " << Byte(regAF.lo)
            << " - BC: " << Short(toShort(regBC.hi, regBC.lo))
            << " - DE: " << Short(toShort(regDE.hi, regDE.lo))
            << " - HL: " << Short(toShort(regHL.hi, regHL.lo))
            << endl;

    handle_op(opCode);

    regPC += opCode.length;

    std::this_thread::sleep_for(std::chrono::milliseconds(cpuSpeed));

    if (interruptsEnabled && bus::read(0xFF0F)) {
        //cout << endl << "Interrupt ready to handle: " << Byte(bus::read(0xFF0F)) << endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        handleInterrupt(bus::read(0xFF0F), true);
    }

    if (interruptsEnabled && vbEnabled && vbRequested) {
        //cout << endl << "VBLANK Interrupt ready to handle: " << Byte(bus::read(0xFF0F)) << endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    remainingTicks = opCode.cycles - 1;

    SDL_Event e;
    if (SDL_PollEvent(&e) > 0)
    {
        SDL_UpdateWindowSurface(window);

        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            exit(0);
        }
    }
}

void changePC(ushort address) {
    ushort *sp = (ushort *)&regSP;
    *sp = (*sp) - 2;
    bus::write(*sp, (regPC + 1) & 0xFF);
    bus::write((*sp) + 1, ((regPC + 1) >> 8) & 0xFF);
    regPC = address;
}

void handleInterrupt(byte flag, bool request) {
    if (flag & 1) {
        if (request) vbEnabled = true; else vbRequested = true;

        cout << "V-Blank " << (request ? "requested" : "enabled") << endl;

        if (request && interruptsEnabled) {
            cout << "CPU:> HANDLING VBLANK" << endl;
            intRequestFlag &= ~1;
            cout << "Disabling VBlank: " << Byte(intRequestFlag) << endl;

            changePC(0x40);
            //cpuSpeed = 500;
        }
    }
}


byte getInterruptsEnableFlag() {
    return intEnableFlag;
}
byte getInterruptsRequestsFlag() {
    return intRequestFlag;
}

void setInterruptsEnableFlag(byte f) {
    intEnableFlag |= f;
    cout << endl << "WRITING INT ENABLE FLAG: " << Byte(f) << endl << endl;
}

void setInterruptsRequestsFlag(byte f) {
    intRequestFlag |= f;
    cout << endl << "WRITING INT REQUEST FLAG: " << Byte(f) << " - " << Byte(intRequestFlag) << endl << endl;
}




}
}

