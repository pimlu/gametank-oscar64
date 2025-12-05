#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "double.h"
#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Macro to convert double to unitf raw value
#define UNITF_FROM_DOUBLE(value) ((uint16_t)((value) * 65536.0 + 0.5))

typedef struct {
    uint16_t data;
} unitf_t;

// Constructors
unitf_t unitf_from_raw(uint16_t data);
unitf_t unitf_from_raw_bytes(uint8_t lsb, uint8_t msb);
unitf_t unitf_zero(void);

// Accessors
uint16_t unitf_get_raw(unitf_t val);
uint8_t unitf_lsb(unitf_t val);
uint8_t unitf_msb(unitf_t val);

// Comparison operators
bool unitf_eq(unitf_t a, unitf_t b);
bool unitf_lt(unitf_t a, unitf_t b);
bool unitf_gt(unitf_t a, unitf_t b);
bool unitf_le(unitf_t a, unitf_t b);
bool unitf_ge(unitf_t a, unitf_t b);
bool unitf_ne(unitf_t a, unitf_t b);

// Arithmetic operations
unitf_t unitf_add(unitf_t a, unitf_t b);
unitf_t unitf_neg(unitf_t val);
void unitf_negate(unitf_t *val);
unitf_t unitf_sub(unitf_t a, unitf_t b);
unitf_t unitf_small_int_mult(unitf_t val, uint16_t r);
unitf_t unitf_mul(unitf_t a, unitf_t b);
unitf_t unitf_scale_by_uint8(unitf_t val, uint8_t scale);

#pragma compile("unitf.c")
