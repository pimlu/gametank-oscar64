#include "types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

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

