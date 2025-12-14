#include "horizon.h"

#include "geometry/fixed_etc.h"
#include "system/i8helpers.h"
#include "graphics/screen.h"
#include "system/bcr.h"
#include "system/interrupts.h"

#pragma code(code62)
#pragma data(data62)
#pragma bss(bss)

void draw_horizon(geof_t pos_) {
    int8_t pos = -geometry_round_geof(pos_);

    int8_t ocean_lo = GRAPHICS_SCENE_Y_TO_FRAME + GRAPHICS_SCENE_Y_LO;
    int8_t ocean_hi = GRAPHICS_SCENE_Y_TO_FRAME + min_i8(GRAPHICS_SCENE_Y_HI, pos);

    int8_t sand_lo = GRAPHICS_SCENE_Y_TO_FRAME + max_i8(GRAPHICS_SCENE_Y_LO, pos);
    int8_t sand_hi = GRAPHICS_SCENE_Y_TO_FRAME + GRAPHICS_SCENE_Y_HI;

    
    if (ocean_lo < ocean_hi) {
        bcr_draw_box(
            GRAPHICS_FRAME_X_LO, ocean_lo,
            GRAPHICS_FRAME_W, ocean_hi - ocean_lo,
            (uint8_t) ~0b11011011);
        wait_for_interrupt();
    }
    if (sand_lo < sand_hi) {
        bcr_draw_box(
            GRAPHICS_FRAME_X_LO, sand_lo,
            GRAPHICS_FRAME_W, sand_hi - sand_lo,
            (uint8_t) ~0b01001100);
        wait_for_interrupt();
    }
}

