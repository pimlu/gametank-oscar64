#include "imul.h"


#include <stdint.h>
#include <stdbool.h>


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

uint64_t mul32_to_64(uint32_t x, uint32_t y) {
    // Karatsuba algorithm: split 32-bit numbers into 16-bit halves
    const uint8_t M = 16;
    const uint32_t MBIT = ((uint32_t)1) << M;
    const uint32_t MASK = MBIT - 1;
    
    uint16_t x0 = x & MASK;
    uint16_t x1 = x >> M;
    uint16_t y0 = y & MASK;
    uint16_t y1 = y >> M;
    
    // z0 = x0 * y0 (low 32 bits of result)
    uint64_t z0 = mul16_to_32(x0, y0);
    
    // z2 = x1 * y1 (high 32 bits of result, shifted left by 32)
    uint64_t z2 = mul16_to_32(x1, y1);
    
    // Compute z3 = (x1 + x0) * (y1 + y0)
    // The tricky part: these sums can overflow 16 bits, so we need to handle carry
    uint32_t z3l = (uint32_t)x1 + (uint32_t)x0;
    uint32_t z3r = (uint32_t)y1 + (uint32_t)y0;
    uint32_t z3lLo = z3l & MASK;
    uint32_t z3rLo = z3r & MASK;
    uint64_t z3Base = mul16_to_32((uint16_t)z3lLo, (uint16_t)z3rLo);
    bool z3lCarry = (z3l >> M) != 0;
    bool z3rCarry = (z3r >> M) != 0;
    uint64_t carry = 0;
    if (z3lCarry) {
        carry += z3rLo;
    }
    if (z3rCarry) {
        carry += z3lLo;
    }
    if (z3lCarry && z3rCarry) {
        carry += MBIT;
    }
    
    uint64_t z3 = z3Base + (carry << M);
    
    // z1 = z3 - z2 - z0
    uint64_t z1 = z3 - z2 - z0;
    
    // Combine: result = z0 + (z1 << 16) + (z2 << 32)
    z1 <<= M;
    z2 <<= (M * 2);
    
    return z0 + z1 + z2;
}

// TODO overflow probably handled wrong

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

int64_t imul32_to_64(int32_t x, int32_t y) {
    bool neg = false;
    uint32_t ux, uy;
    if (x < 0) {
        neg ^= true;
        ux = -(uint32_t)x;
    } else {
        ux = x;
    }
    if (y < 0) {
        neg ^= true;
        uy = -(uint32_t)y;
    } else {
        uy = y;
    }
    int64_t res = (int64_t) mul32_to_64(ux, uy);
    return neg ? -res : res;
}
