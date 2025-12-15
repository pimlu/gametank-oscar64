#pragma once

#include "types.h"
#include "triangle.h"
#include "camera.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

typedef struct {
    struct coord lo, hi;
    struct coord verts[8];
    // x-lo, x-hi, y-lo, y-hi, z-lo, z-hi
    uint8_t colors[6];
} cube_t;

geof_t cube_calc_distance(cube_t *cube, camera_t *cam);
void cube_paint(cube_t *cube, camera_t *cam);
struct coord cube_debug_get_vert(const cube_t *cube, uint8_t i);

#pragma compile("cube.c")
