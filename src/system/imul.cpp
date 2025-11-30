#include "imul.h"


#include <stdint.h>


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
void mulInit() {
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
uint16_t mul8To16(uint8_t a, uint8_t b) {
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
uint32_t mul16To32(uint16_t in_x, uint16_t in_y) {
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



// TODO overflow probably handled wrong

int16_t imul8To16(int8_t x, int8_t y) {
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
    int16_t res = (int16_t) mul8To16(ux, uy);
    return neg ? -res : res;
}

int32_t imul16To32(int16_t x, int16_t y) {
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
    int32_t res = (int32_t) mul16To32(ux, uy);
    return neg ? -res : res;
}
