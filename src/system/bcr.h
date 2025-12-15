#pragma once
#include "types.h"
#include "reg_macros.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// https://wiki.gametank.zone/doku.php?id=hardware:blitter#blitter_registers
DECLARE_MEMREG(bcr_reg_vx);
DECLARE_MEMREG(bcr_reg_vy);
DECLARE_MEMREG(bcr_reg_gx);
DECLARE_MEMREG(bcr_reg_gy);
DECLARE_MEMREG(bcr_reg_width);
DECLARE_MEMREG(bcr_reg_height);
DECLARE_MEMREG(bcr_reg_start);
DECLARE_MEMREG(bcr_reg_color);

void bcr_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c);
void bcr_reset_irq(void);
void bcr_setup_row_fill(uint8_t c);
void bcr_trigger_row_fill(uint8_t x, uint8_t y, uint8_t w);

#pragma compile("bcr.c")
