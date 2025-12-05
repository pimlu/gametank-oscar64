#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "geof.h"
#include "unitf.h"
#include "fixed_etc.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct lut_config {
    geof_t start;
    geof_t step;
    unitf_t last;
    unitf_t lo_miss;
    unitf_t hi_miss;
};

struct lut {
    uint8_t lsb[256];
    uint8_t msb[256];
};

// Lookup functions
unitf_t lut_lookup(const struct lut *lut, uint8_t i);
unitf_t lut_lerp(const struct lut *lut, const struct lut_config *config, unitf_t a, unitf_t b, uint8_t frac);
unitf_t lut_lookup_or_const(const struct lut *lut, const struct lut_config *config, geof_t x);

#pragma compile("lut.c")