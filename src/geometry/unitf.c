#include "unitf.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Constructors
unitf_t unitf_from_raw(uint16_t data) {
    unitf_t result = {data};
    return result;
}

unitf_t unitf_from_raw_bytes(uint8_t lsb, uint8_t msb) {
    uint16_t data = (msb << 8) | lsb;
    unitf_t result = {data};
    return result;
}

unitf_t unitf_zero(void) {
    unitf_t result = {0};
    return result;
}

// Accessors
uint16_t unitf_get_raw(unitf_t val) {
    return val.data;
}

uint8_t unitf_lsb(unitf_t val) {
    return val.data & 0xff;
}

uint8_t unitf_msb(unitf_t val) {
    return (val.data >> 8) & 0xff;
}

// Comparison operators
bool unitf_eq(unitf_t a, unitf_t b) {
    return a.data == b.data;
}

bool unitf_lt(unitf_t a, unitf_t b) {
    return a.data < b.data;
}

bool unitf_gt(unitf_t a, unitf_t b) {
    return a.data > b.data;
}

bool unitf_le(unitf_t a, unitf_t b) {
    return !unitf_gt(a, b);
}

bool unitf_ge(unitf_t a, unitf_t b) {
    return !unitf_lt(a, b);
}

bool unitf_ne(unitf_t a, unitf_t b) {
    return !unitf_eq(a, b);
}

// Arithmetic operations
unitf_t unitf_add(unitf_t a, unitf_t b) {
    unitf_t result = {a.data + b.data};
    return result;
}

unitf_t unitf_neg(unitf_t val) {
    unitf_t result = {(uint16_t)-val.data};
    return result;
}

void unitf_negate(unitf_t *val) {
    val->data = (uint16_t)-val->data;
}

unitf_t unitf_sub(unitf_t a, unitf_t b) {
    unitf_t result = {a.data - b.data};
    return result;
}

unitf_t unitf_small_int_mult(unitf_t val, uint16_t r) {
    unitf_t result = {val.data * r};
    return result;
}

unitf_t unitf_mul(unitf_t a, unitf_t b) {
    uint32_t res = mul16_to_32(a.data, b.data);
    unitf_t result = {(uint16_t)(res >> 16)};
    return result;
}

unitf_t unitf_scale_by_uint8(unitf_t val, uint8_t scale) {
    uint32_t res = mul16_to_32(val.data, scale);
    unitf_t result = {(uint16_t)(res >> 8)};
    return result;
}

