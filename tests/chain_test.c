#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

// Include headers (pragmas will be ignored by clang)
#include "graphics/bresenham.h"
#include "graphics/convex.h"
#include "graphics/types.h"
#include "system/i8helpers.h"
#include "system/imul.h"

// ---------------------------------------------------------------------------
// Shared test bitmap + fill callback (same idea as bresenham_test.c)
// ---------------------------------------------------------------------------
static struct {
    bool **bits;
    size_t w, h;
    bool bad_range;
} test_fill_context;

static void test_fill_callback(int8_t y, int8_t x_left, int8_t x_right) {
    if (y < 0 || (size_t)y >= test_fill_context.h || x_left < 0 || (size_t)x_right > test_fill_context.w) {
        if (!test_fill_context.bad_range) {
            test_fill_context.bad_range = true;
            printf("OOPS. bad range y=%d xLeft=%d xRight=%d\n", y, x_left, x_right);
            printf("w=%zu h=%zu\n", test_fill_context.w, test_fill_context.h);
        }
        return;
    }
    for (int x = x_left; x < x_right; x++) {
        test_fill_context.bits[y][x] = true;
    }
}

// ---------------------------------------------------------------------------
// Generate the production triangle filler (for the two-triangle reference).
// ---------------------------------------------------------------------------
#define TRIANGLE_FUNC_NAME graphics_fill_triangle_impl
#define TRIANGLE_FILL_CALL test_fill_callback
#define TRIANGLE_BRES_STRUCT struct bresenham
#define TRIANGLE_BRES_INIT_CALL bresenham_init
#define TRIANGLE_BRES_ITER_CALL bresenham_iter
#include "graphics/triangle_impl.c"

extern void graphics_fill_triangle_impl(struct graphics_screen_pos a,
                                        struct graphics_screen_pos b,
                                        struct graphics_screen_pos c);

// ---------------------------------------------------------------------------
// Generate the chain filler under test (uses the same production bresenham).
// ---------------------------------------------------------------------------
#define CHAINS_FUNC_NAME graphics_fill_chains_rows
#define CHAINS_FILL_CALL test_fill_callback
#define CHAINS_BRES_STRUCT struct bresenham
#define CHAINS_BRES_INIT_CALL bresenham_init
#define CHAINS_BRES_ITER_CALL bresenham_iter
#include "graphics/chains_impl.c"

extern void graphics_fill_chains_rows(const struct graphics_screen_pos *l, uint8_t nl,
                                      const struct graphics_screen_pos *r, uint8_t nr);

// ---------------------------------------------------------------------------
// Bitmap helpers
// ---------------------------------------------------------------------------
static bool **alloc_bits(size_t w, size_t h) {
    bool **bits = malloc(h * sizeof(bool *));
    for (size_t i = 0; i < h; i++) bits[i] = calloc(w, sizeof(bool));
    return bits;
}

static void free_bits(bool **bits, size_t h) {
    for (size_t i = 0; i < h; i++) free(bits[i]);
    free(bits);
}

static void print_bits(bool **bits, size_t w, size_t h) {
    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) putchar(bits[y][x] ? '#' : '.');
        putchar('\n');
    }
}

// ---------------------------------------------------------------------------
// Convex hull (monotone chain) for small point sets. Returns hull vertices in
// order into `out`, or 0 if the points aren't in strictly-convex position.
// ---------------------------------------------------------------------------
static int cross_o(struct graphics_screen_pos o, struct graphics_screen_pos a, struct graphics_screen_pos b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

static int cmp_pos(const void *pa, const void *pb) {
    const struct graphics_screen_pos *a = pa, *b = pb;
    if (a->x != b->x) return a->x - b->x;
    return a->y - b->y;
}

static int convex_hull(struct graphics_screen_pos *pts, int n, struct graphics_screen_pos *out) {
    qsort(pts, n, sizeof(pts[0]), cmp_pos);
    // reject duplicate points
    for (int i = 1; i < n; i++)
        if (pts[i].x == pts[i - 1].x && pts[i].y == pts[i - 1].y) return 0;

    int k = 0;
    struct graphics_screen_pos h[16];
    for (int i = 0; i < n; i++) {
        while (k >= 2 && cross_o(h[k - 2], h[k - 1], pts[i]) <= 0) k--;
        h[k++] = pts[i];
    }
    int lower = k + 1;
    for (int i = n - 2; i >= 0; i--) {
        while (k >= lower && cross_o(h[k - 2], h[k - 1], pts[i]) <= 0) k--;
        h[k++] = pts[i];
    }
    int m = k - 1;  // last point == first
    for (int i = 0; i < m; i++) out[i] = h[i];
    return m;
}

// ---------------------------------------------------------------------------
// Fill a convex polygon two ways and compare.
// ---------------------------------------------------------------------------
static bool compare_fill(const struct graphics_screen_pos *poly, int n, size_t w, size_t h) {
    // A: chain fill
    struct graphics_screen_pos left[16], right[16];
    uint8_t nl, nr;
    graphics_convex_to_chains(poly, n, left, &nl, right, &nr);

    bool **a_bits = alloc_bits(w, h);
    test_fill_context.bits = a_bits;
    test_fill_context.w = w;
    test_fill_context.h = h;
    test_fill_context.bad_range = false;
    graphics_fill_chains_rows(left, nl, right, nr);
    bool a_bad = test_fill_context.bad_range;

    // B: fan of triangles (v0,vi,vi+1) sharing vertex 0. For a convex polygon
    // these tile it exactly under the pos/neg edge convention.
    bool **b_bits = alloc_bits(w, h);
    test_fill_context.bits = b_bits;
    test_fill_context.bad_range = false;
    for (int i = 1; i + 1 < n; i++) {
        graphics_fill_triangle_impl(poly[0], poly[i], poly[i + 1]);
    }
    bool b_bad = test_fill_context.bad_range;

    bool match = !a_bad && !b_bad;
    if (match) {
        for (size_t y = 0; y < h && match; y++)
            for (size_t x = 0; x < w && match; x++)
                if (a_bits[y][x] != b_bits[y][x]) match = false;
    }

    if (!match) {
        printf("MISMATCH poly:");
        for (int i = 0; i < n; i++) printf(" {%d,%d}", poly[i].x, poly[i].y);
        printf("\n  left:");
        for (int i = 0; i < nl; i++) printf(" {%d,%d}", left[i].x, left[i].y);
        printf("\n  right:");
        for (int i = 0; i < nr; i++) printf(" {%d,%d}", right[i].x, right[i].y);
        printf("\n--- chain fill%s ---\n", a_bad ? " (BAD RANGE)" : "");
        print_bits(a_bits, w, h);
        printf("--- triangle fill%s ---\n", b_bad ? " (BAD RANGE)" : "");
        print_bits(b_bits, w, h);
    }

    free_bits(a_bits, h);
    free_bits(b_bits, h);
    return match;
}

// ---------------------------------------------------------------------------
static int8_t rng_int8(int8_t min, int8_t max) {
    int range = max - min + 1;
    return (int8_t)(min + (rand() % range));
}

int main(void) {
    const size_t W = 20, H = 20;
    const int8_t LO = 1, HI = 18;

    // A couple of fixed sanity cases first.
    {
        // a triangle expressed as a degenerate "quad" (3 points)
        struct graphics_screen_pos tri[3] = {{4, 2}, {16, 6}, {6, 17}};
        if (!compare_fill(tri, 3, W, H)) return 1;
    }
    {
        // an axis-aligned rectangle (flat top and flat bottom)
        struct graphics_screen_pos rect[4] = {{3, 3}, {15, 3}, {15, 14}, {3, 14}};
        if (!compare_fill(rect, 4, W, H)) return 1;
    }

    srand(time(NULL));

    const size_t ITERS = 2000000;
    printf("fuzzing convex quads: chain fill vs two-triangle fill (%zu iters)\n", ITERS);
    size_t tested = 0, rejected = 0;
    for (size_t it = 0; it < ITERS; it++) {
        struct graphics_screen_pos pts[4];
        for (int i = 0; i < 4; i++) {
            pts[i].x = rng_int8(LO, HI);
            pts[i].y = rng_int8(LO, HI);
        }
        struct graphics_screen_pos hull[4];
        int m = convex_hull(pts, 4, hull);
        if (m != 4) { rejected++; continue; }  // not a strictly-convex quad

        if (!compare_fill(hull, m, W, H)) {
            printf("failed after %zu tested\n", tested);
            return 1;
        }
        tested++;
    }

    printf("passed: %zu convex quads tested, %zu rejected (degenerate)\n", tested, rejected);
    printf("passed all tests\n");
    return 0;
}
