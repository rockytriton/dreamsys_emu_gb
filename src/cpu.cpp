#include "cpu.h"
#include "memory.h"

#include <iomanip>

namespace dsemu {
namespace cpu {

void init_handlers();
void handle_op(OpCode opCode);

Register regAF;
Register regBC;
Register regDE;
Register regHL;

Register regSP;
ushort regPC = 0;

static inline bool get_flag(Flags n) {
    return regAF.lo & (1 << n);
}

static inline void set_flag(Flags n, bool val) {
    if (val) {
        regAF.lo |= (1 << n);
    } else {
        regAF.lo &= ~(1 << n);
    }
}

void run() {
    regPC = 0;
    init_handlers();

    while(true) {
        byte b = memory::read(regPC);

        cout << "READ OP CODE: " << std::hex << (int)b << endl;
        cout << "\tBC: " << *((short *)&regBC) << " A: " << (int)regAF.hi << endl;
        cout << "\tC0DE: " << std::hex << std::setfill('0') << std::setw(2) << (int)memory::ram[0xC0DE] << endl;

        if (b == 0xFF) {
            cout << endl << "DONE" << endl;
            return;
        }

        OpCode opCode = opCodes[b];
        handle_op(opCode);

        regPC += opCode.length;
    }
}

}
}

