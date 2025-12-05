#pragma once
#include "types.h"
#include "reg_macros.h"

#define INPUT_MASK_UP		2056
#define INPUT_MASK_DOWN		1028
#define INPUT_MASK_LEFT		512
#define INPUT_MASK_RIGHT	256
#define INPUT_MASK_A		16
#define INPUT_MASK_B		4096
#define INPUT_MASK_C		8192
#define INPUT_MASK_START	32
#define INPUT_MASK_ALL_KEYS INPUT_MASK_UP|INPUT_MASK_DOWN|INPUT_MASK_LEFT|INPUT_MASK_RIGHT|INPUT_MASK_A|INPUT_MASK_B|INPUT_MASK_C|INPUT_MASK_START

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// https://wiki.gametank.zone/doku.php?id=hardware:memorymap#system_control_registers
DECLARE_MEMREG(scr_reg_audio_rst);
DECLARE_MEMREG(scr_reg_audio_nmi);
DECLARE_MEMREG(scr_reg_banking);
DECLARE_MEMREG(scr_reg_audio_cfg);
DECLARE_MEMREG(scr_reg_video_cfg);
DECLARE_READREG(scr_reg_gamepad1);
DECLARE_READREG(scr_reg_gamepad2);

void scr_set_colorfill_mode(bool enabled);
void scr_flip_framebuffer(void);
void scr_set_default_video_flags(void);
void scr_set_enable_vblank_nmi(bool enabled);
uint16_t scr_read_gamepad1(void);

#pragma compile("scr.c")
