#include "ppu.h"
#include "cpu.h"
#include "memory.h"
#include "io.h"

#include <chrono>
#include <thread>
#include <unistd.h>
#include <SDL2/SDL.h>

namespace dsemu::ppu {

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
static SDL_Surface *screen;

byte lcdControl;

byte lcdStats = 0;

ScrollInfo scrollInfo;
int currentFrame = 0;
byte currentLine = 0;
bool paused = false;

struct OAMEntry {
    byte y;
    byte x;
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


    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &sdlWindow, &sdlRenderer);

    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                            0x00FF0000,
                                            0x0000FF00,
                                            0x000000FF,
                                            0xFF000000);
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                SCREEN_WIDTH, SCREEN_HEIGHT);
}

static unsigned long colors[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000};

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

	unsigned int *b = (unsigned int *)screen->pixels;
	int y, tx, ty;
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
		b[(ty*640*8)+(tx*8) + (y*640) + 0 + 0x1F400] = colors[(!!(b1&0x80))<<1 | !!(b2&0x80)];
		b[(ty*640*8)+(tx*8) + (y*640) + 1 + 0x1F400] = colors[(!!(b1&0x40))<<1 | !!(b2&0x40)];
		b[(ty*640*8)+(tx*8) + (y*640) + 2 + 0x1F400] = colors[(!!(b1&0x20))<<1 | !!(b2&0x20)];
		b[(ty*640*8)+(tx*8) + (y*640) + 3 + 0x1F400] = colors[(!!(b1&0x10))<<1 | !!(b2&0x10)];
		b[(ty*640*8)+(tx*8) + (y*640) + 4 + 0x1F400] = colors[(!!(b1&0x8))<<1 | !!(b2&0x8)];
		b[(ty*640*8)+(tx*8) + (y*640) + 5 + 0x1F400] = colors[(!!(b1&0x4))<<1 | !!(b2&0x4)];
		b[(ty*640*8)+(tx*8) + (y*640) + 6 + 0x1F400] = colors[(!!(b1&0x2))<<1 | !!(b2&0x2)];
		b[(ty*640*8)+(tx*8) + (y*640) + 7 + 0x1F400] = colors[(!!(b1&0x1))<<1 | !!(b2&0x1)];
	}
	}
	}


    int scale = 5;
    int xDraw = 0;
    int yDraw = 0;
    SDL_Rect rc;

    cout << endl << "MAP:" << endl;

    for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
            ushort tileNum = memory::read(bgMapStart() + x + (y * 32));
            cout << Byte(tileNum) << "-";

            for (int tileY=0; tileY<16; tileY += 2) {
                
                byte b1 = memory::read(bgTileStart() + (tileNum * 16) + tileY);
                byte b2 = memory::read(bgTileStart() + (tileNum * 16) + 1 + tileY);
                
                for (int bit=7, n=0; bit >= 0; bit--, n++) {
                    byte hi = !!(b1 & (1 << bit)) << 1;
                    byte lo = !!(b2 & (1 << bit));

                    byte color = hi|lo;

                    rc.x = xDraw + (x * scale) + (n * scale);
                    rc.y = yDraw + (y * scale) + (tileY / 2 * scale);
                    rc.w = scale;
                    rc.h = scale;

                    SDL_FillRect(screen, &rc, colors[color]);
                }

                //xDraw = 0;
                //xDraw += (8 * scale);
            }

            xDraw += (7 * scale);
        }
        cout << endl;

        yDraw += (7 * scale);
        xDraw = 0;
    }
    cout << endl;

    OAMEntry *entry = (OAMEntry *)&oamRAM;

    yDraw = 0;
    xDraw = 0;

    if (entry->tile != 0) {
        for (int tileY=0; tileY<16; tileY += 2) {
            byte b1 = memory::read(bgTileStart() + (entry->tile * 16) + tileY);
            byte b2 = memory::read(bgTileStart() + (entry->tile * 16) + 1 + tileY);
            
            for (int bit=7, n=0; bit >= 0; bit--, n++) {
                byte hi = !!(b1 & (1 << bit)) << 1;
                byte lo = !!(b2 & (1 << bit));

                byte color = hi|lo;

                rc.x = xDraw + (entry->x * scale) + (n * scale);
                rc.y = yDraw + (entry->y * scale) + (tileY / 2 * scale);
                rc.w = scale;
                rc.h = scale;

                SDL_FillRect(screen, &rc, colors[color]);
            }

            //xDraw = 0;
            //xDraw += (8 * scale);
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

    if (lcdStats & 0x40 && memory::read(0xFF45) == l) {
        cpu::handleInterrupt(2, true, false);
    }
    
    if (l != currentLine && l == 144) {
        currentFrame++;
        drawFrame();
        cout << endl << "PPU:> NEW FRAME: " << currentFrame << endl << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        cpu::handleInterrupt(cpu::IVBlank, true, false);
    }

    currentLine = l;

    //if (!DEBUG) return;

    SDL_Event e;
    if (SDL_PollEvent(&e) > 0)
    {
        SDL_UpdateWindowSurface(sdlWindow);

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_p) {
            paused = !paused;
            cout << "Paused: " << paused << endl;
        }

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

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_CAPSLOCK) {
            cout << "KEYDOWN" << endl;
            io::selectDown = true;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_CAPSLOCK) {
            cout << "KEYUP" << endl;
            io::selectDown = false;
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

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT) {
            cout << "KEYDOWN Left" << endl;
            io::leftDown = true;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LEFT) {
            cout << "KEYUP Left" << endl;
            io::leftDown = false;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT) {
            cout << "KEYDOWN rightDown" << endl;
            io::rightDown = true;
            //sleepMs(1000);
        }

        if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_RIGHT) {
            cout << "KEYUP rightDown" << endl;
            io::rightDown = false;
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