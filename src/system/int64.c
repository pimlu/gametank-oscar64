#include "int64.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

uint64_box uint64_box_from_uint32(uint32_t value) {
    uint64_box result;
    result.lo = value;
    result.hi = 0;
    return result;
}

int64_box int64_box_from_int32(int32_t value) {
    int64_box result;
    if (value >= 0) {
        result.lo = (uint32_t)value;
        result.hi = 0;
    } else {
        // For negative values, we need to sign-extend
        result.lo = (uint32_t)value;
        result.hi = 0xFFFFFFFF;
    }
    return result;
}

int64_box int64_box_from_uint64_box(uint64_box value) {
    int64_box result;
    result.lo = value.lo;
    result.hi = value.hi;
    return result;
}

uint64_box uint64_box_add(uint64_box a, uint64_box b) {
    uint64_box result;
    uint32_t sum_lo = a.lo + b.lo;
    uint32_t carry = (sum_lo < a.lo) ? 1 : 0;  // Check for overflow
    result.lo = sum_lo;
    result.hi = a.hi + b.hi + carry;
    return result;
}

uint64_box uint64_box_sub(uint64_box a, uint64_box b) {
    uint64_box result;
    uint32_t borrow = (a.lo < b.lo) ? 1 : 0;  // Check if we need to borrow
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi - borrow;
    return result;
}

int64_box int64_box_add(int64_box a, int64_box b) {
    int64_box result;
    // Treat as unsigned for addition, then handle sign
    uint32_t sum_lo = (uint32_t)a.lo + (uint32_t)b.lo;
    uint32_t carry = (sum_lo < (uint32_t)a.lo) ? 1 : 0;
    result.lo = sum_lo;
    result.hi = a.hi + b.hi + carry;
    return result;
}

int64_box int64_box_sub(int64_box a, int64_box b) {
    int64_box result;
    // Treat as unsigned for subtraction, then handle borrow
    uint32_t borrow = ((uint32_t)a.lo < (uint32_t)b.lo) ? 1 : 0;
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi - borrow;
    return result;
}

int64_box int64_box_neg(int64_box a) {
    int64_box result;
    // Two's complement negation: ~a + 1
    uint32_t not_lo = ~(uint32_t)a.lo;
    uint32_t not_hi = ~(uint32_t)a.hi;
    uint32_t sum_lo = not_lo + 1;
    uint32_t carry = (sum_lo < not_lo) ? 1 : 0;
    result.lo = sum_lo;
    result.hi = not_hi + carry;
    return result;
}

uint64_box uint64_box_shl(uint64_box a, uint8_t shift) {
    uint64_box result;
    if (shift == 0) {
        return a;
    }
    if (shift >= 64) {
        result.lo = 0;
        result.hi = 0;
        return result;
    }
    if (shift >= 32) {
        // Shift >= 32: move hi to lo, clear hi
        result.lo = a.hi << (shift - 32);
        result.hi = 0;
        return result;
    }
    // Shift < 32: normal case
    result.lo = a.lo << shift;
    result.hi = (a.hi << shift) | (a.lo >> (32 - shift));
    return result;
}

uint64_box uint64_box_shr(uint64_box a, uint8_t shift) {
    uint64_box result;
    if (shift == 0) {
        return a;
    }
    if (shift >= 64) {
        result.lo = 0;
        result.hi = 0;
        return result;
    }
    if (shift >= 32) {
        // Shift >= 32: move hi to lo, clear hi
        result.lo = a.hi >> (shift - 32);
        result.hi = 0;
        return result;
    }
    // Shift < 32: normal case
    result.lo = (a.lo >> shift) | (a.hi << (32 - shift));
    result.hi = a.hi >> shift;
    return result;
}

int64_box int64_box_shl(int64_box a, uint8_t shift) {
    int64_box result;
    if (shift == 0) {
        return a;
    }
    if (shift >= 64) {
        result.lo = 0;
        result.hi = 0;
        return result;
    }
    if (shift >= 32) {
        // Shift >= 32: move hi to lo, sign-extend hi
        result.lo = a.hi << (shift - 32);
        result.hi = (a.hi < 0) ? 0xFFFFFFFF : 0;
        return result;
    }
    // Shift < 32: normal case
    result.lo = a.lo << shift;
    result.hi = (a.hi << shift) | ((uint32_t)a.lo >> (32 - shift));
    return result;
}

int64_box int64_box_shr(int64_box a, uint8_t shift) {
    int64_box result;
    if (shift == 0) {
        return a;
    }
    if (shift >= 64) {
        // Arithmetic right shift: sign-extend
        result.lo = (a.hi < 0) ? 0xFFFFFFFF : 0;
        result.hi = (a.hi < 0) ? 0xFFFFFFFF : 0;
        return result;
    }
    if (shift >= 32) {
        // Shift >= 32: move hi to lo, sign-extend hi
        result.lo = a.hi >> (shift - 32);
        result.hi = (a.hi < 0) ? 0xFFFFFFFF : 0;
        return result;
    }
    // Shift < 32: normal case
    result.lo = ((uint32_t)a.lo >> shift) | ((uint32_t)a.hi << (32 - shift));
    result.hi = a.hi >> shift;  // Arithmetic right shift preserves sign
    return result;
}
