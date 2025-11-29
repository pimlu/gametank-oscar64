#pragma once

#include <stdint.h>


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace bits {
    uint8_t updateBit(uint8_t reg, uint8_t bit, bool val);
    uint8_t toggleBit(uint8_t reg, uint8_t bit);
    bool getBit(uint8_t reg, uint8_t bit);
    uint8_t oneShiftN(uint8_t n);
}

template<int ADDR>
struct memreg {
    uint8_t mirror = 0;

    inline void write(uint8_t val) {
        // write ADDR first. this is important for the memory bank register
        // because the mirror will be affected
        *(volatile uint8_t*) ADDR = val;
        // treat just the write as volatile since I'm okay with read-side
        // optimizations
        *(volatile uint8_t*) &mirror = val;
    }

    inline uint8_t setBit(uint8_t bit, bool val) {
        uint8_t res = bits::updateBit(mirror, bit, val);
        write(res);
        return res;
    }
    inline uint8_t toggleBit(uint8_t bit) {
        uint8_t res = bits::toggleBit(mirror, bit);
        write(res);
        return res;
    }
    inline bool getBit(uint8_t bit) {
        return bits::getBit(mirror, bit);
    }
};


template<int ADDR>
struct readreg {
    uint8_t read() {
        volatile uint8_t *ptr = (volatile uint8_t*) ADDR;
        return *ptr;
    }
};



#pragma compile("types.cpp")
