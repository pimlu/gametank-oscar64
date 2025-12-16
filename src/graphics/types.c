#include "types.h"

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

struct graphics_screen_pos graphics_screen_pos_transpose(struct graphics_screen_pos pos) {
    struct graphics_screen_pos result = {pos.y, pos.x};
    return result;
}

struct graphics_screen_pos graphics_screen_pos_flip(struct graphics_screen_pos pos) {
    struct graphics_screen_pos result = {(int8_t)-pos.y, pos.x};
    return result;
}

struct graphics_screen_pos graphics_screen_pos_unflip(struct graphics_screen_pos pos) {
    struct graphics_screen_pos result = {pos.y, (int8_t)-pos.x};
    return result;
}

