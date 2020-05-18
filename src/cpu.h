#pragma once

#include "common.h"

namespace dsemu {
namespace cpu {

struct Register {
    byte lo;
    byte hi;
};

enum Flags {
    FlagZ = 7,
    FlagN = 6,
    FlagH = 5,
    FlagC = 4
};

static inline bool get_flag(Flags n);
static inline void set_flag(Flags n, bool val);

enum Op {
    NOP,
    LD,
    LDI,
};

enum AddrType {
    ATypeRR,
    ATypeIR,
    ATypeAR,
    ATypeRI,
    ATypeRA
    /*
    ATypeA1R,
    ATypeA2R,
    ATypeRA1,
    ATypeRA2,
    ATypeIA2R,
    ATypeIA1R,
    ATypeRIA1,
    ATypeRIA2
    */
};

enum ParamType {
    A,
    B,
    C,
    D,
    E,
    H,
    L,
    BC,
    DE,
    HL,
    SP,
    PC,
    N,
    NN
};

struct OpCode {
    byte value;
    const string &name;
    Op op;
    byte length;
    AddrType mode;
    ParamType params[4];
};

extern Register regAF;
extern Register regBC;
extern Register regDE;
extern Register regHL;
extern ushort regPC;
extern Register regSP;

extern OpCode opCodes[];

void run();

}
}

