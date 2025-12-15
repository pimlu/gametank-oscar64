#include "screen.h"

#include "system/bcr.h"
#include "system/interrupts.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void graphics_clear_border(uint8_t c) {
    bcr_draw_box(0, 0, GRAPHICS_SCREEN_WIDTH - GRAPHICS_PADDING_RIGHT, GRAPHICS_PADDING_TOP, c);
    wait_for_interrupt();
    bcr_draw_box(0, GRAPHICS_PADDING_TOP, GRAPHICS_PADDING_RIGHT, GRAPHICS_SCREEN_HEIGHT - GRAPHICS_PADDING_TOP, c);
    wait_for_interrupt();
    bcr_draw_box(GRAPHICS_PADDING_LEFT, GRAPHICS_SCREEN_HEIGHT - GRAPHICS_PADDING_BOT, GRAPHICS_SCREEN_WIDTH - GRAPHICS_PADDING_LEFT, GRAPHICS_PADDING_BOT, c);
    wait_for_interrupt();
    bcr_draw_box(GRAPHICS_SCREEN_WIDTH - GRAPHICS_PADDING_RIGHT, 0, GRAPHICS_PADDING_RIGHT, GRAPHICS_SCREEN_HEIGHT - GRAPHICS_PADDING_BOT, c);
    wait_for_interrupt();
}

void graphics_clear_screen(uint8_t c) {
    bcr_draw_box(
        GRAPHICS_FRAME_X_LO, GRAPHICS_FRAME_Y_LO,
        GRAPHICS_FRAME_W, GRAPHICS_FRAME_H,
        c);
    wait_for_interrupt();
}