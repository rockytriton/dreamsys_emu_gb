#include "cpu.h"
#include "memory.h"
#include "bus.h"

#include <map>
#include <utility>
#include <unistd.h>

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

bool interruptsEnabled;

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

byte *getPointer(ParamType pt, bool hlAsRam = true) {
    switch(pt) {
        case A: return &regAF.hi;
        case B: return &regBC.hi; 
        case C: return &regBC.lo; 
        case D: return &regDE.hi; 
        case E: return &regDE.lo; 
        case H: return &regHL.hi; 
        case L: return &regHL.lo;
        case AF: return &regAF.hi;
        case BC: return &regBC.hi;
        case DE: return &regDE.hi;
        case N: return &memory::ram[regPC + 1]; 
        case HL: return hlAsRam ? &memory::ram[toShort(regHL.hi, regHL.lo)] : &regHL.hi;
        default:
            break;
    }

    cout << "ERROR BAD POINTER" << endl;
    exit(-1);

    return nullptr;
}

void handleGoto(ushort address) {

}

void handleLDH(const OpCode &op) {
    bool srcIsA = op.params[1] == A;

    if (srcIsA) {
        byte *p = getPointer(op.params[0]);

        if (DEBUG) cout << "Writing A to address: " << Short(*p | 0xFF00) << endl;

        bus::write(*p | 0xFF00, regAF.hi);
    } else {
        byte *p = getPointer(op.params[1]);
        regAF.hi = bus::read(*p | 0xFF00);
    }
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
            srcValue = bus::read(regPC + 1);
        } else if (op.params[1] == NN){
            srcValue = bus::read(regPC + 1) << 8;
            srcValue |= bus::read(regPC + 2);
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

            bus::write(dstValue, srcValue);
            break;
        case ATypeAR:
        case ATypeRR:
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

        default:
            cout << "INVALID OP FLAG" << endl;
            exit(-1);
            return;
            
    }

}

void handleNOP(const OpCode &op) {

}

void handleJumpRelative(const OpCode &op) {
    byte b = bus::read(regPC + 1);
    ushort location = regPC + (char)b;

    if (DEBUG) cout << "Jumping Relative: " << (int)(char)b << " to " << location << endl;

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
            location = toShort(bus::read(regPC + 1), bus::read(regPC + 2));
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

void handleDAA(const OpCode &op) {
    if (!get_flag(FlagN)) {
        ushort a = regAF.hi;
        byte nl = (regAF.hi & 0x0f);
        bool finalVal = false;
        
        if (get_flag(FlagH) || nl > 0x09) {
            a += 6;
        }

        if (get_flag(FlagC) || (a & 0xFFF0) > 0x90) {
            a += 0x60;
            finalVal = true;
        }

        regAF.hi = (byte)(a & 0xFF);
        set_flag(FlagC, finalVal);
    } else {
        if (get_flag(FlagH)) {
            regAF.hi -= 6;
        }

        if (get_flag(FlagC)) {
            regAF.hi -= 0x60;
        } else {
            set_flag(FlagC, false);
        }
    }

    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagH, false);
}


ushort lastCallAddress = 0;

void handlePOP(const OpCode &op) {
    ushort *p = (ushort *)getPointer(op.params[0], false);
    *p = spop();
}

void handlePUSH(const OpCode &op) {
    ushort *p = (ushort *)getPointer(op.params[0], false);
    push(*p);
}

void handleCALL(const OpCode &op) {
    lastCallAddress = regPC + 1;

    cout << "HANDLING CALL: " << Short(regPC) << endl;

    cpu::push((ushort)(regPC + 1));

    handleJump(op);
}

void handleRET(const OpCode &op) {
    regPC = cpu::spop() - op.length;

    cout << "HANDLING RET: " << Short(lastCallAddress) << " - " << Short(regPC) << endl;
    //sleep(50);
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
        case N: return bus::read(regPC + 1); 
        case HL: return bus::read(toShort(regHL.hi, regHL.lo)); break;
        default:
            cout << "BAD LDD" << endl;
            exit(-1);
            return 0;
    }
}

void handleLDD(const OpCode &op) {
    handleLD(op);
    ushort *p = (ushort *)&regHL;
    (*p)--;
}

void handleLDI(const OpCode &op) {
    handleLD(op);
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
                byte b = bus::read((*((ushort *)&regHL)));
                b++;
                bus::write((*((ushort *)&regHL)), b);
            } else {
                (*((ushort *)&regHL))++; 
            }
        } break;
        default:
            cout << "INVALID INC FLAG" << endl;
            exit(-1);
            return;
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
                byte b = bus::read((*((ushort *)&regHL)));
                b--;
                bus::write((*((ushort *)&regHL)), b);
            } else {
                (*((ushort *)&regHL))--; 
            }
        } break;
        default:
            cout << "INVALID DEC FLAG" << endl;
            exit(-1);
            return;
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

void handleDI(const OpCode &op) {
    interruptsEnabled = false;
    cout << "DISABLED INT" << endl;
    //sleep(3);
}

void handleEI(const OpCode &op) {
    interruptsEnabled = true;
    cout << "ENABLED INT" << endl;
    //sleep(3);
}

void handleRST(const OpCode &op) {
    ushort *sp = (ushort *)&regSP;
    *sp = (*sp) - 2;
    bus::write(*sp, (regPC + 1) & 0xFF);
    bus::write((*sp) + 1, ((regPC + 1) >> 8) & 0xFF);

    switch(op.params[0]) {
        case x00: regPC = 0x00; break;
        case x08: regPC = 0x08; break;
        case x10: regPC = 0x10; break;
        case x18: regPC = 0x18; break;
        case x20: regPC = 0x20; break;
        case x28: regPC = 0x28; break;
        case x30: regPC = 0x30; break;
        case x38: regPC = 0x38; break;
        default: {
            cout << "UNKNOWN RST" << endl;
            exit(-1);
        }
    }

    regPC -= 1;
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
    handlerMap[DI] = handleDI;
    handlerMap[EI] = handleEI;
    handlerMap[LDH] = handleLDH;
    handlerMap[RST] = handleRST;
    handlerMap[CALL] = handleCALL;
    handlerMap[RET] = handleRET;
    handlerMap[DAA] = handleDAA;
    handlerMap[PUSH] = handlePUSH;
    handlerMap[POP] = handlePOP;

    initParamTypeMap();
}

void handle_op(OpCode &opCode) {
    std::map<Op, HANDLER>::iterator it = handlerMap.find(opCode.op);

    if (it == handlerMap.end()) {
        cout << "UNKNOWN OP CODE: " << Byte(opCode.value) << endl;
        exit(-1);
    }

    it->second(opCode);
}

}
