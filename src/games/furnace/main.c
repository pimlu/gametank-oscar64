#include "system/boot.h"

#include <stdint.h>

#include "system/acp.h"
#include "system/bcr.h"
#include "system/interrupts.h"
#include "system/scr.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// the bank boot.c selected for $8000-$BFFF
#define GAME_ROM_BANK 126

// phase increments for C4..C5 at 13983 Hz: f * 65536 / 13982.6
static const uint16_t scale[8] = { 1226, 1376, 1545, 1637, 1837, 2062, 2315, 2452 };

#define FRAMES_PER_NOTE 30

void game_start(void) {
    // boot.c's last blit left the blitter IRQ asserted, and WAI returns immediately while it is
    bcr_reset_irq();

    acp_init(ACP_RATE_13983, GAME_ROM_BANK);

    uint8_t note = 0;
    uint8_t frames = 0;

    for (;;) {
        if (frames == 0) {
            acp_set_pitch(scale[note]);
            note = (note + 1) & 7;
            frames = FRAMES_PER_NOTE;
        }
        frames--;

        scr_set_enable_vblank_nmi(true);
        wait_for_interrupt();
        scr_set_enable_vblank_nmi(false);
    }
}
