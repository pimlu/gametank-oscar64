#pragma once

#include "geof.h"
#include "unitf.h"
#include "graphics/types.h"
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

geof_t geometry_scale_geof(geof_t val, unitf_t scale);

// this lame thing is because UnitF is unsigned
typedef struct {
    unitf_t val;
    bool negated;
} iunitf_t;

typedef struct {
    geof_t x, y, z;
} coord_t;

typedef struct {
    geof_t x, y;
} coord2d_t;

#pragma compile("types.c")

