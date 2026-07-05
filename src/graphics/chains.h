#pragma once

#include "types.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Fill a convex quadrilateral in a single pass. The four corners must be given
// in convex boundary order (either winding). Produces pixels identical to
// filling the two triangles (a,b,c) and (a,c,d), but pays the per-row bresenham
// cost only once instead of twice.
void graphics_fill_convex_quad(struct graphics_screen_pos a, struct graphics_screen_pos b,
                               struct graphics_screen_pos c, struct graphics_screen_pos d,
                               uint8_t color);

#pragma compile("chains.c")
