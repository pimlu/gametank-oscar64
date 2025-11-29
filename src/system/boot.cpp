#include <stdint.h>

#pragma section( startup, 0 )
#pragma section( boot, 0 )
#pragma section( random, 0 )

#pragma region( zeropage, 0x010, 0x0100, , , { zeropage } )
#pragma region( main, 0x0100, 0x2000, , , { bss, random, heap, stack } )
#pragma stacksize(512)
#pragma heapsize(0)


// banked ROM (GameTank 126) - appears at $8000-$BFFF when bank 126 is selected
#pragma section( code62, 0 )
#pragma section( data62, 0 )
#pragma region( rom62, 0x8000, 0xC000, , 62, { code62, data62 } )

// fixed ROM (GameTank 127) - always visible at $C000-$FFFF
#pragma section( code63, 0 )
#pragma section( data63, 0 )
// startup code at the START of fixed bank (known address 0xC000)
// NOTE: region must be named exactly "startup" for oscar64 to use it
#pragma region( startup, 0xC000, 0xC100, , 63, { startup } )
// fixed code/data area after startup
#pragma region( rom63, 0xC100, 0xFFFA, , 63, { code, data, code63, data63 } )
// 6-byte vector table at the very top ($FFFA-$FFFF)
#pragma region( boot, 0xFFFA, 0x10000, , 63, { boot } )


// add a dummy value to data62 so that oscar64 generates something
#pragma data(data62)
__export uint8_t foo = 1;
__export uint8_t bar = 1;


// put vector table functions on fixed bank since the banked memory is not configured yet
#pragma code(code63)
#pragma data(data63)


__asm nmi_handler {
    rti
}

__asm irq_handler {
    rti
}


#include "via.h"
#include "imul.h"

int main(void) {
    // why 254? apparently 255 (which is the same as 127, but they recommend setting the MSB)
    // is always in the fixed section, and this is the bank immediately preceding.
    via.changeRomBank(254);
    imulInit();

    *(volatile uint8_t*) 0x2008 = 0xaa;


    int16_t baz = mulAsm((*(volatile uint8_t*) &foo), (*(volatile uint8_t*) &bar));
    *(volatile uint16_t*) 0x2008 = baz;


    // for (int i = 0; i < 256; i++) {
    //     for (int j = 0; j < 256; j++) {
    //         int16_t baz = mulAsm(i, j);
    //         if (baz != ((uint16_t)i * (uint16_t)j)) {
    //             *(volatile int16_t*) 0x2008 = 0xdead;
    //             *(volatile int8_t*) 0x2008 = i;
    //             *(volatile int8_t*) 0x2008 = j;
    //             *(volatile int16_t*) 0x2008 = baz;
    //         }
    //     }
    // }

    *(volatile uint8_t*) 0x2008 = 0xbb;

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
