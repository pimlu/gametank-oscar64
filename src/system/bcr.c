#include "bcr.h"

#include "scr.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

DEFINE_MEMREG(bcr_reg_vx, 0x4000);
DEFINE_MEMREG(bcr_reg_vy, 0x4001);
DEFINE_MEMREG(bcr_reg_gx, 0x4002);
DEFINE_MEMREG(bcr_reg_gy, 0x4003);
DEFINE_MEMREG(bcr_reg_width, 0x4004);
DEFINE_MEMREG(bcr_reg_height, 0x4005);
DEFINE_MEMREG(bcr_reg_start, 0x4006);
DEFINE_MEMREG(bcr_reg_color, 0x4007);

void bcr_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c) {
    __asm {
        sei
    };

    scr_set_colorfill_mode(true);

    bcr_reg_vx_write(x);
    bcr_reg_vy_write(y);
    bcr_reg_width_write(w);
    bcr_reg_height_write(h);
    bcr_reg_color_write(c);
    
    bcr_reg_start_write(1);
    __asm {
        cli
        byt 0xcb // wai
    };
}

void bcr_reset_irq(void) {
    bcr_reg_start_write(0);
}

void bcr_setup_row_fill(uint8_t c) {
    scr_set_colorfill_mode(true);
    bcr_reg_height_write(1);
    bcr_reg_color_write(c);
}

static uint8_t write_times;

void bcr_trigger_row_fill(uint8_t x, uint8_t y, uint8_t w) {
    __asm {
        sei
    };

    // if (write_times < 4) {
    //     write_times++;

    //     *(volatile uint8_t*) 0x2008 = 0x42;
    //     *(volatile uint8_t*) 0x2008 = x;
    //     *(volatile uint8_t*) 0x2008 = y;
    //     *(volatile uint8_t*) 0x2008 = w;
    // }

    bcr_reg_vx_write(x);
    bcr_reg_vy_write(y);
    bcr_reg_width_write(w);

    bcr_reg_start_write(1);
}

void bcr_row_fill_wait(void) {
    // if (write_times < 4) {
    //     *(volatile uint8_t*) 0x2008 = 0x43;
    // }
    __asm volatile {
        cli
        // byt 0xcb // wai
    };
}