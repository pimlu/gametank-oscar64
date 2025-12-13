#include "bresenham.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void graphics_swap_pos(struct graphics_screen_pos *a, struct graphics_screen_pos *b) {
    int8_t tmp;

    tmp = a->x;
    a->x = b->x;
    b->x = tmp;

    tmp = a->y;
    a->y = b->y;
    b->y = tmp;
}

static void bresenham_core_inc(struct bresenham_core *core) {
    if (core->is_pos) {
        core->x_l++;
    } else {
        core->x_l--;
    }
}

void bresenham_core_init(struct bresenham_core *core, struct graphics_screen_pos a, struct graphics_screen_pos b, bool flip_excl) {
    core->x_l = a.x;
    uint8_t dy = b.y - a.y;
    core->is_pos = a.x <= b.x;
    core->is_excl = core->is_pos;
    if (flip_excl) {
        core->is_excl = !core->is_excl;
    }
    uint8_t dx;
    if (core->is_pos) {
        dx = b.x - a.x;
    } else {
        dx = a.x - b.x;
    }
    core->e = dy - dx;

    core->two_dy_sub_two_dx = 2 * dy - 2 * dx;
    core->two_dx = 2 * dx;

    if (!core->is_excl && core->e == 0) {
        // E is "negated" compared to the positive algo
        // unfortunately E can equal 0 in the first step
        core->e += core->two_dy_sub_two_dx;
        bresenham_core_inc(core);
    }
}

void bresenham_core_init_simple(struct bresenham_core *core, struct graphics_screen_pos a, struct graphics_screen_pos b) {
    bresenham_core_init(core, a, b, false);
}

int8_t bresenham_core_iter(struct bresenham_core *core) {
    // in the real algorithm, the lowest E can get to is -dx.
    // we've established since E is unsigned that the algorithm doesn't trigger
    // the while loop in the first pass. so we can safely "rotate" the phase of the algorithm
    // the point of rotating it is to keep it positive (add before subtracting)
    int8_t ret = core->x_l;
    // if subtracting 2*dx would underflow...
    bool should_inc = core->e < core->two_dx || (!core->is_excl && core->e == core->two_dx);
    if (should_inc) {
        bresenham_core_inc(core);
        core->e += core->two_dy_sub_two_dx;
    } else {
        core->e -= core->two_dx;
    }
    return ret;
}

int8_t bresenham_core_cur_x_l(struct bresenham_core *core) {
    return core->x_l;
}

bool bresenham_check_swapped(struct graphics_screen_pos a, struct graphics_screen_pos b) {
    uint8_t dx = abs_diff_i8(a.x, b.x);
    uint8_t dy = abs_diff_i8(a.y, b.y);
    return dx > dy;
}

void bresenham_init(struct bresenham *bres, struct graphics_screen_pos a, struct graphics_screen_pos b) {
    bres->is_swapped = bresenham_check_swapped(a, b);
    
    if (bres->is_swapped) {
        /*
        observations
        invariant: output length should be absDiffi8(aT.x, bT.x)
        last run of output.x is not included in output because input.y is exclusive
        uses the last output.x of each run as true Y
        */
        struct graphics_screen_pos a_t = graphics_screen_pos_flip(a);
        struct graphics_screen_pos b_t = graphics_screen_pos_flip(b);
        if (a_t.y > b_t.y) {
            graphics_swap_pos(&a_t, &b_t);
        }
        bres->prev_fake_x = a_t.x;
        bres->swapped_is_pos = a_t.x >= b_t.x;
        bres->fake_y = (bres->swapped_is_pos ? a_t.y - 1 : b_t.y + 1);
        bresenham_core_init_simple(&bres->core, a_t, b_t);
    } else {
        bresenham_core_init_simple(&bres->core, a, b);
    }
}

int8_t bresenham_iter(struct bresenham *bres) {
    if (bres->is_swapped) {
        int8_t fake_x;
        do {
            fake_x = bresenham_core_iter(&bres->core);
            if (bres->swapped_is_pos) {
                bres->fake_y++;
            } else {
                bres->fake_y--;
            }
        } while (fake_x == bres->prev_fake_x);
        bres->prev_fake_x = fake_x;

        return bres->fake_y;
    } else {
        return bresenham_core_iter(&bres->core);
    }
}
