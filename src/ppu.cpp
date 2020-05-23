#include "ppu.h"
#include "cpu.h"
#include "memory.h"

#include <chrono>
#include <thread>
#include <unistd.h>
#include <SDL2/SDL.h>

namespace dsemu::ppu {

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
static SDL_Surface *screen;

ScrollInfo scrollInfo;
int currentFrame = 0;
byte currentLine = 0;

byte oamRAM[160];

void init() {
    currentFrame = 0;
    currentLine = 0;

    SDL_Init(SDL_INIT_VIDEO);


    SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_RESIZABLE, &sdlWindow, &sdlRenderer);

    screen = SDL_CreateRGBSurface(0, 640, 480, 32,
                                            0x00FF0000,
                                            0x0000FF00,
                                            0x000000FF,
                                            0xFF000000);
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                640, 480);
}

static unsigned long colours[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000};

void drawFrame() {
    
	int y, tx, ty;
	unsigned int *b = (unsigned int *)screen->pixels;

	for(ty = 0; ty < 24; ty++)
	{
	for(tx = 0; tx < 16; tx++)
	{
	for(y = 0; y<8; y++)
	{
		unsigned char b1, b2;
		int tileaddr = 0x8000 +  ty*0x100 + tx*16 + y*2;

		b1 = memory::read(tileaddr);
		b2 = memory::read(tileaddr+1);
		b[(ty*640*8)+(tx*8) + (y*640) + 0 + 0x1F4F0] = colours[(!!(b1&0x80))<<1 | !!(b2&0x80)];
		b[(ty*640*8)+(tx*8) + (y*640) + 1 + 0x1F4F0] = colours[(!!(b1&0x40))<<1 | !!(b2&0x40)];
		b[(ty*640*8)+(tx*8) + (y*640) + 2 + 0x1F4F0] = colours[(!!(b1&0x20))<<1 | !!(b2&0x20)];
		b[(ty*640*8)+(tx*8) + (y*640) + 3 + 0x1F4F0] = colours[(!!(b1&0x10))<<1 | !!(b2&0x10)];
		b[(ty*640*8)+(tx*8) + (y*640) + 4 + 0x1F4F0] = colours[(!!(b1&0x8))<<1 | !!(b2&0x8)];
		b[(ty*640*8)+(tx*8) + (y*640) + 5 + 0x1F4F0] = colours[(!!(b1&0x4))<<1 | !!(b2&0x4)];
		b[(ty*640*8)+(tx*8) + (y*640) + 6 + 0x1F4F0] = colours[(!!(b1&0x2))<<1 | !!(b2&0x2)];
		b[(ty*640*8)+(tx*8) + (y*640) + 7 + 0x1F4F0] = colours[(!!(b1&0x1))<<1 | !!(b2&0x1)];
	}
	}
	}

	SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
}

byte getCurrentLine() {
    return currentLine;
}
byte readOAM(ushort address) {


    cout << "READ DUMPING OAM RAM: " << Short(address) << " - " << Byte(memory::ram[address]) << " - " << endl;

    for (int i=0; i<160; i++) {
        if ((i % 32) == 0) cout << endl;
        cout << " " << Byte(oamRAM[i]);
    }

    cout << endl;

    sleep(2);


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

void tick() {
    int f = cpu::getTickCount() % (TICKS_PER_FRAME);
    int l = f / 114;

    if (l != currentLine && l < 144) {
        if (DEBUG) cout << "PPU:> NEW LINE: " << l << " FRAME: " << currentFrame << endl;
    }

    if (l != currentLine && l == 145) {
        currentFrame++;
        drawFrame();
        cout << endl << "PPU:> NEW FRAME: " << currentFrame << endl << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        cpu::handleInterrupt(cpu::IVBlank, true);
    }

    currentLine = l;

    SDL_Event e;
    if (SDL_PollEvent(&e) > 0)
    {
        SDL_UpdateWindowSurface(sdlWindow);

        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            exit(0);
        }
    }
}

}

