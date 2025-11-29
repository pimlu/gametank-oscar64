#include "types.h"

#pragma code(code63)
#pragma data(data63)

uint8_t bits::updateBit(uint8_t reg, uint8_t bit, bool val) {
    if (val) {
        return reg | (1 << bit);
    } else {
        return reg & ~(1 << bit);
    }
}

uint8_t bits::toggleBit(uint8_t reg, uint8_t bit) {
    return reg ^ (1 << bit);
}

bool bits::getBit(uint8_t reg, uint8_t bit) {
    return reg & (1 << bit);
}