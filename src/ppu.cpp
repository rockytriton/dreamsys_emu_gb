#include "ppu.h"
#include "cpu.h"
#include "memory.h"
#include "io.h"
#include "ui.h"

#include <chrono>
#include <thread>
#include <unistd.h>
#include <cstring>
#include <SDL2/SDL.h>

namespace dsemu::ppu {


byte lcdControl;

byte lcdStats = 0;

ScrollInfo scrollInfo;
int currentFrame = 0;
byte currentLine = 0;
byte oamRAM[160];

unsigned long **videoBuffer;

void init() {
    currentFrame = 0;
    currentLine = 0;
    memset(oamRAM, 0, sizeof(oamRAM));

    videoBuffer = new unsigned long *[256];

    cout << "Video Buffer: " << Int64((uint64_t)videoBuffer) << endl;

    for (int i=0; i<256; i++) {
        videoBuffer[i] = new unsigned long[256];
        //cout << "Video Buffer[" << i << "] = " << Int64((uint64_t)videoBuffer[i]) << endl;
        //sleepMs(500);
    }

    for (int y=0; y<256; y++) {
        for (int x=0; x<256; x++) {
            videoBuffer[y][x] = 0;
        }
    }

    //memset(videoBuffer, 0, 256 * 256 * sizeof(unsigned long));

}

void drawFrame() {

    cout << "SPRITES: " << endl;
    for (int i=0; i<160; i += 4) {
        OAMEntry *entry = (OAMEntry *)&oamRAM[i];
        
        cout << Byte(entry->x) << "," << Byte(entry->y) << "-" << Byte(entry->tile) << " ";

        if ((i % 40) == 0) {
            cout << endl;
        }
    }

    cout << endl;


    cout  << endl << "LCD Status: " << endl
         << "\tbgDisplay: " << Byte(bgDisplay()) << endl
         << "\tspriteDisplay: " << Byte(spriteDisplay()) << endl
         << "\tspriteSize8x16: " << Byte(spriteSize8x16()) << endl
         << "\tbgMapStart: " << Short(bgMapStart()) << endl
         << "\tbgTileStart: " << Short(bgTileStart()) << endl
         << "\twindowDisplay: " << Byte(windowDisplay()) << endl
         << "\twindowMapSelect: " << Short(windowMapSelect()) << endl
         << "\tlcdOn: " << Byte(lcdOn()) << endl << endl;

    //if (!DEBUG) return;

}

byte getCurrentLine() {
    return currentLine;
}
byte readOAM(ushort address) {


    //cout << "READ DUMPING OAM RAM: " << Short(address) << " - " << Byte(memory::ram[address]) << " - " << endl;

    for (int i=0; i<160; i++) {
        if ((i % 32) == 0) cout << endl;
        //cout << " " << Byte(oamRAM[i]);
    }

    cout << endl;

    //sleep(2);


    return oamRAM[address];
}

void writeOAM(ushort address, byte b) {
    oamRAM[address] = b;

/*
    cout << "WRITE DUMPING OAM RAM: " << Short(address) << " - " << Byte(memory::ram[address]) << " - " << Byte(b) << endl;

    for (int i=0; i<160; i++) {
        if ((i % 32) == 0) cout << endl;
        cout << " " << Byte(oamRAM[i]);
    }

    cout << endl;
*/

    //sleep(2);
}

long targetFrameTime = 1000 / 60;
long start = SDL_GetTicks();
int count = 0;
long prev = SDL_GetTicks();
static unsigned long colors[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000};

void drawLine(int lineNum) {
    cout << "Drawing Line: " << lineNum << endl;

   // sleep(5);

    for (int x=0; x<256; x += 8) {

        //cout << "Draw line X: " << x << endl;
        unsigned long *pLine = videoBuffer[lineNum];

        //cout << Int64((uint64_t)pLine) << endl;

        //cout << "Got pline" << endl;
        ushort bgTileNum = memory::read(ppu::bgMapStart() + x + (lineNum * 32));
        //cout << "Got tile" << endl;
        
        if (ppu::bgTileStart() == 0x8800) {
            bgTileNum += 128;
        }

        byte tileY = lineNum % 8;

        //cout << "Getting b1" << endl;
        byte b1 = memory::read(ppu::bgTileStart() + (bgTileNum * 16) + tileY);
        //cout << "Getting b2" << endl;
        byte b2 = memory::read(ppu::bgTileStart() + (bgTileNum * 16) + 1 + tileY);

        //cout << "looping" << endl;

        for (int bit=7, n=0; bit >= 0; bit--, n++) {
            byte hi = !!(b1 & (1 << bit)) << 1;
            byte lo = !!(b2 & (1 << bit));

            byte color = hi|lo;
            unsigned long c = colors[color];
            pLine[x + n] = c;

        }
/*
        int bit = x % 8;
        byte hi = !!(b1 & (1 << bit)) << 1;
        byte lo = !!(b2 & (1 << bit));
        byte color = hi|lo;
        unsigned long c = colors[color];
        pLine[x] = c;
*/

        //cout << "Looped" << endl;
    }


    /*
    for (int tileY=0; tileY<16; tileY += 2) {
        byte b1 = memory::read(startLocation + (tileNum * 16) + tileY);
        byte b2 = memory::read(startLocation + (tileNum * 16) + 1 + tileY);
        
        for (int bit=7, n=0; bit >= 0; bit--, n++) {
            byte hi = !!(b1 & (1 << bit)) << 1;
            byte lo = !!(b2 & (1 << bit));

            byte color = hi|lo;

            rc.x = x + (n * scale);
            rc.y = y + (tileY / 2 * scale);
            rc.w = scale;
            rc.h = scale;

            SDL_FillRect(surface, &rc, colors[color]);
        }
    }
    */

   //cout << "DREW" << endl;
}

void tick() {
    int f = cpu::getTickCount() % (TICKS_PER_FRAME);
    int l = f / 114;

    if (l != currentLine && l < 144) {
        if (DEBUG) cout << "PPU:> NEW LINE: " << l << " FRAME: " << currentFrame << endl;

        drawLine(currentLine);
    }

    if (lcdStats & 0x40 && memory::read(0xFF45) == l) {
        cpu::handleInterrupt(2, true, false);
    }
    
    if (l != currentLine && l == 144) {
        long end = SDL_GetTicks();
		long frameTime = end - prev;

		if (frameTime < targetFrameTime) 
		{
			SDL_Delay(targetFrameTime - frameTime);
		}

		if (end - start >= 1000) {
			cout << "FPS: " << count << endl;
			count = 0;
			start = end;
		}

		count++;

        currentFrame++;
        drawFrame();
        cout << endl << "PPU:> NEW FRAME: " << currentFrame << endl << endl;

        //std::this_thread::sleep_for(std::chrono::milliseconds(100));

        cpu::handleInterrupt(cpu::IVBlank, true, false);

        prev = SDL_GetTicks();
    }

    currentLine = l;

    //if (!DEBUG) return;

}

}



/*




    SDL_Rect rc;
    rc.x = 10;
    rc.y = 10;
    rc.w = 5;
    rc.h = 5;

    SDL_FillRect(screen, &rc, 0xFF00FFFF);

    int drawX = 0;
    int drawY = 0;
    int scale = 1;

    //cout << "READING TILES: ";

    for (int y=0; y<32; y++) {
    for (int x=0; x<32; x++) {
        byte tileNum = memory::read((x + (y *32)) + bgMapStart());
        //tileNum = 1;
        ushort tileStart = bgTileStart() + (tileNum * 16);
        //cout << Byte(tileNum) << "-";

        drawY = (y * 8 * scale);

        for (int i=0; i<16; i += 2) {
		    //int tileaddr = 0x8000 +  0*0x100 + tileNum*16 + i;
            byte b1 = memory::read(tileStart + i);
            byte b2 = memory::read(tileStart + i + 1);
            //byte b1 = memory::read(tileaddr);
            //byte b2 = memory::read(tileaddr +1);

            for (int bit=7; bit>=0; bit--) {
                byte hiBit = (!!(b1 & (1 << bit)) << 1);
                byte loBit = (!!(b2 & (1 << bit)));

                byte color = hiBit | loBit;
                rc.x = drawX + ((7 - bit) * scale);
                rc.y = drawY;
                rc.w = scale;
                rc.h = scale;

                SDL_FillRect(screen, &rc, colours[color]);
            }

            drawY += scale;
        }
        

        drawX += (8 * scale);
    }
    drawX = 0;
    }

    cout << endl;
*/