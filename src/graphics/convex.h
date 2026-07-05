#pragma once

#include "types.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Split a convex polygon into a left and right vertex chain suitable for
// graphics_fill_chains_rows.
//
// `poly` holds `n` vertices in convex boundary order (either winding). The
// output chains run top-to-bottom (increasing y) and satisfy the invariants in
// chains_impl.c. `out_left` and `out_right` must each hold at least n+1 entries.
//
// Costs exactly two 8->16 multiplies (a single orientation test recovers the
// polygon's winding, which determines both chain directions).
void graphics_convex_to_chains(const struct graphics_screen_pos *poly, uint8_t n,
                               struct graphics_screen_pos *out_left, uint8_t *nl,
                               struct graphics_screen_pos *out_right, uint8_t *nr);

#pragma compile("convex.c")
