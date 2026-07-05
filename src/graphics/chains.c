#include "chains.h"

#include "bresenham.h"
#include "convex.h"
#include "screen.h"
#include "system/bcr.h"
#include "system/i8helpers.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Per-row fill: identical to triangle.c's fill_triangle_callback. Vertices are
// in scene coordinates (origin at screen center, y up); convert to frame
// coordinates and hand a solid row to the blitter.
static void fill_row_callback(int8_t y, int8_t x_left, int8_t x_right) {
    // change y to point up
    y = -y;
    // check for clipping and remove
    if (y < GRAPHICS_SCENE_Y_LO || y >= GRAPHICS_SCENE_Y_HI) {
        return;
    }
    x_left = max_i8(x_left, GRAPHICS_SCENE_X_LO);
    x_right = min_i8(x_right, GRAPHICS_SCENE_X_HI);
    if (x_left >= x_right) {
        return;
    }
    int8_t width = x_right - x_left;

    x_left += GRAPHICS_SCENE_X_TO_FRAME;
    y += GRAPHICS_SCENE_Y_TO_FRAME;

    bcr_trigger_row_fill(x_left, y, width);
}

// Generate the chain filler bound to the blitter callback.
#define CHAINS_FUNC_NAME graphics_fill_chains_rows
#define CHAINS_FILL_CALL fill_row_callback
#define CHAINS_BRES_STRUCT struct bresenham
#define CHAINS_BRES_INIT_CALL bresenham_init
#define CHAINS_BRES_ITER_CALL bresenham_iter
#include "chains_impl.c"

void graphics_fill_convex_quad(struct graphics_screen_pos a, struct graphics_screen_pos b,
                               struct graphics_screen_pos c, struct graphics_screen_pos d,
                               uint8_t color) {
    struct graphics_screen_pos poly[4] = {a, b, c, d};
    struct graphics_screen_pos left[5], right[5];
    uint8_t nl, nr;
    graphics_convex_to_chains(poly, 4, left, &nl, right, &nr);

    bcr_setup_row_fill(color);
    graphics_fill_chains_rows(left, nl, right, nr);
}
