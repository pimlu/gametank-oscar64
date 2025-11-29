#pragma once

#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


uint16_t mul(uint8_t a, uint8_t b);
void mulInit();
uint16_t mulAsm(uint8_t a, uint8_t b);
uint32_t mulAsm16(uint16_t in_x, uint16_t in_y);

extern const uint8_t sqrlo[512];
extern const uint8_t sqrhi[512];
extern const uint8_t negsqrlo[512];
extern const uint8_t negsqrhi[512];

#pragma compile("imul.cpp")

#pragma compile("imul_const.cpp")

