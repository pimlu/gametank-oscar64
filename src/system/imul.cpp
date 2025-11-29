#include "imul.h"


#include <stdint.h>


#pragma code(code63)
#pragma data(data63)


// based off mult66

// 8-bit x 8-bit unsigned multiply, 16-bit result
// Uses quarter-square method: a*b = ((a+b)² - (a-b)²) / 4
uint16_t imul(uint8_t a, uint8_t b) {
    uint16_t sum_idx = (uint16_t)a + b;  // 0..510
    
    // Get (a+b)²/4 from table1
    uint16_t sum_sq = ((uint16_t)squaretable1_msb[sum_idx] << 8) | squaretable1_lsb[sum_idx];


    
    // Get (a-b)²/4 - use table1 if b >= a, table2 otherwise
    uint16_t diff_sq;
    if (b >= a) {
        uint8_t diff_idx = b - a;
        diff_sq = ((uint16_t)squaretable1_msb[diff_idx] << 8) | squaretable1_lsb[diff_idx];
    } else {
        uint8_t diff_idx = b - a;  // wraps to 256 + (b - a)
        diff_sq = ((uint16_t)squaretable2_msb[diff_idx] << 8) | squaretable2_lsb[diff_idx];
        diff_sq++;  // Table2 has -1 baked in for 6502 carry trick; compensate in C
    }
    
    return sum_sq - diff_sq;
}


__zeropage uint8_t* lmul0;
__zeropage uint8_t* lmul1;
// Call once at startup
void imulInit() {
    __asm {
        lda #>squaretable1_lsb
        sta lmul0 + 1
        lda #>squaretable1_msb
        sta lmul1 + 1
    }
}

uint16_t imulAsm(uint8_t a, uint8_t b) {
    return __asm {
        ldx a
        ldy b

        stx lmul0           // Set low byte of pointer (index into table)
        stx lmul1
        tya
        sec
        sbc lmul0
        tax
        lda (lmul0),Y       // Get table1_lsb[a+b]
        bcc lbl_table2
        
        // Case: b >= a, use table1 for difference
        sbc squaretable1_lsb,X
        sta accu            // LOW BYTE of result
        lda (lmul1),Y       // Get table1_msb[a+b]
        sbc squaretable1_msb,X
        sta accu + 1        // HIGH BYTE of result
        jmp done
        
    lbl_table2:
        // Case: b < a, use table2 for difference  
        sbc squaretable2_lsb,X
        sta accu            // LOW BYTE of result
        lda (lmul1),Y
        sbc squaretable2_msb,X
        sta accu + 1        // HIGH BYTE of result
    done:
    };
}
