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

typedef int (*HANDLER)(const OpCode &op);

std::map<Op, HANDLER> handlerMap;

enum RegParamType {
    RPT16,
    RPTHi,
    RPTLo
};

typedef std::map<ParamType, std::pair<Register *, RegParamType>> PTM;

PTM paramTypeMap;
bool eiCalled = false;

bool interruptsEnabled;

std::map<byte, byte> jumpCycleMap;

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

    jumpCycleMap[0x20] = 4;
    jumpCycleMap[0x30] = 4;
    jumpCycleMap[0x28] = 4;
    jumpCycleMap[0x38] = 4;
    jumpCycleMap[0xC0] = 12;
    jumpCycleMap[0xD0] = 12;
    jumpCycleMap[0xC2] = 4;
    jumpCycleMap[0xD2] = 4;
    jumpCycleMap[0xC4] = 12;
    jumpCycleMap[0xD4] = 12;
    jumpCycleMap[0xC8] = 12;
    jumpCycleMap[0xD8] = 12;
    jumpCycleMap[0xCA] = 4;
    jumpCycleMap[0xDA] = 4;
    jumpCycleMap[0xCC] = 12;
    jumpCycleMap[0xDC] = 12;

}

byte *regFromBits(byte reg) {
    byte *pReg = nullptr;

    switch(reg) {
        case 0: pReg = &regBC.hi; break;
        case 1: pReg = &regBC.lo; break;
        case 2: pReg = &regDE.hi; break;
        case 3: pReg = &regDE.lo; break;
        case 4: pReg = &regHL.hi; break;
        case 5: pReg = &regHL.lo; break;
        case 6: pReg = &memory::ram[toShort(regHL.hi, regHL.lo)]; break;
        case 7: pReg = &regAF.hi; break;
            default:
                cout << "INVALID REG " << endl;
                exit(-1);
    }

    return pReg;
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
        case AF: return &regAF.lo;
        case BC: return &regBC.lo;
        case DE: return &regDE.lo;
        case N: return &memory::ram[regPC + 1]; 
        case HL: {
            if (toShort(regHL.hi, regHL.lo) >= 0x8000 && toShort(regHL.hi, regHL.lo) < 0xA000) {
                //cout << "HL ACCESSING VRAM: " << Short(toShort(regHL.hi, regHL.lo)) << endl;
            }
            return hlAsRam ? &memory::ram[toShort(regHL.hi, regHL.lo)] : &regHL.lo;
        }
        default:
            break;
    }

    cout << "ERROR BAD POINTER" << endl;
    exit(-1);

    return nullptr;
}

int handleGoto(ushort address) {
    return 0;
}

int handleLDH(const OpCode &op) {
    bool srcIsA = op.params[1] == A;

    if (srcIsA) {
        byte *p = getPointer(op.params[0]);

        if (DEBUG) cout << "Writing A to address: " << Short(*p | 0xFF00) << endl;

        bus::write(*p | 0xFF00, regAF.hi);
    } else {
        byte *p = getPointer(op.params[1]);
        regAF.hi = bus::read(*p | 0xFF00);
    }
    return 0;
}

ushort getAddrValue(ParamType op, short srcValue) {
    switch(op) {
        case HL: return *((ushort *)&regHL);
        case AF: return *((ushort *)&regAF);
        case BC: return *((ushort *)&regBC);
        case DE: return *((ushort *)&regDE);
        case NN: return srcValue;
        case N: return 0xFF00 | srcValue;
        default: 
            cout << "ERROR BAD ADDR VAL: " << endl;
            exit(-1);
            return 0;
    }
}

int handleLD(const OpCode &op) {
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
        } else if (op.params[1] == NN) {
            srcValue = bus::read(regPC + 1);
            srcValue |= bus::read(regPC + 2) << 8;
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
        dst = nullptr;
        dstType = RegParamType::RPT16;

        if (op.params[0] == N) {
            //
            //exit(-1);
        } else if (op.params[0] == NN){
            //
            //exit(-1);
            
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
        case ATypeRA: {

            ushort addr = getAddrValue(op.params[0], srcValue);
            bus::write(addr, srcValue);
        }
            break;
        case ATypeAR: {
            ushort addr = getAddrValue(op.params[1], srcValue);

            if (dstType == RPTLo) {
                dst->lo = bus::read(addr); ;
            } else if (dstType == RPTHi) {
                dst->hi = bus::read(addr); ;
            } else {
                *((ushort *)dst) = bus::read(addr); ;
            }
            
            break;
        } break;
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
            cout << "INVALID OP FLAG: " << dstValue << endl;
            exit(-1);
            return 0;
            
    }
    return 0;

}

int handleNOP(const OpCode &op) {
    //cout << " NOP " << endl;
    //sleep(1);
    return 0;

}

int handleCB(const OpCode &op) {
    int code = bus::read(regPC + 1);
    byte reg = code & 7;
    byte bitOp = (code >> 6) & 3;
    byte bit = (code >> 3) & 7;
    byte *pReg = regFromBits(reg);

    if (bitOp) {
        switch(bitOp) {
            case 1:
                set_flag(FlagZ, (*pReg) & (1 << bit));
                set_flag(FlagN, false);
                set_flag(FlagH, true);
                break;
            case 2:
                (*pReg) &= ~(1 << bit);
                break;
            case 3:
                (*pReg) |= (1 << bit);
                break;
            default:
                cout << "INVALID BIT " << endl;
                exit(-1);
        }

        return 0;
    }

    bitOp = bit;
    int cBit = get_flag(FlagC);

    switch (bitOp) {
        case 0: //RLC
            (*pReg) <<= (1 + get_flag(FlagC));
            break;
        case 1: //RRC
            (*pReg) >>= (1 + get_flag(FlagC));
            break;
        case 2: //RL
            (*pReg) <<= 1;
            break;
        case 3: //RR
            (*pReg) >>= 1;
            break;
        case 4: //SLA
            set_flag(FlagC, (*pReg) & 0x80);
            (*pReg) <<= 1;
            (*pReg) |= cBit;
            break;
        case 5: //SRA
            set_flag(FlagC, (*pReg) & 1);
            (*pReg) >>= 1;
            (*pReg) |= (cBit << 7);
            break;
        case 6: //SRL
            set_flag(FlagC, (*pReg) & 1);
            (*pReg) >>= 1;
            break;
        case 7: //SWAP
            set_flag(FlagC, 0);
            (*pReg) = (((*pReg) & 0xF0) >> 4) | (((*pReg) & 0xF) << 4);
            break;
        default:
            cout << "INVALID BIT2 " << endl;
            exit(-1);
    }

    set_flag(FlagH, 0);
    set_flag(FlagN, 0);
    return 0;

}

int conditionalJump(ushort location, const OpCode &op, bool &didJump) {
    int diff = jumpCycleMap[op.value];

    if (op.mode == ATypeJ) {
        regPC = location;
        didJump = true;
        return 0;
    } else if (op.mode == ATypeJ_C && get_flag(FlagC)) {
        regPC = location;
        didJump = true;
        return diff;
    } else if (op.mode == ATypeJ_NC && !get_flag(FlagC)) {
        regPC = location;
        didJump = true;
        return diff;
    } else if (op.mode == ATypeJ_NZ && !get_flag(FlagZ)) {
        regPC = location;
        didJump = true;
        return diff;
    } else if (op.mode == ATypeJ_Z && get_flag(FlagZ)) {
        regPC = location;
        didJump = true;
        return diff;
    }

    return 0;
}

int handleJumpRelative(const OpCode &op) {
    byte b = bus::read(regPC + 1);
    ushort location = regPC + (char)b;
    bool didJump;

    return conditionalJump(location, op, didJump);
}

int handleJump(const OpCode &op) {
    ushort location = 0;
    bool didJump;

    switch(op.params[0]) {
        case NN:
            location = toShort(bus::read(regPC + 1), bus::read(regPC + 2));
            break;
        case HL:
            location = toShort(regHL.lo, regHL.hi);
            break;
        default:
            cout << "ERRO BAD JUMP" << endl;
            exit(-1);

    }

    return conditionalJump(location - op.length, op, didJump);

/*
    if (op.mode == ATypeJ) {
        regPC = location - op.length;
        return 0;
    } else if (op.mode == ATypeJ_C && get_flag(FlagC)) {
        regPC = location - op.length;
        return 4;
    } else if (op.mode == ATypeJ_NC && !get_flag(FlagC)) {
        regPC = location - op.length;
        return 4;
    } else if (op.mode == ATypeJ_NZ && !get_flag(FlagZ)) {
        regPC = location - op.length;
        return 4;
    } else if (op.mode == ATypeJ_Z && get_flag(FlagZ)) {
        regPC = location - op.length;
        return 4;
    }
    return 0;
*/
}

int handleDAA(const OpCode &op) {
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
    return 0;
}


ushort lastCallAddress = 0;

int handlePOP(const OpCode &op) {
    ushort *p = (ushort *)getPointer(op.params[0], false);
    *p = spop();
    return 0;
}

int handlePUSH(const OpCode &op) {
    ushort *p = (ushort *)getPointer(op.params[0], false);
    push(*p);

    return 0;
}

int handleCALL(const OpCode &op) {
    ushort lca = regPC + op.length;
    bool didJump;
    ushort location = toShort(bus::read(regPC + 1), bus::read(regPC + 2)) - op.length;
    //lastCallAddress = regPC + op.length;

    //cout << "HANDLING CALL: " << Short(regPC) << endl;


    int ret = conditionalJump(location, op, didJump);

    if (didJump) {
        cpu::push((ushort)(lca));
        lastCallAddress = lca;
    }

    return ret;
}

int handleRET(const OpCode &op) {
    bool didJump;
    ushort location = cpu::spop();

    //cout << "HANDLING RET: " << Short(lastCallAddress) << " - " << Short(location) << endl;
    int ret = conditionalJump(location - 1, op, didJump);

    //cout << "RET - AFTER RET: " << ret << " - " << Short(regPC) << endl;
    
    if (!didJump) {
        cpu::push(location);
    }

    return ret;
}

int handleRETI(const OpCode &op) {
    interruptsEnabled = true;
    int n = handleRET(op);

    //cout << "RETI - AFTER RET: " << n << " - " << Short(regPC) << endl;

    return n;
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

    return;
}

int handleCP(const OpCode &op) { 
    byte *val = getPointer(op.params[0]);

    setFlags(regAF.hi, *val, false, false);
    return 0;
}

int handleADC(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    ushort a = regAF.hi + *val + get_flag(FlagC);
    setFlags(regAF.hi, *val, true, true);

    regAF.hi = a & 0x00FF;
    return 0;
}

int handleADD(const OpCode &op) {
    if (op.params[0] == A) {
        byte *p = getPointer(op.params[1]);
        ushort a = regAF.hi + *p;
        setFlags(regAF.hi, *p, true, false);
        regAF.hi = a & 0x00FF;

    } else if (op.params[0] == SP) {
        byte e = bus::read(regPC + 1);
        setFlags(*((ushort *)&regSP), e, true, false);
        *((ushort *)&regSP) += e;
        set_flag(FlagZ, false);
    } else {
        ushort *p = (ushort *)getPointer(op.params[1]);
        ushort *pHL = (ushort *)&regHL;
        int n = *pHL + *p;

        set_flag(FlagC, n >= 0x10000);
        set_flag(FlagN, false);
        set_flag(FlagH, (n & 0xFFF) < (*pHL & 0xFFF));
        *pHL = n & 0xFFFF;
    }

    return 0;
}

int handleSUB(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    short a = regAF.hi - *val;
    setFlags(regAF.hi, *val, false, false);

    regAF.hi = a & 0x00FF;
    return 0;
}

int handleSBC(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    short a = regAF.hi - *val - get_flag(FlagC);
    setFlags(regAF.hi, *val, false, true);

    regAF.hi = a & 0x00FF;
    return 0;
}

int handleAND(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    regAF.hi &= *val;
    
    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, false);
    set_flag(FlagH, true);
    set_flag(FlagC, 0);
    return 0;
}

int handleOR(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    regAF.hi |= *val;
    
    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, false);
    set_flag(FlagH, false);
    set_flag(FlagC, 0);
    return 0;
}

int handleXOR(const OpCode &op) {
    byte *val = getPointer(op.params[0]);

    regAF.hi ^= *val;
    
    set_flag(FlagZ, regAF.hi == 0);
    set_flag(FlagN, false);
    set_flag(FlagH, false);
    set_flag(FlagC, 0);
    return 0;
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

int handleLDD(const OpCode &op) {
    handleLD(op);
    ushort *p = (ushort *)&regHL;
    (*p)--;
    return 0;
}

int handleLDI(const OpCode &op) {
    handleLD(op);
    ushort *p = (ushort *)&regHL;
    (*p)++;
    return 0;
}

int handleINC(const OpCode &op) {
    ushort val = 0;

    switch(op.params[0]) {
        case A: regAF.hi++; val = regAF.hi; break;
        case B: regBC.hi++; val = regBC.hi; break;
        case C: regBC.lo++; val = regBC.lo; break;
        case D: regDE.hi++; val = regDE.hi; break;
        case E: regDE.lo++; val = regDE.lo; break;
        case H: regHL.hi++; val = regHL.hi; break;
        case L: regHL.lo++; val = regHL.lo; break;
        case BC: (*((ushort *)&regBC))++; val = (*((ushort *)&regBC));  return 0;
        case DE: (*((ushort *)&regDE))++; val = (*((ushort *)&regDE)); return 0;
        case HL: {
            if (op.mode == ATypeA) {
                byte b = bus::read((*((ushort *)&regHL)));
                b++;
                bus::write((*((ushort *)&regHL)), b);
                val = b;

                //cout << "SETTING VAL TO B: " << Byte(val) << " - " << (uint64_t)&val << endl;

            } else {
                (*((ushort *)&regHL))++; 
                val = (*((ushort *)&regHL)); 
                return 0;
            }
        } break;
        default:
            cout << "INVALID INC FLAG" << endl;
            exit(-1);
            return 0;
    }

    //cout << "VAL AGAIN: " << Byte(val) << " - " << (uint64_t)&val << endl;

    set_flag(FlagZ, val == 0);
    set_flag(FlagN, 0);
    set_flag(FlagH, (val & 0xF) + 1 > 0xF);
    return 0;
}

int handleDEC(const OpCode &op) {
    ushort val = 0;

    switch(op.params[0]) {
        case A: regAF.hi--; val = regAF.hi; break;
        case B: regBC.hi--; val = regBC.hi; break;
        case C: regBC.lo--; val = regBC.lo; break;
        case D: regDE.hi--; val = regDE.hi; break;
        case E: regDE.lo--; val = regDE.lo; break;
        case H: regHL.hi--; val = regHL.hi; break;
        case L: regHL.lo--; val = regHL.lo; break;
        case BC: (*((ushort *)&regBC))--; val = (*((ushort *)&regBC)); return 0; //cout << "DECD BC" << endl; return;
        case DE: (*((ushort *)&regDE))--; val = (*((ushort *)&regDE)); return 0;
        case HL: {
            if (op.mode == ATypeA) {
                byte b = bus::read((*((ushort *)&regHL)));
                b--;
                bus::write((*((ushort *)&regHL)), b);
                val = b;
            } else {
                (*((ushort *)&regHL))--; 
                val = (*((ushort *)&regHL)); 
                return 0;
            }
        } break;
        default:
            cout << "INVALID DEC FLAG" << endl;
            exit(-1);
            return 0;
    }

    //cout << "FUCKING WITH FLAGS..." << endl;
    set_flag(FlagZ, val == 0);
    set_flag(FlagN, 1);
    set_flag(FlagH, (val & 0xF) == 0x0F);
    return 0;
}

int handleRLA(const OpCode &op) {
    byte b = regAF.hi & (1 << 7);
    regAF.hi <<= 1;
    regAF.hi &= (b);
    return 0;
}

int handleRLCA(const OpCode &op) {
    byte b = regAF.hi & (1 << 7);
    regAF.hi <<= 1;
    
    if (b & (1 << 7)) {
        set_flag(FlagC, true);
    } else {
        set_flag(FlagC, false);
    }
    return 0;
}

int handleRRA(const OpCode &op) {
    byte b = regAF.hi & 1;
    regAF.hi >>= 1;
    regAF.hi &= (b << 7);
    return 0;
}

int handleCPL(const OpCode &op) {
    regAF.hi = ~regAF.hi;
    set_flag(FlagH, true);
    set_flag(FlagN, true);
    return 0;
}

int handleRRCA(const OpCode &op) {
    byte b = regAF.hi & 1;
    regAF.hi >>= 1;
    
    if (b) {
        set_flag(FlagC, true);
    } else {
        set_flag(FlagC, false);
    }
    return 0;
}

int handleDI(const OpCode &op) {
    interruptsEnabled = false;
    cout << "DISABLED INT" << endl;
    //sleep(3);
    return 0;
}

int handleEI(const OpCode &op) {
    eiCalled = true;
    cout << "ENABLED INT" << endl;
    //sleep(3);
    return 0;
}

int handleRST(const OpCode &op) {
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
    return 0;
}

int handleHALT(const OpCode &opCode) {
    haltWaitingForInterrupt = true;
    cout << "HALTING..." << endl;
    return 0;
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
    handlerMap[RETI] = handleRETI;
    handlerMap[DAA] = handleDAA;
    handlerMap[PUSH] = handlePUSH;
    handlerMap[POP] = handlePOP;
    handlerMap[CB] = handleCB;
    handlerMap[CPL] = handleCPL;
    handlerMap[HALT] = handleHALT;

    initParamTypeMap();
}

int handle_op(OpCode &opCode) {
    std::map<Op, HANDLER>::iterator it = handlerMap.find(opCode.op);

    if (it == handlerMap.end()) {
        cout << "UNKNOWN OP CODE: " << Byte(opCode.value) << endl;
        exit(-1);
    }

    int ret = it->second(opCode);

    if (opCode.op != EI && eiCalled) {
        eiCalled = false;
        interruptsEnabled = true;
    }

    return ret;
}

}
