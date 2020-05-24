#pragma once

#include "common.h"


namespace dsemu::ppu {

struct ScrollInfo {
    byte x;
    byte y;
};

extern byte lcdControl;
extern ScrollInfo scrollInfo;

const int HZ = 1048576;
const int LINES_PER_FRAME = 154;
const int TICKS_PER_LINE = 114;
const int TICKS_PER_FRAME = LINES_PER_FRAME * TICKS_PER_LINE;
const int OAM_TICKS = 20;
const int PIXEL_TICKS = 43;
const int VBLANK_LINE = 144;

void tick();
void init();

byte getCurrentLine();

byte readOAM(ushort address);
void writeOAM(ushort address, byte b);

inline byte getXScroll() { return scrollInfo.x; }
inline byte getYScroll() { return scrollInfo.y; }

inline void setXScroll(byte b) { scrollInfo.x = b; }
inline void setYScroll(byte b) { scrollInfo.y = b; }

}



