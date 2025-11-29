#pragma once

#include <stdint.h>

// 8-bit x 8-bit unsigned multiply, 16-bit result
// Fast quarter-square multiplication (~45 cycles in asm)
uint16_t imul(uint8_t a, uint8_t b);
void imulInit();
uint16_t imulAsm(uint8_t a, uint8_t b);

extern const uint8_t squaretable1_lsb[511];
extern const uint8_t squaretable1_msb[511];
extern const uint8_t squaretable2_lsb[256];
extern const uint8_t squaretable2_msb[256];

#pragma compile("imul.cpp")

#pragma compile("imul_const.cpp")

