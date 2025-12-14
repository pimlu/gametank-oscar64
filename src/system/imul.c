#include "imul.h"

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>


#ifdef __OSCAR64C__

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


__zeropage uint8_t* lmul0;
__zeropage uint8_t* lmul1;

// pointers for 16x16 multiply
__zeropage uint8_t* p_sqr_lo1;
__zeropage uint8_t* p_sqr_hi1;
__zeropage uint8_t* p_neg_sqr_lo;
__zeropage uint8_t* p_neg_sqr_hi;
__zeropage uint8_t* p_sqr_lo2;
__zeropage uint8_t* p_sqr_hi2;

// intermediate products for 16x16 multiply (replaces self-modifying code)
__zeropage uint8_t sm_x1y0l;
__zeropage uint8_t sm_x1y0h;
__zeropage uint8_t sm_x1y1l;
__zeropage uint8_t sm_x0y1l;

// Call once at startup
void mul_init(void) {
    __asm {
        // 8x8 multiply pointers
        lda #>sqrlo
        sta lmul0 + 1
        lda #>sqrhi
        sta lmul1 + 1

        // 16x16 multiply pointers
        lda #>sqrlo
        sta p_sqr_lo1 + 1
        sta p_sqr_lo2 + 1
        lda #>sqrhi
        sta p_sqr_hi1 + 1
        sta p_sqr_hi2 + 1
        lda #>negsqrlo
        sta p_neg_sqr_lo + 1
        lda #>negsqrhi
        sta p_neg_sqr_hi + 1
    }
}


// based off https://github.com/TobyLobster/multiply_test/blob/main/tests/mult65.a
uint16_t mul8_to_16(uint8_t a, uint8_t b) {
    return __asm {
        ldx a
        ldy b

        stx lmul0
        stx lmul1
        tya
        sec
        sbc lmul0
        bcs lbl_skip
        sbc #0          // negate A
        eor #$ff
    lbl_skip:
        tax
        lda (lmul0),Y
        sbc sqrlo,X
        sta accu
        lda (lmul1),Y
        sbc sqrhi,X

        sta accu + 1
    };
}


// based off https://github.com/TobyLobster/multiply_test/blob/main/tests/mult86.a
// 16 bit x 16 bit unsigned multiply, 32 bit result
uint32_t mul16_to_32(uint16_t in_x, uint16_t in_y) {
    // Access in_x and in_y directly as bytes (little-endian):
    // in_x = low byte (x0), in_x+1 = high byte (x1)
    // in_y = low byte (y0), in_y+1 = high byte (y1)

    return __asm {
        // set multiplier as x1
        lda in_x+1
        sta p_sqr_lo2
        sta p_sqr_hi1
        eor #$ff
        sta p_neg_sqr_lo
        sta p_neg_sqr_hi

        // set multiplicand as y0
        ldy in_y

        // x1y0l = low(x1*y0)
        // x1y0h = high(x1*y0)
        sec
        lda (p_sqr_lo2),y
        sbc (p_neg_sqr_lo),y
        sta sm_x1y0l
        lda (p_sqr_hi1),y
        sbc (p_neg_sqr_hi),y
        sta sm_x1y0h

        // set multiplicand as y1
        ldy in_y+1

        // x1y1l = low(x1*y1)
        // z3 = high(x1*y1)
        lda (p_sqr_lo2),y
        sbc (p_neg_sqr_lo),y
        sta sm_x1y1l
        lda (p_sqr_hi1),y
        sbc (p_neg_sqr_hi),y
        sta accu+3

        // set multiplier as x0
        lda in_x
        sta p_sqr_lo1
        sta p_sqr_hi2
        eor #$ff
        sta p_neg_sqr_lo
        sta p_neg_sqr_hi

        // x0y1l = low(x0*y1)
        // X = high(x0*y1)
        lda (p_sqr_lo1),y
        sbc (p_neg_sqr_lo),y
        sta sm_x0y1l
        lda (p_sqr_hi2),y
        sbc (p_neg_sqr_hi),y
        tax

        // set multiplicand as y0
        ldy in_y

        // z0 = low(x0*y0)
        // A = high(x0*y0)
        lda (p_sqr_lo1),y
        sbc (p_neg_sqr_lo),y
        sta accu
        lda (p_sqr_hi2),y
        sbc (p_neg_sqr_hi),y

        clc

        // add the first two numbers of column 1
        adc sm_x0y1l    // x0y0h + x0y1l
        tay

        // continue to first two numbers of column 2
        txa
        adc sm_x1y0h    // x0y1h + x1y0h
        tax             // X=z2 so far
        bcc skip1
        inc accu+3      // column 3
        clc

        // add last number of column 1
    skip1:
        tya
        adc sm_x1y0l    // + x1y0l
        sta accu+1      // z1

        // add last number of column 2
        txa
        adc sm_x1y1l    // + x1y1l
        sta accu+2      // z2
        bcc fin
        inc accu+3      // column 3
    fin:
    };
}

#else

void mul_init(void) {
    // no initialization needed
}

uint16_t mul8_to_16(uint8_t a, uint8_t b) {
    return (uint16_t)a * (uint16_t)b;
}

uint32_t mul16_to_32(uint16_t x, uint16_t y) {
    return (uint32_t)x * (uint32_t)y;
}

#endif

int16_t imul8_to_16(int8_t x, int8_t y) {
    bool neg = false;
    uint8_t ux, uy;
    if (x < 0) {
        neg ^= true;
        ux = -(uint8_t)x;
    } else {
        ux = x;
    }
    if (y < 0) {
        neg ^= true;
        uy = -(uint8_t)y;
    } else {
        uy = y;
    }
    int16_t res = (int16_t) mul8_to_16(ux, uy);
    return neg ? -res : res;
}

int32_t imul16_to_32(int16_t x, int16_t y) {
    bool neg = false;
    uint16_t ux, uy;
    if (x < 0) {
        neg ^= true;
        ux = -(uint16_t)x;
    } else {
        ux = x;
    }
    if (y < 0) {
        neg ^= true;
        uy = -(uint16_t)y;
    } else {
        uy = y;
    }
    int32_t res = (int32_t) mul16_to_32(ux, uy);
    return neg ? -res : res;
}


// returns (x * y) >> 24
// returns int16 result; sets *oflag if overflow, else clears it
int16_t mul_shr24_sat0(int32_t x, uint16_t y, bool *oflag) {
    uint16_t x2x1 = (x & 0x00ffff00) >> 8;
    uint32_t x2x1_y_full = mul16_to_32(x2x1, y);
    uint32_t x2x1_y = x2x1_y_full >> 16;
    
    // Extract lower 8 bits of x (x0) and compute x0 * y
    // Split y into bytes: x0 * y = x0 * (y_low + y_high * 256)
    uint8_t x0 = x & 0xff;
    uint32_t x0_y = mul8_to_16(x0, y & 0xff) + ((uint32_t) mul8_to_16(x0, y >> 8) << 8);
    
    // Compute carry from lower bits: x2x1_y_low (bits [8:23]) + x0_y (bits [0:15])
    // The lower 16 bits of x2x1_y_full represent bits [8:23] in the final product
    uint16_t x2x1_y_low = x2x1_y_full & 0xffff;
    uint32_t low_sum = x0_y + ((uint32_t)x2x1_y_low << 8);
    uint32_t carry = low_sum >> 24;

    // Extract top byte of x (x3) and compute its contribution
    // x3 is at position 24, so (x3 * y) >> 24 = x3 * y (contributes directly)
    int8_t x3 = x >> 24;
    bool neg = x3 < 0;
    uint8_t pos_x3 = neg ? -x3 : x3;  // pos_x3 is at most 128, fits in uint8_t
    uint32_t x3_y_full = mul8_to_16(pos_x3, y & 0xff) + ((uint32_t)mul8_to_16(pos_x3, y >> 8) << 8);
    int32_t x3_y_shifted = neg ? -(int32_t)x3_y_full : (int32_t)x3_y_full;

    int32_t result_signed = (int32_t)x2x1_y + x3_y_shifted + (int32_t)carry;
    
    bool overflow = result_signed < INT16_MIN || result_signed > INT16_MAX;
    *oflag = overflow;
    return overflow ? 0 : result_signed;
}
