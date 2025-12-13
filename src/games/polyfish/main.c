#include "system/boot.h"

#include <stdint.h>

#include "system/bcr.h"
#include "system/imul.h"
#include "system/interrupts.h"
#include "system/scr.h"

#include "graphics/screen.h"
#include "graphics/triangle.h"

#include "geometry/types.h"
#include "geometry/geof.h"
#include "geometry/sin_lut.h"

#include "geometry/constants.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void init(void);
void tick(void);

void game_start(void) {
    mul_init();

    init();
    
    // geof_t foo = {GEOF_FROM_DOUBLE(1.2)};
    // geof_t baz = {GEOF_FROM_DOUBLE(2.5)};
    // geof_t quz = geof_mul(foo, baz);
    // *(volatile int16_t*) 0x2008 = geof_get_raw(quz);

    // iunitf_t bar = geometry_sin((geof_t){GEOF_FROM_DOUBLE(32.0/3.0)});
    // *(volatile int16_t*) 0x2008 = unitf_get_raw(bar.val);

    // iunitf_t bar = geometry_sin((geof_t){GEOF_PI / 6});
    // *(volatile int16_t*) 0x2008 = unitf_get_raw(bar);
    scr_flip_framebuffer();


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
    *(volatile uint8_t*) 0x2008 = 0xf0;
    graphics_clear_screen(blue);
    *(volatile uint8_t*) 0x2008 = 0xa1;
    *(volatile int8_t*) 0x2008 = 1;

    graphics_fill_triangle((struct graphics_screen_pos){-10,-10}, (struct graphics_screen_pos){20,10}, (struct graphics_screen_pos){30, 30}, red);

    if (x + dx < GRAPHICS_FRAME_X_LO || x + size + dx >= GRAPHICS_FRAME_X_HI) {
        dx = -dx;
    }

    if (y + dy < GRAPHICS_FRAME_Y_LO || y + size + dy >= GRAPHICS_FRAME_Y_HI) {
        dy = -dy;
    }

    x += dx;
    y += dy;
}