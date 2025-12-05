#include "geof.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Constructors
geof_t geof_from_raw(int16_t data) {
    geof_t result = {data};
    return result;
}

geof_t geof_from_raw_bytes(uint8_t lsb, uint8_t msb) {
    int16_t data = (msb << 8) | lsb;
    geof_t result = {data};
    return result;
}

geof_t geof_zero(void) {
    geof_t result = {0};
    return result;
}

// Accessors
int16_t geof_get_raw(geof_t val) {
    return val.data;
}

uint8_t geof_lsb(geof_t val) {
    return val.data & 0xff;
}

uint8_t geof_msb(geof_t val) {
    return (val.data >> 8) & 0xff;
}

// Comparison operators
bool geof_eq(geof_t a, geof_t b) {
    return a.data == b.data;
}

bool geof_lt(geof_t a, geof_t b) {
    return a.data < b.data;
}

bool geof_gt(geof_t a, geof_t b) {
    return a.data > b.data;
}

bool geof_le(geof_t a, geof_t b) {
    return !geof_gt(a, b);
}

bool geof_ge(geof_t a, geof_t b) {
    return !geof_lt(a, b);
}

bool geof_ne(geof_t a, geof_t b) {
    return !geof_eq(a, b);
}

// Arithmetic operations
geof_t geof_add(geof_t a, geof_t b) {
    geof_t result = {a.data + b.data};
    return result;
}

geof_t geof_neg(geof_t val) {
    geof_t result = {-val.data};
    return result;
}

void geof_negate(geof_t *val) {
    val->data = -val->data;
}

geof_t geof_sub(geof_t a, geof_t b) {
    geof_t result = {a.data - b.data};
    return result;
}

geof_t geof_small_int_mult(geof_t val, uint16_t r) {
    geof_t result = {val.data * r};
    return result;
}

geof_t geof_mul(geof_t a, geof_t b) {
    int32_t res = imul16_to_32(a.data, b.data);
    geof_t result = {(int16_t)(res >> 8)};
    return result;
}

