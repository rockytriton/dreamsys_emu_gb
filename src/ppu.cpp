#include "ppu.h"
#include "cpu.h"
#include "memory.h"
#include "io.h"

#include <chrono>
#include <thread>
#include <unistd.h>
#include <SDL2/SDL.h>

namespace dsemu::ppu {

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
static SDL_Surface *screen;

byte lcdControl;

ScrollInfo scrollInfo;
int currentFrame = 0;
byte currentLine = 0;

struct OAMEntry {
    byte x;
    byte y;
    byte tile;
    byte flags;
};

inline bool bgDisplay() { return getBit(lcdControl, 0); }
inline bool spriteDisplay() { return getBit(lcdControl, 1); }
inline bool spriteSize8x16() { return getBit(lcdControl, 2); }
inline ushort bgMapStart() { return getBit(lcdControl, 3) ? 0x9C00 : 0x9800; }
inline ushort bgTileStart() { return getBit(lcdControl, 4) ? 0x8000 : 0x8800; }
inline bool windowDisplay() { return getBit(lcdControl, 5); }
inline ushort windowMapSelect() { return getBit(lcdControl, 6) ? 0x9C00 : 0x9800; }
inline bool lcdOn() { return getBit(lcdControl, 7); }

/*
OAM Entry:
    PosX
    PosY
    Tile Num
    Priority bit
    FlipX bit
    FlipY bit
    Palette bits
*/

byte oamRAM[160];

void init() {
    currentFrame = 0;
    currentLine = 0;
    memset(oamRAM, 0, sizeof(oamRAM));

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

static unsigned long colours[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x444444};

void drawFrame() {
    
	int y, tx, ty;
	unsigned int *b = (unsigned int *)screen->pixels;


    //OAMEntry *entry = (OAMEntry *)&oamRAM;

    if (DEBUG) cout  << endl << "LCD Status: " << endl
         << "\tbgDisplay: " << Byte(bgDisplay()) << endl
         << "\tspriteDisplay: " << Byte(spriteDisplay()) << endl
         << "\tspriteSize8x16: " << Byte(spriteSize8x16()) << endl
         << "\tbgMapStart: " << Short(bgMapStart()) << endl
         << "\tbgTileStart: " << Short(bgTileStart()) << endl
         << "\twindowDisplay: " << Byte(windowDisplay()) << endl
         << "\twindowMapSelect: " << Short(windowMapSelect()) << endl
         << "\tlcdOn: " << Byte(lcdOn()) << endl << endl;

    //if (!DEBUG) return;


	SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
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

void tick() {
    int f = cpu::getTickCount() % (TICKS_PER_FRAME);
    int l = f / 114;

    if (l != currentLine && l < 144) {
        if (DEBUG) cout << "PPU:> NEW LINE: " << l << " FRAME: " << currentFrame << endl;
    }

    if (l != currentLine && l == 144) {
        currentFrame++;
        drawFrame();
        cout << endl << "PPU:> NEW FRAME: " << currentFrame << endl << endl;

        //std::this_thread::sleep_for(std::chrono::milliseconds(10));

        cpu::handleInterrupt(cpu::IVBlank, true, false);
    }

    currentLine = l;

    //if (!DEBUG) return;

    SDL_Event e;
    if (SDL_PollEvent(&e) > 0)
    {
        SDL_UpdateWindowSurface(sdlWindow);

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
            cout << "KEYDOWN" << endl;
            io::startDown = true;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_RETURN) {
            cout << "KEYUP" << endl;
            io::startDown = false;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a) {
            cout << "KEYDOWN a" << endl;
            io::aDown = true;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a) {
            cout << "KEYUP a" << endl;
            io::aDown = false;
            //sleepMs(1000);
        }

        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            exit(0);
        }
    }
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