#include "boot.h"

#include <stdint.h>

// // add a dummy value to data62 so that oscar64 generates something
// #pragma data(data62)
// __export uint8_t foo = 2;
// __export uint8_t bar = 2;


// put vector table functions on fixed bank since the banked memory is not configured yet
#pragma code(code63)
#pragma data(data63)


// NMI is triggered by GameTank vblank
__asm nmi_handler {
    rti
}

// IRQ is triggered when blits finish
__asm irq_handler {
    byt 0x9c, 0x06, 0x40 // stz 0x4006 (DMA_Start)
    rti
}

#include "bcr.h"
#include "scr.h"
#include "via.h"
#include "imul.h"

#include "graphics/screen.h"

int main(void) {
    // set the banking register first since the "mirror" registers
    // are affected by the memory bank
    scr.bankingReg.write(0);

    // why 254? apparently 255 (which is the same as 127, but they recommend setting the MSB)
    // is always in the fixed section, and this is the bank immediately preceding.
    via.changeRomBank(254);
    mulInit();


    scr.audioRst.write(0);
    scr.audioNmi.write(0);
    scr.audioCfg.write(0);
    scr.videoCfg.write(0);


    bcr.resetIrq();

    scr.setDefaultVideoFlags();
    
    scr.flipFramebuffer();


    uint8_t black = (uint8_t) ~0b00100000;
    graphics::clearBorder(black);
    scr.flipFramebuffer();
    graphics::clearBorder(black);

    gameStart();

    // write to a debug address to log in the emulator
    for(;;) {
        
    }

    return 0;
}

#pragma data(boot)
__export struct Boot {
    void *nmi, *reset, *irq;
} boot = { nmi_handler, (void *)0xC000, irq_handler };

#pragma data(data63)
