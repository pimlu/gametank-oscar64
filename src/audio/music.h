#pragma once

// Music player: streams a GTS song (made by music/vgm2gts.py) from ROM to the synth
// running on the audio coprocessor.
//
// The ACP is a second 65C02 with 4K of RAM, a sample-rate timer on its IRQ and an 8-bit DAC
// (https://wiki.gametank.zone/doku.php?id=hardware:audio). It sees its RAM at $0000-$0FFF,
// mirrored through its whole address space; we see the same RAM at $3000-$3FFF. So firmware
// linked for $3xxx works on both sides: on the ACP the address wraps to $0xxx, and for us it is
// where that byte lives once uploaded.
//
// The ACP keeps musical time by counting its own samples, so none of this cares how long a
// frame takes. It plays from a 256 byte ring buffer; all the main CPU has to do is call
// music_pump() often enough that the ring doesn't run dry (if it does, the music drags instead
// of glitching). A full ring is about 0.4 s of a busy song.
//
// The song lives in ROM banks 0.. (layout: music/gts.py, song_banks); it loops if it says so. These functions switch
// ROM banks and back, so they have to stay in the fixed bank (hence __noinline: their callers
// may well live in a banked one). So do the interrupt handlers.

#include <stdbool.h>
#include <stdint.h>

#include "acp_fw.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Upload firmware and waves, start playing. False if there is no song in the ROM.
// Leaves ROM bank `restore_bank` selected.
__noinline bool music_init(uint8_t restore_bank);

// Top up the ring buffer. Call once per frame.
__noinline void music_pump(uint8_t restore_bank);

#pragma compile("music.c")
