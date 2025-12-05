#include "i8helpers.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

int8_t min_i8(int8_t a, int8_t b) {
    return a <= b ? a : b;
}

int8_t max_i8(int8_t a, int8_t b) {
    return a <= b ? b : a;
}

uint8_t abs_i8(int8_t a) {
    return a < 0 ? -a : a;
}

uint8_t abs_diff_i8(int8_t a, int8_t b) {
    if (a >= b) {
        return a - b;
    } else {
        return b - a;
    }
}

