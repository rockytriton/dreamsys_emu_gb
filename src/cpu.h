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
    LDD,
    INC,
    DEC,
    JR,
    DAA,
    ADD,
    SUB,
    RLA,
    RLCA,
    RRA,
    RRCA,
    STOP,
    SCF,
    CPL,
    CCF,
    SBC,
    XOR,
    AOR,
    CP,
    POP,
    PUSH,
    AND,
    OR,
    JP,
    RET,
    DI,
    EI,
    CALL,
    RST,
    CB,
    ADC,
    X,
    RETI,
    LDH
};

enum AddrType {
    ATypeNA,
    ATypeRR,
    ATypeIR,
    ATypeAR,
    ATypeRI,
    ATypeRA,
    ATypeJ,
    ATypeJ_NZ,
    ATypeJ_Z,
    ATypeJ_NC,
    ATypeJ_C,
    ATypeR,
    ATypeA,
    ATypeSP
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
    NN,
    x00, x10, x20, x30, x08, x18, x28, x38
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

