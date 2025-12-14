#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "double.h"
#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

typedef struct {
    int16_t data;
} geof_t;

// Constructors
geof_t geof_from_raw(int16_t data);
geof_t geof_from_raw_bytes(uint8_t lsb, uint8_t msb);

// Accessors
int16_t geof_get_raw(geof_t val);
uint8_t geof_lsb(geof_t val);
uint8_t geof_msb(geof_t val);

// Comparison operators
bool geof_eq(geof_t a, geof_t b);
bool geof_lt(geof_t a, geof_t b);
bool geof_gt(geof_t a, geof_t b);
bool geof_le(geof_t a, geof_t b);
bool geof_ge(geof_t a, geof_t b);
bool geof_ne(geof_t a, geof_t b);

// Arithmetic operations
geof_t geof_add(geof_t a, geof_t b);
geof_t geof_neg(geof_t val);
void geof_negate(geof_t *val);
geof_t geof_sub(geof_t a, geof_t b);
geof_t geof_small_int_mult(geof_t val, uint16_t r);
geof_t geof_mul(geof_t a, geof_t b);

#pragma compile("geof.c")
