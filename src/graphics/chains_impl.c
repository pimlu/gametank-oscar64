// Template implementation for filling "y-function chains".
// Include this file with appropriate defines to generate specialized versions,
// exactly like triangle_impl.c.
//
// Required defines:
//   CHAINS_FUNC_NAME       - function name (e.g., graphics_fill_chains_rows)
//   CHAINS_FILL_CALL       - fill callback (y, x_left, x_right)
//   CHAINS_BRES_STRUCT     - bresenham struct type (e.g., struct bresenham)
//   CHAINS_BRES_INIT_CALL  - bresenham init function
//   CHAINS_BRES_ITER_CALL  - bresenham iter function
//
// Fills the region between a left vertex chain `l` and a right vertex chain `r`,
// one row at a time. This is the direct generalization of triangle_impl.c: a
// triangle is just the case where one side has a single mid vertex (the classic
// "swap in edge b->c at vertex b").
//
// Invariants (see design notes):
//   l[0].y == r[0].y                 both chains start at the same top row
//   l[0].x <= r[0].x
//   l[i].y <= l[i+1].y               each chain is non-decreasing in y
//   r[i].y <= r[i+1].y
//   l[nl-1].y == r[nr-1].y           both chains end at the same bottom row
//   l[nl-1].x <= r[nr-1].x
// At any row, the left x must be <= the right x.
//
// A segment with l[i].y == l[i+1].y is a horizontal edge: the side's x jumps
// discontinuously at that row (useful for shapes like an L). Such segments are
// skipped by the `while` loops below and produce no rows.
//
// Precondition: the FIRST segment of each chain descends (l[0].y < l[1].y and
// r[0].y < r[1].y). A flat top is expressed by l[0]/r[0] sharing a y with
// l[0].x <= r[0].x, not by a horizontal first segment. Interior and bottom
// horizontal edges are fine.

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

#ifdef __OSCAR64C__
static __zeropage CHAINS_BRES_STRUCT chains_left_bres, chains_right_bres;
#endif

void CHAINS_FUNC_NAME(const struct graphics_screen_pos *l, uint8_t nl,
                      const struct graphics_screen_pos *r, uint8_t nr) {
#ifndef __OSCAR64C__
    CHAINS_BRES_STRUCT chains_left_bres, chains_right_bres;
#endif

    (void)nr;  // nr only asserts r[nr-1].y == l[nl-1].y (the shared bottom row)

    uint8_t li = 0;  // left side: current segment is l[li] -> l[li+1]
    uint8_t ri = 0;  // right side: current segment is r[ri] -> r[ri+1]

    CHAINS_BRES_INIT_CALL(&chains_left_bres, l[0], l[1]);
    CHAINS_BRES_INIT_CALL(&chains_right_bres, r[0], r[1]);

    int8_t y_bot = l[nl - 1].y;  // == r[nr - 1].y
    for (int8_t y = l[0].y; y < y_bot; y++) {
        // Advance each side to the segment that actually contains this row,
        // skipping any zero-height (horizontal) segments along the way.
        while (y >= l[li + 1].y) {
            li++;
            CHAINS_BRES_INIT_CALL(&chains_left_bres, l[li], l[li + 1]);
        }
        while (y >= r[ri + 1].y) {
            ri++;
            CHAINS_BRES_INIT_CALL(&chains_right_bres, r[ri], r[ri + 1]);
        }
        int8_t x_left = CHAINS_BRES_ITER_CALL(&chains_left_bres);
        int8_t x_right = CHAINS_BRES_ITER_CALL(&chains_right_bres);
        CHAINS_FILL_CALL(y, x_left, x_right);
    }
}

#undef CHAINS_FUNC_NAME
#undef CHAINS_FILL_CALL
#undef CHAINS_BRES_STRUCT
#undef CHAINS_BRES_INIT_CALL
#undef CHAINS_BRES_ITER_CALL
