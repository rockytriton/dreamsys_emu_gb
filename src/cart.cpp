#include "cart.h"

#include <fstream>
#include <cstring>

using std::memcpy;

namespace dsemu::cart {


Header g_header;

byte *g_romData = nullptr;
int g_romSize = 0;

bool load(const string &romFile) {
    std::ifstream in(romFile, std::ios::binary);

    if (!in) {
        cout << "Unable to open ROM: " << romFile << endl;
        return false;
    }

    if (g_romData != nullptr) {
        delete[] g_romData;
    }

    in.seekg(0, in.end);
    g_romSize = in.tellg();
    in.seekg(0, in.beg);

    g_romData = new byte[g_romSize];

    in.read((char *)g_romData, g_romSize);

    memcpy(&g_header, g_romData + 0x100, sizeof(Header));
    in.close();

    cout << "Loaded Rom: " << romFile << endl;
    cout << "\t    Size: " << g_romSize << endl;
    cout << "\t   Title: " << g_header.title << endl;
    cout << "\tCart Typ: " << Byte(g_header.cartType) << endl;
    cout << "\tCGB Flag: " << Byte(g_header.gbcFlag) << endl;
    cout << "\tSGB Flag: " << Byte(g_header.sgbFlag) << endl;
    cout << "\tROM Size: " << Byte(g_header.romSize) << endl;
    cout << "\tRAM Size: " << Byte(g_header.ramSize) << endl;
    cout << "\tJap Flag: " << Byte(g_header.japFlag) << endl;
    cout << "\t   Entry: " << Byte(g_header.entry[0]) << " "
                           << Byte(g_header.entry[1]) << " "
                           << Byte(g_header.entry[2]) << " "
                           << Byte(g_header.entry[3]) << endl;

    return true;

}


}