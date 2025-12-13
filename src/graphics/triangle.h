#pragma once

#include "types.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void graphics_fill_triangle(struct graphics_screen_pos a, struct graphics_screen_pos b, struct graphics_screen_pos c, uint8_t color);

#pragma compile("triangle.c")
