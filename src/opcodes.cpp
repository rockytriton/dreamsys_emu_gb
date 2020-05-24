
#include "cpu.h"

namespace dsemu::cpu {

OpCode opCodes[] = {
    {0x00, "NOP", NOP, 1, 4},
    {0x01, "LD BC,nn", LD, 3, 12, ATypeIR, {BC, NN}},
    {0x02, "LD (BC),A", LD, 1, 8, ATypeRA, {BC, A}},
    {0x03, "INC BC", INC, 1, 8, ATypeNA, {BC}},
    {0x04, "INC B", INC, 1, 4, ATypeNA, {B}},
    {0x05, "DEC B", DEC, 1, 4, ATypeNA, {B}},
    {0x06, "LD B,n", LD, 2, 8, ATypeIR, {B, N}},
    {0x07, "RLCA", RLCA, 1, 4, ATypeNA},
    {0x08, "LD (nn),SP", LD, 3, 20, ATypeRA, {NN, SP}},
    {0x09, "ADD HL,BC", ADD, 1, 8, ATypeNA, {HL, BC}},
    {0x0A, "LD A,(BC)", LD, 1, 8, ATypeAR, {A, BC}},
    {0x0B, "DEC BC", DEC, 1, 8, ATypeNA, {BC}},
    {0x0C, "INC C", INC, 1, 4, ATypeNA, {C}},
    {0x0D, "DEC C", DEC, 1, 4, ATypeNA, {C}},
    {0x0E, "LD C,n", LD, 2, 8, ATypeIR, {C, N}},
    {0x0F, "RRCA", RRCA, 1, 4, ATypeNA},

    {0x10, "STOP", STOP, 2, 4},
    {0x11, "LD DE,nn", LD, 3, 12, ATypeIR, {DE, NN}},
    {0x12, "LD (DE),A", LD, 1, 8, ATypeRA, {DE, A}},
    {0x13, "INC DE", INC, 1, 8, ATypeNA, {DE}},
    {0x14, "INC D", INC, 1, 4, ATypeNA, {D}},
    {0x15, "DEC D", DEC, 1, 4, ATypeNA, {D}},
    {0x16, "LD D,n", LD, 2, 8, ATypeIR, {D, N}},
    {0x17, "RLA", RLA, 1, 4},
    {0x18, "JR e", LD, 2, 12, ATypeJ, {N}},
    {0x19, "ADD HL,DE", ADD, 1, 8, ATypeNA, {HL, DE}},
    {0x1A, "LD A,(DE)", LD, 1, 8, ATypeAR, {A, DE}},
    {0x1B, "DEC DE", DEC, 1, 8, ATypeNA, {DE}},
    {0x1C, "INC E", INC, 1, 4, ATypeNA, {E}},
    {0x1D, "DEC E", DEC, 1, 4, ATypeNA, {E}},
    {0x1E, "LD E,n", LD, 2, 8, ATypeIR, {E, N}},
    {0x1F, "RRA", RRA, 1, 4, ATypeNA},

    {0x20, "JR NZ,e", JR, 2, 8, ATypeJ_NZ, {N}},
    {0x21, "LD HL,nn", LD, 3, 12, ATypeIR, {HL, NN}},
    {0x22, "LDI (HL),A", LDI, 1, 8, ATypeRA, {HL, A}},
    {0x23, "INC HL", INC, 1, 8, ATypeNA, {HL}},
    {0x24, "INC H", INC, 1, 4, ATypeNA, {H}},
    {0x25, "DEC H", DEC, 1, 4, ATypeNA, {H}},
    {0x26, "LD H,n", LD, 2, 8, ATypeIR, {H, N}},
    {0x27, "DAA", DAA, 1, 4},
    {0x28, "JR Z,e", JR, 2, 8, ATypeJ_Z, {N}},
    {0x29, "ADD HL,HL", ADD, 1, 8, ATypeNA, {HL, HL}},
    {0x2A, "LDI A,(HL)", LDI, 1, 8, ATypeAR, {A, HL}},
    {0x2B, "DEC HL", DEC, 1, 8, ATypeNA, {HL}},
    {0x2C, "INC L", INC, 1, 4, ATypeNA, {L}},
    {0x2D, "DEC L", DEC, 1, 4, ATypeNA, {L}},
    {0x2E, "LD L,n", LD, 2, 8, ATypeIR, {L, N}},
    {0x2F, "CPL", CPL, 1, 4, ATypeNA},

    {0x30, "JR NC,e", JR, 2, 8, ATypeJ_NC},
    {0x31, "LD SP,nn", LD, 3, 12, ATypeIR, {SP, NN}},
    {0x32, "LDD (HL),A", LDD, 1, 8, ATypeRA, {HL, A}},
    {0x33, "INC SP", INC, 1, 8, ATypeNA, {SP}},
    {0x34, "INC (HL)", INC, 1, 12, ATypeA, {HL}},
    {0x35, "DEC HL", DEC, 1, 12, ATypeA, {HL}},
    {0x36, "LD (HL),n", LD, 2, 12, ATypeRA, {HL, N}},
    {0x37, "DAA", SCF, 1, 4},
    {0x38, "JR C,e", LD, 2, 8, ATypeJ_C, {N}},
    {0x39, "ADD HL,SP", ADD, 1, 8, ATypeNA, {HL, SP}},
    {0x3A, "LDD A,(HL)", LDD, 1, 8, ATypeAR, {A, HL}},
    {0x3B, "DEC SP", DEC, 1, 8, ATypeNA, {SP}},
    {0x3C, "INC A", INC, 1, 4, ATypeNA, {A}},
    {0x3D, "DEC A", DEC, 1, 4, ATypeNA, {A}},
    {0x3E, "LD A,n", LD, 2, 8, ATypeIR, {A, N}},
    {0x3F, "CCF", CCF, 1, 4, ATypeNA},

    {0x40, "LD B,B", LD, 1, 4, ATypeRR, {B, B}},
    {0x41, "LD B,C", LD, 1, 4, ATypeRR, {B, C}},
    {0x42, "LD B,D", LD, 1, 4, ATypeRR, {B, D}},
    {0x43, "LD B,E", LD, 1, 4, ATypeRR, {B, E}},
    {0x44, "LD B,H", LD, 1, 4, ATypeRR, {B, H}},
    {0x45, "LD B,L", LD, 1, 4, ATypeRR, {B, L}},
    {0x46, "LD B,(HL)", LD, 1, 8, ATypeAR, {B, HL}},
    {0x47, "LD B,A", LD, 1, 4, ATypeRR, {B, A}},
    {0x48, "LD C,B", LD, 1, 4, ATypeRR, {C, B}},
    {0x49, "LD C,C", LD, 1, 4, ATypeRR, {C, C}},
    {0x4A, "LD C,D", LD, 1, 4, ATypeRR, {C, D}},
    {0x4B, "LD C,E", LD, 1, 4, ATypeRR, {C, E}},
    {0x4C, "LD C,H", LD, 1, 4, ATypeRR, {C, H}},
    {0x4D, "LD C,L", LD, 1, 4, ATypeRR, {C, L}},
    {0x4E, "LD C,(HL)", LD, 1, 8, ATypeAR, {C, HL}},
    {0x4F, "LD C,A", LD, 1, 4, ATypeRR, {C, A}},

    {0x50, "LD D,B", LD, 1, 4, ATypeRR, {D, B}},
    {0x51, "LD D,C", LD, 1, 4, ATypeRR, {D, C}},
    {0x52, "LD D,D", LD, 1, 4, ATypeRR, {D, D}},
    {0x53, "LD D,E", LD, 1, 4, ATypeRR, {D, E}},
    {0x54, "LD D,H", LD, 1, 4, ATypeRR, {D, H}},
    {0x55, "LD D,L", LD, 1, 4, ATypeRR, {D, L}},
    {0x56, "LD D,(HL)", LD, 1, 8, ATypeAR, {D, HL}},
    {0x57, "LD D,A", LD, 1, 4, ATypeRR, {D, A}},
    {0x58, "LD E,B", LD, 1, 4, ATypeRR, {E, B}},
    {0x59, "LD E,C", LD, 1, 4, ATypeRR, {E, C}},
    {0x5A, "LD E,D", LD, 1, 4, ATypeRR, {E, D}},
    {0x5B, "LD E,E", LD, 1, 4, ATypeRR, {E, E}},
    {0x5C, "LD E,H", LD, 1, 4, ATypeRR, {E, H}},
    {0x5D, "LD E,L", LD, 1, 4, ATypeRR, {E, L}},
    {0x5E, "LD E,(HL)", LD, 1, 8, ATypeAR, {E, HL}},
    {0x5F, "LD E,A", LD, 1, 4, ATypeRR, {E, A}},

    {0x60, "LD H,B", LD, 1, 4, ATypeRR, {H, B}},
    {0x61, "LD H,C", LD, 1, 4, ATypeRR, {H, C}},
    {0x62, "LD H,D", LD, 1, 4, ATypeRR, {H, D}},
    {0x63, "LD H,E", LD, 1, 4, ATypeRR, {H, E}},
    {0x64, "LD H,H", LD, 1, 4, ATypeRR, {H, H}},
    {0x65, "LD H,L", LD, 1, 4, ATypeRR, {H, L}},
    {0x66, "LD H,(HL)", LD, 1, 8, ATypeAR, {H, HL}},
    {0x67, "LD H,A", LD, 1, 4, ATypeRR, {H, A}},
    {0x68, "LD L,B", LD, 1, 4, ATypeRR, {L, B}},
    {0x69, "LD L,C", LD, 1, 4, ATypeRR, {L, C}},
    {0x6A, "LD L,D", LD, 1, 4, ATypeRR, {L, D}},
    {0x6B, "LD L,E", LD, 1, 4, ATypeRR, {L, E}},
    {0x6C, "LD L,H", LD, 1, 4, ATypeRR, {L, H}},
    {0x6D, "LD L,L", LD, 1, 4, ATypeRR, {L, L}},
    {0x6E, "LD L,(HL)", LD, 1, 8, ATypeAR, {L, HL}},
    {0x6F, "LD L,A", LD, 1, 4, ATypeRR, {L, A}},

    {0x70, "LD (HL),B", LD, 1, 8, ATypeRA, {HL, B}},
    {0x71, "LD (HL),C", LD, 1, 8, ATypeRA, {HL, C}},
    {0x72, "LD (HL),D", LD, 1, 8, ATypeRA, {HL, D}},
    {0x73, "LD (HL),E", LD, 1, 8, ATypeRA, {HL, E}},
    {0x74, "LD (HL),H", LD, 1, 8, ATypeRA, {HL, H}},
    {0x75, "LD (HL),L", LD, 1, 8, ATypeRA, {HL, L}},
    {0x76, "LD HALT", HALT, 1, 4, ATypeNA},
    {0x77, "LD (HL),A", LD, 1, 8, ATypeRA, {HL, A}},
    {0x78, "LD A,B", LD, 1, 4, ATypeRR, {A, B}},
    {0x79, "LD A,C", LD, 1, 4, ATypeRR, {A, C}},
    {0x7A, "LD A,D", LD, 1, 4, ATypeRR, {A, D}},
    {0x7B, "LD A,E", LD, 1, 4, ATypeRR, {A, E}},
    {0x7C, "LD A,H", LD, 1, 4, ATypeRR, {A, H}},
    {0x7D, "LD A,L", LD, 1, 4, ATypeRR, {A, L}},
    {0x7E, "LD A,(HL)", LD, 1, 8, ATypeAR, {A, HL}},
    {0x7F, "LD A,A", LD, 1, 4, ATypeRR, {A, A}},

    {0x80, "ADD B", ADD, 1, 4, ATypeR, {A, B}},
    {0x81, "ADD C", ADD, 1, 4, ATypeR, {A, C}},
    {0x82, "ADD D", ADD, 1, 4, ATypeR, {A, D}},
    {0x83, "ADD E", ADD, 1, 4, ATypeR, {A, E}},
    {0x84, "ADD H", ADD, 1, 4, ATypeR, {A, H}},
    {0x85, "ADD L", ADD, 1, 4, ATypeR, {A, L}},
    {0x86, "ADD HL", ADD, 1, 8, ATypeA, {A, HL}},
    {0x87, "ADD A", ADD, 1, 4, ATypeR, {A, A}},
    {0x88, "ADC B", ADC, 1, 4, ATypeR, {A, B}},
    {0x89, "ADC C", ADC, 1, 4, ATypeR, {A, C}},
    {0x8a, "ADC D", ADC, 1, 4, ATypeR, {A, D}},
    {0x8b, "ADC E", ADC, 1, 4, ATypeR, {A, E}},
    {0x8c, "ADC H", ADC, 1, 4, ATypeR, {A, H}},
    {0x8d, "ADC L", ADC, 1, 4, ATypeR, {A, L}},
    {0x8e, "ADC HL", ADC, 1, 8, ATypeA, {A, HL}},
    {0x8f, "ADC A", ADC, 1, 4, ATypeR, {A, A}},

    {0x90, "SUB B", SUB, 1, 4, ATypeR, {B}},
    {0x91, "SUB C", SUB, 1, 4, ATypeR, {C}},
    {0x92, "SUB D", SUB, 1, 4, ATypeR, {D}},
    {0x93, "SUB E", SUB, 1, 4, ATypeR, {E}},
    {0x94, "SUB H", SUB, 1, 4, ATypeR, {H}},
    {0x95, "SUB L", SUB, 1, 4, ATypeR, {L}},
    {0x96, "SUB HL", SUB, 1, 8, ATypeA, {HL}},
    {0x97, "SUB A", SUB, 1, 4, ATypeR, {A}},
    {0x98, "SBC B", SBC, 1, 4, ATypeR, {B}},
    {0x99, "SBC C", SBC, 1, 4, ATypeR, {C}},
    {0x9a, "SBC D", SBC, 1, 4, ATypeR, {D}},
    {0x9b, "SBC E", SBC, 1, 4, ATypeR, {E}},
    {0x9c, "SBC H", SBC, 1, 4, ATypeR, {H}},
    {0x9d, "SBC L", SBC, 1, 4, ATypeR, {L}},
    {0x9e, "SBC HL", SBC, 1, 8, ATypeA, {HL}},
    {0x9f, "SBC A", SBC, 1, 4, ATypeR, {A}},

    {0xA0, "AND B", AND, 1, 4, ATypeR, {B}},
    {0xA1, "AND C", AND, 1, 4, ATypeR, {C}},
    {0xA2, "AND D", AND, 1, 4, ATypeR, {D}},
    {0xA3, "AND E", AND, 1, 4, ATypeR, {E}},
    {0xA4, "AND H", AND, 1, 4, ATypeR, {H}},
    {0xA5, "AND L", AND, 1, 4, ATypeR, {L}},
    {0xA6, "AND HL", AND, 1, 8, ATypeA, {HL}},
    {0xA7, "AND A", AND, 1, 4, ATypeR, {A}},
    {0xA8, "XOR B", XOR, 1, 4, ATypeR, {B}},
    {0xA9, "XOR C", XOR, 1, 4, ATypeR, {C}},
    {0xAa, "XOR D", XOR, 1, 4, ATypeR, {D}},
    {0xAb, "XOR E", XOR, 1, 4, ATypeR, {E}},
    {0xAc, "XOR H", XOR, 1, 4, ATypeR, {H}},
    {0xAd, "XOR L", XOR, 1, 4, ATypeR, {L}},
    {0xAe, "XOR HL", XOR, 1, 8, ATypeA, {HL}},
    {0xAf, "XOR A", XOR, 1, 4, ATypeR, {A}},

    {0xB0, "OR B", OR, 1, 4, ATypeR, {B}},
    {0xB1, "OR C", OR, 1, 4, ATypeR, {C}},
    {0xB2, "OR D", OR, 1, 4, ATypeR, {D}},
    {0xB3, "OR E", OR, 1, 4, ATypeR, {E}},
    {0xB4, "OR H", OR, 1, 4, ATypeR, {H}},
    {0xB5, "OR L", OR, 1, 4, ATypeR, {L}},
    {0xB6, "OR HL", OR, 1, 8, ATypeA, {HL}},
    {0xB7, "OR A", OR, 1, 4, ATypeR, {A}},
    {0xB8, "CP B", CP, 1, 4, ATypeR, {B}},
    {0xB9, "CP C", CP, 1, 4, ATypeR, {C}},
    {0xBa, "CP D", CP, 1, 4, ATypeR, {D}},
    {0xBb, "CP E", CP, 1, 4, ATypeR, {E}},
    {0xBc, "CP H", CP, 1, 4, ATypeR, {H}},
    {0xBd, "CP L", CP, 1, 4, ATypeR, {L}},
    {0xBe, "CP HL", CP, 1, 8, ATypeA, {HL}},
    {0xBf, "CP A", CP, 1, 4, ATypeR, {A}},

    {0xC0, "RET NZ", RET, 1, 8, ATypeJ_NZ},
    {0xC1, "POP BC", POP, 1, 12, ATypeNA, {BC}},
    {0xC2, "JP NZ,nn", JP, 3, 12, ATypeJ_NZ, {NN}},
    {0xC3, "JP nn", JP, 3, 16, ATypeJ, {NN}},
    {0xC4, "CALL NZ,nn", CALL, 3, 12, ATypeJ_NZ, {NN}},
    {0xC5, "PUSH BC", PUSH, 1, 16, ATypeNA, {BC}},
    {0xC6, "ADD n", ADD, 2, 8, ATypeNA, {A, N}},
    {0xC7, "RST 0x00", RST, 1, 16, ATypeNA, {x00}},
    {0xC8, "RET Z", RET, 1, 8, ATypeJ_Z},
    {0xC9, "RET", RET, 1, 16, ATypeJ},
    {0xCA, "JP Z,nn", JP, 3, 12, ATypeJ_Z, {NN}},
    {0xCB, "CB op", CB, 2, 8, ATypeJ},
    {0xCC, "CALL Z,nn", CALL, 3, 12, ATypeJ_Z, {NN}},
    {0xCD, "CALL nn", CALL, 3, 24, ATypeJ, {NN}},
    {0xCE, "ADC n", ADC, 2, 8, ATypeNA, {N}},
    {0xCF, "RST 0x08", RST, 1, 16, ATypeNA, {x08}},

    {0xD0, "RET NC", RET, 1, 8, ATypeJ_NC},
    {0xD1, "POP DE", POP, 1, 12, ATypeNA, {DE}},
    {0xD2, "JP NC,nn", JP, 3, 12, ATypeJ_NC, {NN}},
    {0xD3, "", X, 1},
    {0xD4, "CALL NC,nn", CALL, 3, 12, ATypeJ_NC, {NN}},
    {0xD5, "PUSH DE", PUSH, 1, 16, ATypeNA, {DE}},
    {0xD6, "SUB n", SUB, 2, 8, ATypeNA, {N}},
    {0xD7, "RST 0x10", RST, 1, 16, ATypeNA, {x10}},
    {0xD8, "RET C", RET, 1, 8, ATypeJ_C},
    {0xD9, "RETI", RETI, 1, 16, ATypeJ},
    {0xDA, "JP C,nn", JP, 3, 12, ATypeJ_C, {NN}},
    {0xDB, "", X, 1},
    {0xDC, "CALL C,nn", CALL, 3, 12, ATypeJ_C, {NN}},
    {0xDD, "", X, 1},
    {0xDE, "SBC n", SBC, 2, 8, ATypeNA, {N}},
    {0xDF, "RST 0x08", RST, 1, 16, ATypeNA, {x18}},

    {0xE0, "LDH (n),A", LDH, 2, 12, ATypeRA, {N, A}},
    {0xE1, "POP HL", POP, 1, 12, ATypeNA, {HL}},
    {0xE3, "LDH (C),A", LDH, 1, 8, ATypeRA, {C, A}},
    {0xE4, "", X, 1},
    {0xE5, "", X, 1},
    {0xE5, "PUSH HL", PUSH, 1, 16, ATypeNA, {HL}},
    {0xE6, "AND n", AND, 2, 8, ATypeNA, {N}},
    {0xE7, "RST 0x20", RST, 1, 16, ATypeNA, {x20}},
    {0xE8, "ADD SP,e", ADD, 2, 16, ATypeSP, {SP, E}},
    {0xE9, "JP HL", JP, 1, 4, ATypeJ, {HL}},
    {0xEA, "LD (nn),A", LD, 3, 16, ATypeRA, {NN, A}},
    {0xEB, "", X, 1},
    {0xEC, "", X, 1},
    {0xED, "", X, 1},
    {0xEE, "XOR n", XOR, 2, 8, ATypeNA, {N}},
    {0xEF, "RST 0x28", RST, 1, 16, ATypeNA, {x28}},

    {0xF0, "LDH A,(n)", LDH, 2, 12, ATypeRA, {A, N}},
    {0xF1, "POP AF", POP, 1, 12, ATypeNA, {AF}},
    {0xF2, "LDH A,(C)", LDH, 1, 8, ATypeRA, {A, C}},
    {0xF3, "DI", DI, 1, 4},
    {0xF4, "", X, 1},
    {0xF5, "PUSH AF", PUSH, 1, 16, ATypeNA, {AF}},
    {0xF6, "OR n", OR, 2, 8, ATypeNA, {N}},
    {0xF7, "RST 0x30", RST, 1, 16, ATypeNA, {x30}},
    {0xF8, "LD HL,SP+e", LD, 2, 12, ATypeSP, {HL, E}},
    {0xF9, "LD SP,HL", LD, 1, 8, ATypeNA, {SP, HL}},
    {0xFA, "LD A,(nn)", LD, 3, 16, ATypeAR, {A, NN}},
    {0xFB, "EI", EI, 1, 4},
    {0xFC, "", X, 1},
    {0xFD, "", X, 1},
    {0xFE, "CP n", CP, 2, 8, ATypeNA, {N}},
    {0xFF, "RST 0x38", RST, 1, 16, ATypeNA, {x38}},
};

}
