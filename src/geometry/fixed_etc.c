#include "fixed_etc.h"

#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// stuff that uses fixed integer sizes and doesn't go in the template

geof_t geometry_mul_ratio(geof_t x, geof_t num, unitf_t den, bool *overflow) {
    int16_t x_raw = geof_get_raw(x);
    int16_t num_raw = geof_get_raw(num);
    uint16_t den_raw = unitf_get_raw(den);

    
    int32_t first_prod = imul16_to_32(x_raw, num_raw);
    int16_t res = mul_shr24_sat0(first_prod, den_raw, overflow);

    return geof_from_raw(res);
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
