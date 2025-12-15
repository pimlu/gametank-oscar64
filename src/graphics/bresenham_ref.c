#include "bresenham_ref.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void bresenham_reference_pos_init(bresenham_reference_pos_t *ref, 
                                 struct graphics_screen_pos a, 
                                 struct graphics_screen_pos b) {
    ref->a = a;
    ref->b = b;
    ref->E = (b.y - a.y) - (b.x - a.x);
    ref->X_left = a.x;
}

int8_t bresenham_reference_pos_iter(bresenham_reference_pos_t *ref) {
    while (ref->E < 0) {
        ref->X_left++;
        ref->E += 2 * (ref->b.y - ref->a.y);
    }
    ref->E -= 2 * (ref->b.x - ref->a.x);
    return (int8_t)ref->X_left;
}

void bresenham_reference_neg_init(bresenham_reference_neg_t *ref,
                                  struct graphics_screen_pos a,
                                  struct graphics_screen_pos b) {
    ref->a = a;
    ref->b = b;
    ref->E = -(b.y - a.y) - (b.x - a.x);
    ref->X_left = a.x;
}

int8_t bresenham_reference_neg_iter(bresenham_reference_neg_t *ref) {
    while (ref->E >= 0) {
        ref->X_left--;
        ref->E -= 2 * (ref->b.y - ref->a.y);
    }
    ref->E -= 2 * (ref->b.x - ref->a.x);
    return (int8_t)ref->X_left;
}

void bresenham_reference_init(bresenham_reference_t *ref,
                              struct graphics_screen_pos a,
                              struct graphics_screen_pos b) {
    ref->is_pos = b.x >= a.x;
    if (ref->is_pos) {
        bresenham_reference_pos_init(&ref->pos, a, b);
    } else {
        bresenham_reference_neg_init(&ref->neg, a, b);
    }
}

int8_t bresenham_reference_iter(bresenham_reference_t *ref) {
    if (ref->is_pos) {
        return bresenham_reference_pos_iter(&ref->pos);
    } else {
        return bresenham_reference_neg_iter(&ref->neg);
    }
}

