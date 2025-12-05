#include "interrupts.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void wait_for_interrupt(void) {
    __asm volatile {
        byt 0xcb // wai
    };
}