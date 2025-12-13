#pragma once

#include <stdint.h>
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// 64-bit integer types for oscar64 (which doesn't support native 64-bit ints)
typedef struct {
    uint32_t lo;
    uint32_t hi;
} uint64_box;

typedef struct {
    int32_t lo;
    int32_t hi;
} int64_box;

// Constructors
uint64_box uint64_box_from_uint32(uint32_t value);
int64_box int64_box_from_int32(int32_t value);
int64_box int64_box_from_uint64_box(uint64_box value);

// Arithmetic operations
uint64_box uint64_box_add(uint64_box a, uint64_box b);
uint64_box uint64_box_sub(uint64_box a, uint64_box b);
int64_box int64_box_add(int64_box a, int64_box b);
int64_box int64_box_sub(int64_box a, int64_box b);

// Unary operations
int64_box int64_box_neg(int64_box a);

// Shift operations
uint64_box uint64_box_shl(uint64_box a, uint8_t shift);
uint64_box uint64_box_shr(uint64_box a, uint8_t shift);
int64_box int64_box_shl(int64_box a, uint8_t shift);
int64_box int64_box_shr(int64_box a, uint8_t shift);

#pragma compile("int64.c")
