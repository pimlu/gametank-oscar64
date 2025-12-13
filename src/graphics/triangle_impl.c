// Template implementation for triangle filling
// Include this file with appropriate defines to generate specialized versions
//
// Required defines:
//   TRIANGLE_FUNC_NAME - function name (e.g., graphics_fill_triangle_prod, graphics_fill_triangle_ref)
//   TRIANGLE_FILL_CALL - fill callback function name (e.g., fill_triangle_callback)
//   TRIANGLE_FILL_CTX - fill context type (e.g., struct fill_triangle_context)
//   TRIANGLE_BRES_STRUCT - bresenham struct type (e.g., struct bresenham)
//   TRIANGLE_BRES_INIT_CALL - bresenham init function (e.g., bresenham_init)
//   TRIANGLE_BRES_ITER_CALL - bresenham iter function (e.g., bresenham_iter)
//
// This file should be included after the necessary headers and function declarations

#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "system/imul.h"

void TRIANGLE_FUNC_NAME(struct graphics_screen_pos a, struct graphics_screen_pos b, struct graphics_screen_pos c, TRIANGLE_FILL_CTX *fill_context) {
    // sort them so a.y <= b.y <= c.y
    if (a.y > b.y) graphics_swap_pos(&a, &b);
    if (b.y > c.y) graphics_swap_pos(&b, &c);
    if (a.y > b.y) graphics_swap_pos(&a, &b);

    bool b_is_left;
    if (b.y != a.y) {
        // annoying slope comparison to check if the left side is AB or AC
        // this is the thing where a/b < c/d turns into a*d < c*b
        b_is_left = imul8_to_16(b.x - a.x, c.y - a.y) < imul8_to_16(c.x - a.x, b.y - a.y);
    } else {
        // annoying edge case: if there is no flat top triangle,
        // left side vs right side is determined by whether B is to the left
        // or right of A.
        b_is_left = b.x < a.x;
    }

    struct graphics_screen_pos left = c;
    struct graphics_screen_pos right = b;
    if (b_is_left) {
        left = b;
        right = c;
    }

    // if a.y == b.y, need to skip [a, b]

    // first, fill the "flat top triangle"
    TRIANGLE_BRES_STRUCT left_bres;
    TRIANGLE_BRES_STRUCT right_bres;
    TRIANGLE_BRES_INIT_CALL(&left_bres, a, left);
    TRIANGLE_BRES_INIT_CALL(&right_bres, a, right);

    int8_t y = a.y;
    for (; y < b.y; y++) {
        int8_t x_left = TRIANGLE_BRES_ITER_CALL(&left_bres);
        int8_t x_right = TRIANGLE_BRES_ITER_CALL(&right_bres);
        TRIANGLE_FILL_CALL(y, x_left, x_right, fill_context);
    }

    // we've now reached b (vertically speaking) and need to replace it with c
    if (b_is_left) {
        TRIANGLE_BRES_INIT_CALL(&left_bres, b, c);
    } else {
        TRIANGLE_BRES_INIT_CALL(&right_bres, b, c);
    }

    // now, fill the "flat bottom triangle"
    for (; y < c.y; y++) {
        int8_t x_left = TRIANGLE_BRES_ITER_CALL(&left_bres);
        int8_t x_right = TRIANGLE_BRES_ITER_CALL(&right_bres);
        TRIANGLE_FILL_CALL(y, x_left, x_right, fill_context);
    }
}

#undef TRIANGLE_FUNC_NAME
#undef TRIANGLE_FILL_CALL
#undef TRIANGLE_FILL_CTX
#undef TRIANGLE_BRES_STRUCT
#undef TRIANGLE_BRES_INIT_CALL
#undef TRIANGLE_BRES_ITER_CALL
