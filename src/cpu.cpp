#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "bus.h"


#include <SDL2/SDL.h>

namespace dsemu {
namespace cpu {

//const ushort IV_VBLANK = 0x40;

extern bool interruptsEnabled;

void init_handlers();
int handle_op(OpCode &opCode);

Register regAF;
Register regBC;
Register regDE;
Register regHL;
int extraCycles;

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

    //cout << "HANDLING PUSH: " << Short(s) << endl;
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


void init() {
    regPC = 0x100;
    *((short *)&regAF) = 0x01B0;
    *((short *)&regBC) = 0x0013;
    *((short *)&regDE) = 0x00D8;
    *((short *)&regHL) = 0x014D;
    *((short *)&regSP) = 0xFFFE;

    init_handlers();
}

int cpuSpeed = 0; //1; //5;
int n = 0;

void tick() {
    totalTicks++;

    if (remainingTicks) {
        remainingTicks--;
        return;
    }

    if (n > 0xEF90) {
        //cpuSpeed = 500;
        //DEBUG = true;
    }

    byte b = bus::read(regPC);

    OpCode opCode = opCodes[b];
    n++;

    if (DEBUG) cout << Int64(n) << ": " << Short(regPC) << ": " << Byte(b) << " " << Byte(bus::read(regPC + 1)) << " " << Byte(bus::read(regPC + 2)) << " (" << opCode.name << ") "
            << " - AF: " << Short(toShort(regAF.lo, regAF.hi))
            << " - BC: " << Short(toShort(regBC.lo, regBC.hi))
            << " - DE: " << Short(toShort(regDE.lo, regDE.hi))
            << " - HL: " << Short(toShort(regHL.lo, regHL.hi))
            << " - Cycles: " << (totalTicks - 1)
            << endl;

    int n = handle_op(opCode);

    if (opCode.value == 0xff) {
        sleep(5);
    }

    regPC += opCode.length;

    std::this_thread::sleep_for(std::chrono::milliseconds(cpuSpeed));

    if (interruptsEnabled && bus::read(0xFF0F)) {
        //cout << endl << "Interrupt ready to handle: " << Byte(bus::read(0xFF0F)) << endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        handleInterrupt(bus::read(0xFF0F), true, false);
    }

    if (interruptsEnabled && vbEnabled && vbRequested) {
        //cout << endl << "VBLANK Interrupt ready to handle: " << Byte(bus::read(0xFF0F)) << endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    remainingTicks = ((n + opCode.cycles) / 4) - 1;

    if (extraCycles) {
        remainingTicks += extraCycles;
        extraCycles = 0;
    }
}

void handleInterrupt(byte flag, bool request, bool pcp1) {
    if (flag & 1) {
        if (request) vbEnabled = true; else vbRequested = true;

        cout << "V-Blank " << (request ? "requested" : "enabled") << endl;

        if (request && interruptsEnabled && (intEnableFlag & 1)) {
            cout << "CPU:> HANDLING VBLANK" << endl;
            intRequestFlag &= ~1;
            //cout << "Disabling VBlank: " << Byte(intRequestFlag) << endl;

            if (pcp1) {
                push((ushort)(regPC + 1));
            } else {
                push(regPC);
            }

            regPC = 0x40;
            //changePC(0x40);
            //cpuSpeed = 500;
        } else {
            cout << "DENIED" << endl;
        }
    } else {
        cout << "OK ANOTHER INTERRUPT..." << endl;
        //sleep(10);
    }
}


byte getInterruptsEnableFlag() {
    return intEnableFlag;
}
byte getInterruptsRequestsFlag() {
    return intRequestFlag;
}

void setInterruptsEnableFlag(byte f) {
    intEnableFlag = f;
    cout << endl << "WRITING INT ENABLE FLAG: " << Byte(f) << endl << endl;
}

void setInterruptsRequestsFlag(byte f) {
    intRequestFlag = f;
    cout << endl << "WRITING INT REQUEST FLAG: " << Byte(f) << " - " << Byte(intRequestFlag) << endl << endl;
}




}
}

