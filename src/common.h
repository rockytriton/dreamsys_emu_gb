#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using std::string;
using std::vector;
using std::cout;
using std::endl;

typedef uint8_t byte; 
typedef uint16_t ushort;

ushort toShort(byte a, byte b);

struct Byte {
    Byte(byte _b) : b(_b) {}
    byte b;
};

struct Short {
    Short(ushort _s) : s(_s) {}
    ushort s;
};

inline std::ostream &operator <<(std::ostream &os, Byte b) {
    os << std::hex << std::setfill('0') << std::setw(2) << (int)b.b << std::dec;
    return os;
}

inline std::ostream &operator <<(std::ostream &os, Short s) {
    os << std::hex << std::setfill('0') << std::setw(4) << s.s << std::dec;
    return os;
}

/*
ostream& operator << (std::ostream& os, complex<double> a) {
        os << "(" << real(a) << "," << imag(a) << ")";
        return os;
}
*/