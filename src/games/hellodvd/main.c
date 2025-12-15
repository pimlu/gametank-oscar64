#include "system/boot.h"

#include <stdint.h>

#include "system/bcr.h"
#include "system/interrupts.h"
#include "system/scr.h"
#include "graphics/screen.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void init(void);
void tick(void);

void game_start(void) {
    init();

    for (;;) {
        scr_set_enable_vblank_nmi(false);

        tick();
        scr_flip_framebuffer();
        
        scr_set_enable_vblank_nmi(true);
        wait_for_interrupt();
    }
}


uint8_t blue = (uint8_t) ~0b11011100;
uint8_t red  = (uint8_t) ~0b01011110;

uint8_t size = 10;

uint8_t x, y;
int8_t dx, dy;

void init(void) {
    x = 20;
    y = 70;

    dx = 1;
    dy = 1;
}

// draw a bouncing box on the screen
void tick(void) {
    graphics_clear_screen(blue);

    bcr_draw_box(x, y, size, size, red);
    wait_for_interrupt();

    if (x + dx < GRAPHICS_FRAME_X_LO || x + size + dx >= GRAPHICS_FRAME_X_HI) {
        dx = -dx;
    }

    if (y + dy < GRAPHICS_FRAME_Y_LO || y + size + dy >= GRAPHICS_FRAME_Y_HI) {
        dy = -dy;
    }

    x += dx;
    y += dy;
}