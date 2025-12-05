#pragma once

#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct graphics_screen_pos {
    int8_t x, y;
};

struct graphics_screen_pos graphics_screen_pos_transpose(struct graphics_screen_pos pos);
struct graphics_screen_pos graphics_screen_pos_flip(struct graphics_screen_pos pos);
struct graphics_screen_pos graphics_screen_pos_unflip(struct graphics_screen_pos pos);

#pragma compile("types.c")
