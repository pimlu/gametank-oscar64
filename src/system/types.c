#include "types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

uint8_t bits_update(uint8_t reg, uint8_t bit, bool val) {
    if (val) {
        return reg | (1 << bit);
    } else {
        return reg & ~(1 << bit);
    }
}

uint8_t bits_toggle(uint8_t reg, uint8_t bit) {
    return reg ^ (1 << bit);
}

bool bits_get(uint8_t reg, uint8_t bit) {
    return reg & (1 << bit);
}
