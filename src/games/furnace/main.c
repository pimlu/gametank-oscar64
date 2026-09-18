#include "system/boot.h"

#include <stdint.h>

#include "audio/music.h"
#include "system/bcr.h"
#include "system/interrupts.h"
#include "system/scr.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Plays the song that ./build_with_song.sh put into the ROM, and does nothing else.

// the bank boot.c selected for $8000-$BFFF
#define GAME_ROM_BANK 126

void game_start(void) {
    // boot.c's last blit left the blitter IRQ asserted, and WAI returns immediately while it is
    bcr_reset_irq();

    music_init(GAME_ROM_BANK);

    for (;;) {
        music_pump(GAME_ROM_BANK);

        scr_set_enable_vblank_nmi(true);
        wait_for_interrupt();
        scr_set_enable_vblank_nmi(false);
    }
}
