#include "cpu.h"
#include "memory.h"

#include <map>
#include <utility>

ushort toShort(byte a, byte b) {
    ushort s = a << 8;
    s |= b;

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
    paramTypeMap[BC] = std::make_pair(&regBC, RPT16);
    paramTypeMap[SP] = std::make_pair(&regSP, RPT16);

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

void handleLDI(const OpCode &op) {
    
}

void handleNOP(const OpCode &op) {

}

void init_handlers() {
    handlerMap[LD] = handleLD;
    handlerMap[LDI] = handleLDI;
    handlerMap[NOP] = handleNOP;

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
