#include "interrupts.h"


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


inline void waitForInterrupt() {
    __asm volatile {
        byt 0xcb // wai
    };
}