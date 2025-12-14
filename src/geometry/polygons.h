#pragma once

#include <stdint.h>
#include "types.h"
#include "triangle.h"
#include "camera.h"
#include "graphics/types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct graphics_screen_pos geometry_to_screen(struct coord c);
void geometry_fill_triangle(const struct camera *cam, struct triangle *t, uint8_t color);

#pragma compile("polygons.c")
