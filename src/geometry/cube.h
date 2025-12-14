#pragma once

#include "types.h"
#include "triangle.h"
#include "camera.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct cube {
    struct coord lo, hi;
    struct coord verts[8];
    // x-lo, x-hi, y-lo, y-hi, z-lo, z-hi
    uint8_t colors[6];
};

geof_t cube_calc_distance(struct cube *cube, struct camera *cam);
void cube_paint(struct cube *cube, struct camera *cam);
struct coord cube_debug_get_vert(const struct cube *cube, uint8_t i);

#pragma compile("cube.c")
