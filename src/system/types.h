#pragma once

#include <stdint.h>
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Bit manipulation functions
uint8_t bits_update(uint8_t reg, uint8_t bit, bool val);
uint8_t bits_toggle(uint8_t reg, uint8_t bit);
bool bits_get(uint8_t reg, uint8_t bit);
uint8_t bits_one_shift_n(uint8_t n);


#pragma compile("types.c")
