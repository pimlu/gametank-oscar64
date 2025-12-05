#include "scr.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

DEFINE_MEMREG(scr_reg_audio_rst, 0x2000);
DEFINE_MEMREG(scr_reg_audio_nmi, 0x2001);
DEFINE_MEMREG(scr_reg_banking, 0x2005);
DEFINE_MEMREG(scr_reg_audio_cfg, 0x2006);
DEFINE_MEMREG(scr_reg_video_cfg, 0x2007);
DEFINE_READREG(scr_reg_gamepad1, 0x2008);
DEFINE_READREG(scr_reg_gamepad2, 0x2009);

void scr_set_colorfill_mode(bool enabled) {
    scr_reg_video_cfg_set_bit(3, enabled);
}

void scr_flip_framebuffer(void) {
    // DMA_PAGE_OUT - Select framebuffer page sent to TV
    scr_reg_video_cfg_toggle_bit(1);
    // "Select which framebuffer to read/write/blit"
    scr_reg_banking_toggle_bit(3);
}

void scr_set_default_video_flags(void) {
    // DMA_PAGE_OUT (framebuffer) should be opposite the blitter target in order
    // to prevent screen tearing
    scr_reg_video_cfg_write(0b01101011);
}

void scr_set_enable_vblank_nmi(bool enabled) {
    scr_reg_video_cfg_set_bit(2, enabled);
}

uint16_t scr_read_gamepad1(void) {
    // according to https://gametank.zone/manual/, reset the state via reading 2 first
    scr_reg_gamepad2_read();
    
    uint8_t lo = scr_reg_gamepad1_read();
    uint8_t hi = scr_reg_gamepad1_read();
    uint16_t combo = (hi << 8) | lo;

    return ~combo;
}