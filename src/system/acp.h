#pragma once

// Audio coprocessor (ACP): a second 65C02 with 4K of RAM, a sample-rate timer on its IRQ and
// an 8-bit DAC. https://wiki.gametank.zone/doku.php?id=hardware:audio
//
// The ACP sees its RAM at $0000-$0FFF, mirrored through its whole address space (so its
// vectors at $FFFA live at RAM offset $0FFA). We see the same RAM at $3000-$3FFF.
//
// The firmware is stored in its own ROM bank (oscar64 bank 61 = GameTank bank 125) but linked
// for $3000-$3FFF. Thanks to the mirroring that address works on both sides: on the ACP $3xxx
// wraps to $0xxx, and for us it is where the byte lives after acp_init() uploaded it. So
// firmware symbols can be used as plain pointers from C. The exception is the ACP's zero page:
// the firmware has to use $00xx operands to get zero page addressing, see ACP_ZP_* in acp.c.

#include <stdint.h>

#include "types.h"

#define ACP_FW_ROM_BANK 125

#pragma section( acpcode, 0 )
#pragma section( acpvec, 0 )
// first two pages are the ACP's zero page and stack, not part of the image
#pragma region( acpcode, 0x8200, 0x8FFA, , 61, { acpcode }, 0x3200 )
#pragma region( acpvec,  0x8FFA, 0x9000, , 61, { acpvec },  0x3FFA )

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Sample rates: value for the rate register ($2006), ACP cycles available per sample
#define ACP_RATE_13983 0xFF // 1024 cycles
#define ACP_RATE_22233 0xD0 //  644 cycles
#define ACP_RATE_31960 0xB7 //  448 cycles

// Upload the firmware and start it. Leaves ROM bank `restore_bank` selected.
void acp_init(uint8_t rate, uint8_t restore_bank);

// Test tone firmware: phase increment per sample, 65536 = one wave cycle. 0 is silence.
void acp_set_pitch(uint16_t inc);

#pragma compile("acp.c")
