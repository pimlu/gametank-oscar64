#pragma once

#include "geometry/types.h"
#include "geometry/rotation.h"
#include "geometry/camera.h"
#include <stdint.h>

#pragma code(code62)
#pragma data(data62)
#pragma bss(bss)

struct fish {
    struct coord base_pos;
    struct angle angle;
    uint8_t colors[3];
    // 7 fish verts then the eye
    struct coord verts[8];
};

geof_t fish_calc_distance(struct fish *fish, struct camera *cam);
void fish_paint(struct fish *fish, struct camera *cam);

#pragma compile("fish.c")

