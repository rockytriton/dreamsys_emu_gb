#include "cpu.h"
#include "memory.h"

#include <map>
#include <utility>

ushort toShort(byte a, byte b) {
    ushort s = b << 8;
    s |= a;

    return s;
}

namespace dsemu::cpu {

typedef void (*HANDLER)(const OpCode &op);

std::map<Op, HANDLER> handlerMap;

enum RegParamType {
    RPT16,
    RPTHi,
    RPTLo
};

typedef std::map<ParamType, std::pair<Register *, RegParamType>> PTM;

PTM paramTypeMap;

void initParamTypeMap() {
    paramTypeMap[A] = std::make_pair(&regAF, RPTHi);
    paramTypeMap[B] = std::make_pair(&regBC, RPTHi);
    paramTypeMap[C] = std::make_pair(&regBC, RPTLo);
    paramTypeMap[D] = std::make_pair(&regDE, RPTHi);
    paramTypeMap[E] = std::make_pair(&regDE, RPTLo);
    paramTypeMap[H] = std::make_pair(&regHL, RPTHi);
    paramTypeMap[L] = std::make_pair(&regHL, RPTLo);
    paramTypeMap[BC] = std::make_pair(&regBC, RPT16);
    paramTypeMap[SP] = std::make_pair(&regSP, RPT16);
    paramTypeMap[HL] = std::make_pair(&regHL, RPT16);
    paramTypeMap[DE] = std::make_pair(&regDE, RPT16);
    paramTypeMap[AF] = std::make_pair(&regAF, RPT16);

}

void handleLD(const OpCode &op) {
    RegParamType srcType;
    RegParamType dstType;

    Register *src;
    Register *dst;

    ushort srcValue = 0;
    ushort dstValue = 0;

    PTM::iterator dit = paramTypeMap.find(op.params[0]);
    PTM::iterator sit = paramTypeMap.find(op.params[1]);

    if (sit == paramTypeMap.end()) {
        if (op.params[1] == N) {
            srcValue = memory::read(regPC + 1);
        } else if (op.params[1] == NN){
            srcValue = memory::read(regPC + 1) << 8;
            srcValue |= memory::read(regPC + 2);
        } else {
            cout << "Unknown LD Source Type " << endl;
            exit(-1);
        }
    } else {
        src = sit->second.first;
        srcType = sit->second.second;

        switch(srcType) {
            case RPT16: {
                ushort *p = (ushort *)src;
                srcValue = *p;
            } break;
            case RPTHi: {
                srcValue = src->hi;
            } break;
            case RPTLo: {
                srcValue = src->lo;
            } break;
        }
    }

    if (dit == paramTypeMap.end()) {
        if (op.params[0] == N) {
            //
        } else if (op.params[0] == NN){
            //
        } else {
            cout << "Unknown LD Dest Type " << endl;
            exit(-1);
        }
    } else {
        dst = dit->second.first;
        dstType = dit->second.second;

        switch(dstType) {
            case RPT16: {
                ushort *p = (ushort *)dst;
                dstValue = *p;
            } break;
            case RPTHi: {
                dstValue = dst->hi;
            } break;
            case RPTLo: {
                dstValue = dst->lo;
            } break;
        }
    }

    switch(op.mode) {
        case ATypeRA:
            cout << "WRITING REGISTER TO ADDRESS: " << endl;
            cout << "\t SRCVALUE = " << std::hex << srcValue << endl;
            cout << "\t DSTVALUE = " << std::hex << dstValue << endl;

            memory::write(dstValue, srcValue);
            break;
        case ATypeIR:
            {
                if (dstType == RPTLo) {
                    dst->lo = srcValue;
                } else if (dstType == RPTHi) {
                    dst->hi = srcValue;
                } else {
                    *((ushort *)dst) = srcValue;
                }
            } break;
            
    }

}

void handleNOP(const OpCode &op) {

}

void handleJumpRelative(const OpCode &op) {
    ushort location = regPC + memory::read(regPC + 1);

    if (op.mode == ATypeJ) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_C && get_flag(FlagC)) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_NC && !get_flag(FlagC)) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_NZ && !get_flag(FlagZ)) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_Z && get_flag(FlagZ)) {
        regPC = location - op.length;
    }
}

void handleJump(const OpCode &op) {
    ushort location = 0;

    switch(op.params[0]) {
        case NN:
            location = toShort(memory::read(regPC + 1), memory::read(regPC + 2));
            break;
        case HL:
            location = toShort(regHL.hi, regHL.lo);
            break;
        default:
            cout << "ERRO BAD JUMP" << endl;
            exit(-1);

    }

    if (op.mode == ATypeJ) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_C && get_flag(FlagC)) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_NC && !get_flag(FlagC)) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_NZ && !get_flag(FlagZ)) {
        regPC = location - op.length;
    } else if (op.mode == ATypeJ_Z && get_flag(FlagZ)) {
        regPC = location - op.length;
    }

}

byte *getPointer(ParamType pt) {
    switch(pt) {
        case A: return &regAF.hi;
        case B: return &regBC.hi; 
        case C: return &regBC.lo; 
        case D: return &regDE.hi; 
        case E: return &regDE.lo; 
        case H: return &regHL.hi; 
        case L: return &regHL.lo; 
        case N: return &memory::ram[memory::read(regPC + 1)]; 
        case HL: return &memory::ram[toShort(regHL.hi, regHL.lo)];
    }

    cout << "ERROR BAD POINTER" << endl;
    exit(-1);

    return nullptr;
}

void setFlags(byte first, byte second, bool add, bool withCarry) {
    if (add) {
        ushort a = first + second + (withCarry && get_flag(FlagC));
        set_flag(FlagZ, a == 0);
        set_flag(FlagC, a > 0xFF);
        set_flag(FlagN, false);
        set_flag(FlagH, (first & 0xF) + (second & 0xF) + (withCarry && get_flag(FlagC)) > 0xF);
    } else {
        short a = first - second - (withCarry && get_flag(FlagC));
        set_flag(FlagZ, a == 0);
        set_flag(FlagC, a < 0);
        set_flag(FlagN, true);
        set_flag(FlagH, (first & 0xF) - (second & 0xF) - (withCarry && get_flag(FlagC)) < 0);
    }
}

void handleCP(const OpCode &op) { 
    byte *val = getPointer(op.params[0]);

    setFlags(regAF.hi, *val, false, false);
}

void handleADC(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    ushort a = regAF.hi + *val + get_flag(FlagC);
    setFlags(regAF.hi, *val, true, true);

    regAF.hi = a & 0x00FF;
}

void handleADD(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    ushort a = regAF.hi + *val;
    setFlags(regAF.hi, *val, true, false);

    regAF.hi = a & 0x00FF;
}

void handleSUB(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    short a = regAF.hi - *val;
    setFlags(regAF.hi, *val, false, false);

    regAF.hi = a & 0x00FF;
}

void handleSBC(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    short a = regAF.hi - *val - get_flag(FlagC);
    setFlags(regAF.hi, *val, false, true);

    regAF.hi = a & 0x00FF;
}

void handleAND(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    regAF.hi &= *val;
    
    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, false);
    set_flag(FlagH, true);
    set_flag(FlagC, 0);
}

void handleOR(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    regAF.hi |= *val;
    
    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, false);
    set_flag(FlagH, false);
    set_flag(FlagC, 0);
}

void handleXOR(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    regAF.hi ^= *val;
    
    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, false);
    set_flag(FlagH, false);
    set_flag(FlagC, 0);
}

byte getVal(ParamType pt) {
    switch(pt) {
        case A: return regAF.hi; 
        case B: return regBC.hi; 
        case C: return regBC.lo; 
        case D: return regDE.hi; 
        case E: return regDE.lo; 
        case H: return regHL.hi; 
        case L: return regHL.lo; 
        case N: return memory::read(regPC + 1); 
        case HL: return memory::read(toShort(regHL.hi, regHL.lo)); break;
        default:
            cout << "BAD LDD" << endl;
            exit(-1);
            return 0;
    }
}

void handleLDD(const OpCode &op) {
    byte val = getVal(op.params[0]);
    ushort *p = (ushort *)&regHL;
    (*p)--;
}

void handleLDI(const OpCode &op) {
    byte val = getVal(op.params[0]);
    ushort *p = (ushort *)&regHL;
    (*p)++;
}

void handleINC(const OpCode &op) {
    switch(op.params[0]) {
        case A: regAF.hi++; break;
        case B: regBC.hi++; break;
        case C: regBC.lo++; break;
        case D: regDE.hi++; break;
        case E: regDE.lo++; break;
        case H: regHL.hi++; break;
        case L: regHL.lo++; break;
        case BC: (*((ushort *)&regBC))++; break;
        case DE: (*((ushort *)&regDE))++; break;
        case HL: {
            if (op.mode == ATypeA) {
                byte b = memory::read((*((ushort *)&regHL)));
                b++;
                memory::write((*((ushort *)&regHL)), b);
            } else {
                (*((ushort *)&regHL))++; 
            }
        } break;
    }

    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, 0);
    set_flag(FlagH, (regAF.hi & 0xF) + 1 > 0xF);
}

void handleDEC(const OpCode &op) {
    switch(op.params[0]) {
        case A: regAF.hi--; break;
        case B: regBC.hi--; break;
        case C: regBC.lo--; break;
        case D: regDE.hi--; break;
        case E: regDE.lo--; break;
        case H: regHL.hi--; break;
        case L: regHL.lo--; break;
        case BC: (*((ushort *)&regBC))--; break;
        case DE: (*((ushort *)&regDE))--; break;
        case HL: {
            if (op.mode == ATypeA) {
                byte b = memory::read((*((ushort *)&regHL)));
                b--;
                memory::write((*((ushort *)&regHL)), b);
            } else {
                (*((ushort *)&regHL))--; 
            }
        } break;
    }

    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, 1);
    set_flag(FlagH, (regAF.hi & 0xF) - 1 < 0);
}

void handleRLA(const OpCode &op) {
    byte b = regAF.hi & (1 << 7);
    regAF.hi <<= 1;
    regAF.hi &= (b);
}

void handleRLCA(const OpCode &op) {
    byte b = regAF.hi & (1 << 7);
    regAF.hi <<= 1;
    
    if (b & (1 << 7)) {
        set_flag(FlagC, true);
    } else {
        set_flag(FlagC, false);
    }
}

void handleRRA(const OpCode &op) {
    byte b = regAF.hi & 1;
    regAF.hi >>= 1;
    regAF.hi &= (b << 7);
}

void handleRRCA(const OpCode &op) {
    byte b = regAF.hi & 1;
    regAF.hi >>= 1;
    
    if (b) {
        set_flag(FlagC, true);
    } else {
        set_flag(FlagC, false);
    }
}

void init_handlers() {
    handlerMap[LD] = handleLD;
    handlerMap[LDI] = handleLDI;
    handlerMap[LDD] = handleLDD;
    handlerMap[NOP] = handleNOP;
    handlerMap[JP] = handleJump;
    handlerMap[JR] = handleJumpRelative;
    handlerMap[XOR] = handleXOR;
    handlerMap[OR] = handleOR;
    handlerMap[AND] = handleAND;
    handlerMap[INC] = handleINC;
    handlerMap[DEC] = handleDEC;
    handlerMap[RRCA] = handleRRCA;
    handlerMap[RRA] = handleRRA;
    handlerMap[RLCA] = handleRLCA;
    handlerMap[RLA] = handleRLA;

    handlerMap[ADD] = handleADD;
    handlerMap[ADC] = handleADC;
    handlerMap[SUB] = handleSUB;
    handlerMap[SBC] = handleSBC;
    handlerMap[CP] = handleCP;


    initParamTypeMap();
}

void handle_op(OpCode opCode) {
    std::map<Op, HANDLER>::iterator it = handlerMap.find(opCode.op);

    if (it == handlerMap.end()) {
        cout << "UNKNOWN OP CODE" << opCode.op << endl;
        exit(-1);
    }

    it->second(opCode);
}

}
