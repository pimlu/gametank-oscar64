#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "types.h"
#include "system/i8helpers.h"
#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// https://mcejp.github.io/2020/11/06/bresenham.html
struct bresenham_core {
    int8_t x_l;
    uint8_t two_dx;
    uint8_t two_dy_sub_two_dx;
    uint8_t e;
    bool is_pos;
};

void bresenham_core_init(struct bresenham_core *core, struct graphics_screen_pos a, struct graphics_screen_pos b);
int8_t bresenham_core_iter(struct bresenham_core *core);
int8_t bresenham_core_cur_x_l(struct bresenham_core *core);

struct bresenham {
    bool is_swapped;
    bool swapped_is_pos;
    struct bresenham_core core;
    int8_t fake_y;
    int8_t prev_fake_x;
};

bool bresenham_check_swapped(struct graphics_screen_pos a, struct graphics_screen_pos b);
void bresenham_init(struct bresenham *bres, struct graphics_screen_pos a, struct graphics_screen_pos b);
int8_t bresenham_iter(struct bresenham *bres);

#pragma compile("bresenham.c")
