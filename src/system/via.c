#include "via.h"

#include "types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

DEFINE_MEMREG(via_reg_iorb, 0x2800);
DEFINE_MEMREG(via_reg_iora, 0x2801);
DEFINE_MEMREG(via_reg_ddrb, 0x2802);
DEFINE_MEMREG(via_reg_ddra, 0x2803);
DEFINE_MEMREG(via_reg_t1cl, 0x2804);
DEFINE_MEMREG(via_reg_t1ch, 0x2805);
DEFINE_MEMREG(via_reg_t2cl, 0x2806);
DEFINE_MEMREG(via_reg_t2ch, 0x2807);
DEFINE_MEMREG(via_reg_sr, 0x2808);
DEFINE_MEMREG(via_reg_acr, 0x2809);
DEFINE_MEMREG(via_reg_pcr, 0x280a);
DEFINE_MEMREG(via_reg_ifr, 0x280b);
DEFINE_MEMREG(via_reg_era, 0x280c);
DEFINE_MEMREG(via_reg_iora_nh, 0x280d);

// https://wiki.gametank.zone/doku.php?id=hardware:flashcarts
void via_change_rom_bank(uint8_t bank_num) {
    via_reg_ddra_write(0b00000111);
    via_reg_iora_write(0);

    // this is like a sequence where it turns the clock (the LSB) on and
    // off while sending data
    for (int i = 7; i >= 0; i--) {
        via_reg_iora_write(bits_get(bank_num, i) ? 2 : 0);
        via_reg_iora_set_bit(0, true);
    }

    via_reg_iora_set_bit(2, true);
    via_reg_iora_write(0);
}

void via_profiler_start(uint8_t id) {
    via_reg_iorb_write(0x80);
    via_reg_iorb_write(id);
}

void via_profiler_end(uint8_t id) {
    via_reg_iorb_write(0x80);
    via_reg_iorb_write(id | 0x40);
}
