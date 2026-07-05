#include "convex.h"
#include "system/imul.h"
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Which boundary direction (+1 forward / -1 backward) from vertex `i` follows
// the LEFT boundary edge, in screen coords (x right, y down)? This is the sign
// of the 2D cross product of the two edges meeting at the vertex: z<0 means the
// forward edge is the left one.
//
// For a convex polygon this sign is the same at every vertex (all turns go the
// same way), so a single call recovers the polygon's winding and thus both
// chain directions. This is the only place we spend multiplies: two of them.
static int8_t poly_left_dir(const struct graphics_screen_pos *poly, uint8_t n, uint8_t i) {
    uint8_t fi = (i + 1 == n) ? 0 : i + 1;
    uint8_t bi = (i == 0) ? n - 1 : i - 1;
    struct graphics_screen_pos p = poly[i], f = poly[fi], b = poly[bi];
    int8_t fx = f.x - p.x, fy = f.y - p.y;
    int8_t bx = b.x - p.x, by = b.y - p.y;
    int16_t z = imul8_to_16(fx, by) - imul8_to_16(fy, bx);
    return z < 0 ? 1 : -1;
}

// Walk the boundary from `start` in direction `dir`, collecting vertices until
// (and including) the first vertex at y == ymax.
static uint8_t build_chain(const struct graphics_screen_pos *poly, uint8_t n,
                           uint8_t start, int8_t dir, int8_t ymax,
                           struct graphics_screen_pos *out) {
    uint8_t count = 0;
    uint8_t i = start;
    out[count++] = poly[i];
    while (poly[i].y != ymax) {
        if (dir > 0) {
            i++;
            if (i == n) i = 0;
        } else {
            if (i == 0) i = n;
            i--;
        }
        out[count++] = poly[i];
    }
    return count;
}

void graphics_convex_to_chains(const struct graphics_screen_pos *poly, uint8_t n,
                               struct graphics_screen_pos *out_left, uint8_t *nl,
                               struct graphics_screen_pos *out_right, uint8_t *nr) {
    int8_t ymin = poly[0].y, ymax = poly[0].y;
    for (uint8_t i = 1; i < n; i++) {
        if (poly[i].y < ymin) ymin = poly[i].y;
        if (poly[i].y > ymax) ymax = poly[i].y;
    }

    // top-left  = min y, tie-break min x  -> start of left chain
    // top-right = min y, tie-break max x  -> start of right chain
    // (equal when the top is a single apex; distinct for a flat top).
    uint8_t tl = 0, tr = 0;
    bool found = false;
    for (uint8_t i = 0; i < n; i++) {
        if (poly[i].y != ymin) continue;
        if (!found) {
            tl = tr = i;
            found = true;
        } else {
            if (poly[i].x < poly[tl].x) tl = i;
            if (poly[i].x > poly[tr].x) tr = i;
        }
    }

    // One orientation test recovers the winding (and thus both directions):
    // the left chain descends from the top-left vertex, the right chain
    // descends the opposite way from the top-right vertex.
    int8_t dir = poly_left_dir(poly, n, tl);
    *nl = build_chain(poly, n, tl, dir, ymax, out_left);
    *nr = build_chain(poly, n, tr, (int8_t)-dir, ymax, out_right);
}
