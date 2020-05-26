#pragma once

#include "common.h"

namespace dsemu::io {

void init();
byte read(ushort address);
void write(ushort address, byte b);

extern bool aDown ;
extern bool startDown;

}
