#include "triangle.h"

#include "bresenham.h"
#include "screen.h"
#include "system/bcr.h"
#include "system/i8helpers.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct fill_triangle_context {
    bool should_wait;
};

static void fill_triangle_callback(int8_t y, int8_t x_left, int8_t x_right, struct fill_triangle_context *ctx) {
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

    // switch from scene coordinates (origin at center of screen, -y) to
    // frame coordinates (what the blitter uses, origin at top left) 
    x_left += GRAPHICS_SCENE_X_TO_FRAME;
    y += GRAPHICS_SCENE_Y_TO_FRAME;

    // it runs bresenham while the blitter is busy, so we wait before
    // calling the blitter, not after
    if (ctx->should_wait) {
        bcr_row_fill_wait();
    } else {
        ctx->should_wait = true;
    }

    bcr_trigger_row_fill(x_left, y, width);
}

#define TRIANGLE_FUNC_NAME graphics_fill_triangle_impl
#define TRIANGLE_FILL_CALL fill_triangle_callback
#define TRIANGLE_FILL_CTX struct fill_triangle_context
#define TRIANGLE_BRES_STRUCT struct bresenham
#define TRIANGLE_BRES_INIT_CALL bresenham_init
#define TRIANGLE_BRES_ITER_CALL bresenham_iter
#include "triangle_impl.c"

void graphics_fill_triangle(struct graphics_screen_pos a, struct graphics_screen_pos b, struct graphics_screen_pos c, uint8_t color) {
    struct fill_triangle_context ctx = {.should_wait = false};

    bcr_setup_row_fill(color);
    graphics_fill_triangle_impl(a, b, c, &ctx);
    if (ctx.should_wait) {
        bcr_row_fill_wait();
    }
}
