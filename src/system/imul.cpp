#include "imul.h"


#include <stdint.h>


#pragma code(code63)
#pragma data(data63)


__zeropage uint8_t* lmul0;
__zeropage uint8_t* lmul1;
// Call once at startup
void mulInit() {
    __asm {
        lda #>sqrlo
        sta lmul0 + 1
        lda #>sqrhi
        sta lmul1 + 1
    }
}


// based off https://github.com/TobyLobster/multiply_test/blob/main/tests/mult65.a
uint16_t mulAsm(uint8_t a, uint8_t b) {
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
