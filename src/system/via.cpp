#include "via.h"

#include "types.h"



#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)
Via via;


// https://wiki.gametank.zone/doku.php?id=hardware:flashcarts
void Via::changeRomBank(uint8_t bankNum) {
    ddra.write(0b00000111);
    iora.write(0);

    // this is like a sequence where it turns the clock (the LSB) on and
    // off while sending data
    for (int i = 7; i >= 0; i--) {

        iora.write(bits::getBit(bankNum, i) ? 2 : 0);
        iora.setBit(0, true);
    }

    iora.setBit(2, true);
    iora.write(0);
}
void Via::profilerStart(uint8_t id) {
    iorb.write(0x80);
    iorb.write(id);
}
void Via::profilerEnd(uint8_t id) {
    iorb.write(0x80);
    iorb.write(id | 0x40);
}
