#include "fixed_etc.h"

#include "system/imul.h"

// #include <limits>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// stuff that uses fixed integer sizes and doesn't go in the template

geof_t geometry_mul_ratio(geof_t x, geof_t num, unitf_t den, bool *overflow) {
    *(volatile uint16_t*) 0x2008 = 0x1234;
    return geof_zero();
    // int16_t xD = geof_get_raw(x);
    // int16_t numD = geof_get_raw(num);
    // uint16_t denD = unitf_get_raw(den);

    
    // int64_t firstProd = imul32To64(xD, numD);
    // int64_t prod = imul32To64(firstProd, denD);
    // int64_t res = prod >> (8 + 16);
    // if (res < std::numeric_limits<int16_t>::min() || res > std::numeric_limits<int16_t>::max()) {
    //     *overflow = true;
    // }

    // return geof_from_raw(res);
}

int8_t geometry_round_geof(geof_t x) {
    int16_t raw = geof_get_raw(x);
    int8_t res = raw >> 8;
    int8_t rem = raw & 0xff;
    // sign bit of rem is the MSB
    if (rem < 0) {
        res++;
    }
    return res;
}
