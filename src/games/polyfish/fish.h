#pragma once

#include "geometry/types.h"
#include "geometry/rotation.h"
#include "geometry/camera.h"
#include <stdint.h>

#pragma code(code62)
#pragma data(data62)
#pragma bss(bss)

typedef struct {
    struct coord base_pos;
    struct angle angle;
    uint8_t colors[3];
    // 7 fish verts then the eye
    struct coord verts[8];
} fish_t;

geof_t fish_calc_distance(fish_t *fish, camera_t *cam);
void fish_paint(fish_t *fish, camera_t *cam);

#pragma compile("fish.c")

