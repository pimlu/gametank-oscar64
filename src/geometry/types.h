#pragma once

#include "geof.h"
#include "unitf.h"
#include "graphics/types.h"
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

geof_t geometry_scale_geof(geof_t val, unitf_t scale);

// oscar64 doesn't like ternary operators on structs
geof_t geof_cond(bool cond, geof_t true_val, geof_t false_val);

// this lame thing is because unitf_t is unsigned
typedef struct {
    unitf_t val;
    bool negated;
} iunitf_t;

struct coord {
    geof_t x, y, z;
};

struct coord2d {
    geof_t x, y;
};

#pragma compile("types.c")

