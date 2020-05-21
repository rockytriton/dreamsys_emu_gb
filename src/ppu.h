#pragma once

#include "common.h"


namespace dsemu::ppu {

const int HZ = 1048576;
const int LINES_PER_FRAME = 154;
const int TICKS_PER_LINE = 114;
const int TICKS_PER_FRAME = LINES_PER_FRAME * TICKS_PER_LINE;
const int OAM_TICKS = 20;
const int PIXEL_TICKS = 43;
const int VBLANK_LINE = 144;

void tick();
void init();

int getCurrentLine();

}



