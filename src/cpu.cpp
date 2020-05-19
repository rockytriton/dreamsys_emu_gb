#include "cpu.h"
#include "memory.h"

#include <unistd.h>
#include <iomanip>
#include <chrono>
#include <thread>

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


void run() {
    regPC = 0x100;
    init_handlers();

    while(true) {
        byte b = memory::read(regPC);

        if (b == 0xFF) {
            cout << endl << "DONE" << endl;
            return;
        }

        OpCode opCode = opCodes[b];

        cout << Short(regPC) << ": " << Byte(b) << " " << Byte(memory::read(regPC + 1)) << " " << Byte(memory::read(regPC + 2)) << " (" << opCode.name << ") "
             << " - AF: " << Short(toShort(regAF.hi, regAF.lo))
             << " - BC: " << Short(toShort(regBC.hi, regBC.lo))
             << " - DE: " << Short(toShort(regDE.hi, regDE.lo))
             << " - HL: " << Short(toShort(regHL.hi, regHL.lo))
             << endl;

        handle_op(opCode);

        regPC += opCode.length;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
    }
}

}
}

