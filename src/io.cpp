#include "io.h"
#include "ppu.h"
#include "cpu.h"
#include "memory.h"
#include "bus.h"

#include <map>
#include <unistd.h>

namespace dsemu::io {


typedef byte (*IO_READ_HANDLER)();
typedef void (*IO_WRITE_HANDLER)(byte b);

typedef std::map<ushort, std::pair<IO_READ_HANDLER, IO_WRITE_HANDLER>> HANDLER_MAP;

HANDLER_MAP handlerMap;

byte readScrollX() {
    return ppu::getXScroll();
}

void writeScrollX(byte b) {
    ppu::setXScroll(b);
}

byte lcdStats = 0;

byte readLCDStats() {
    cout << "READING LCD STATS: " << endl;
    return lcdStats;
}

void writeLCDStats(byte b) {
    //cout << "WRITING LCD STATS: " << Byte(b) << endl;
    lcdStats = b;
}

byte lcdControl = 0;

byte readLCDControl() {
    cout << "READING LCD STATS: " << endl;
    return lcdControl;
}

void writeLCDControl(byte b) {
    cout << "WRITING LCD CONTROL: " << Byte(b) << endl;

    sleepMs(100);

    lcdControl = b;
}

void writeDMA(byte b) {
    cout << endl << "DOING DMA WRITE: " << Byte(b) << endl << endl;
    //sleep(7);

    for (int i=0; i<0xA0; i++) {
        byte d = bus::read((b * 0x100) + i);
        bus::write(0xFE00 + i, d);
    }
    /*
        80
        02
        02
        02
        03
        03
        04
        04
        04
        05
        05
        06
    */


    //sleep(4);

    cout << "VRAM: " << endl;

    for (int i=0x8000; i<0x80FF; i++) {
        if ((i % 32) == 0) {
            cout << endl;
        }

        cout << " " << Byte(memory::ram[i]);
    }

    sleep(1);
}

byte readDMS() {
    cout << endl << "DOING DMA READ" << endl << endl;
    sleep(2);
    return 0;
}

#define ADD_MEMORY_HANDLER(X) handlerMap[X] = std::make_pair([]() -> byte { return memory::read(X); }, [](byte b) -> void { memory::write(X, b); });

void init() {
    IO_WRITE_HANDLER noWrite = [](byte b) -> void { };

    handlerMap[0xFF43] = std::make_pair(readScrollX, writeScrollX);
    handlerMap[0xFF42] = std::make_pair(ppu::getYScroll, ppu::setYScroll);
    handlerMap[0xFF40] = std::make_pair(readLCDControl, writeLCDControl);
    handlerMap[0xFF41] = std::make_pair(readLCDStats, writeLCDStats);
    handlerMap[0xFF44] = std::make_pair(ppu::getCurrentLine, noWrite);
    handlerMap[0xFF46] = std::make_pair(readDMS, writeDMA);

    ADD_MEMORY_HANDLER(0xFF47);
    ADD_MEMORY_HANDLER(0xFF48);
    ADD_MEMORY_HANDLER(0xFF49);
}


byte read(ushort address) {

    HANDLER_MAP::iterator it = handlerMap.find(address);
    
    if (it != handlerMap.end()) {
        return it->second.first();
    }

    if (address == 0xFF00) {
        //cout << endl << "JOYPAD READ: " << endl;
        //sleep(2);
        return 0;
    } else if (address == 0xFF01) {
        //cout << endl << "SERIAL READ: " << endl;
        //sleep(2);
        return 0;
    } else if (address == 0xFF02) {
        //cout << endl << "SERIAL IO: " << endl;
        //sleep(2);
        return 0;
    } else if (address == 0xFF03) {
        //cout << endl << "DIV IO: " << endl;
        //sleep(2);
        return 0;
    } else if (address == 0xFF04) {
        //cout << endl << "TIMA IO: " << endl;
        //sleep(2);
        return 0;
    } else if (address == 0xFF44) {
        return (byte)ppu::getCurrentLine();
        //cout << endl << "READ LCD: " << endl;
        //sleep(2);
    } else if (address == 0xFF0F) {
        return cpu::getInterruptsRequestsFlag();
    } else if (address == 0xFFFF) {
        return cpu::getInterruptsEnableFlag();
    }

    if (DEBUG) cout << "UNKNOWN IO READ: " << Short(address) << endl;
    //sleep(5);
    return 0;
}

void write(ushort address, byte b) {

    HANDLER_MAP::iterator it = handlerMap.find(address);
    
    if (it != handlerMap.end()) {
        it->second.second(b);
        return;
    }

    if (DEBUG) cout << " NO WRITE MAP: " << Short(address) << endl;
    //sleep(1);

    if (address == 0xFF00) {
        //cout << endl << "JOYPAD WRITE: " << endl;
       // sleep(2);
        return;
    } else if (address == 0xFF01) {
       // cout << endl << "SERIAL WRITE: " << endl;
        //sleep(2);
        return;
    } else if (address == 0xFF02) {
        //cout << endl << "SERIAL WRITE: " << endl;
        //sleep(2);
        return;
    } else if (address == 0xFF03) {
       // cout << endl << "DIV WRITE: " << endl;
       // sleep(2);
        return;
    } else if (address == 0xFF04) {
       // cout << endl << "TIMA WRITE: " << endl;
        //sleep(2);
        return;
    } else if (address == 0xFF0F) {
        cpu::setInterruptsRequestsFlag(b);
        return;
    } else if (address == 0xFFFF) {
        cpu::setInterruptsEnableFlag(b);
        return;
    }

    if (DEBUG) cout << "UNKNOWN IO WRITE: " << Short(address) << endl;
    //leep(5);
}

}

