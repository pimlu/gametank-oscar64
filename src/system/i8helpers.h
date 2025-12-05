#pragma once

#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

int8_t min_i8(int8_t a, int8_t b);
int8_t max_i8(int8_t a, int8_t b);
uint8_t abs_i8(int8_t a);
uint8_t abs_diff_i8(int8_t a, int8_t b);

#pragma compile("i8helpers.c")