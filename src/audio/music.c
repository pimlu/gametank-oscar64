#include "music.h"

#include "system/scr.h"
#include "system/via.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// song header at $8000 of ROM bank 0, see music/gts.py
#define SONG_MAGIC0     'G'
#define SONG_MAGIC1     'T'
#define SONG_MAGIC2     'S'
#define SONG_MAGIC3     '1'
#define SONG_RATE       4
#define SONG_NUM_WAVES  5
#define SONG_TICK_N     6
#define SONG_TICK_F     8
#define SONG_LOOP_BANK  9   // 0: play once
#define SONG_LOOP_ADDR  10  // where in that bank's $8000-$BFFF to go on from
#define SONG_WAVES      16
#define SONG_FIRST_STREAM_BANK 1

// volatile: what is at these addresses changes whenever we switch ROM banks
#define ROM_BASE ((const volatile uint8_t *) 0x8000)

static inline uint8_t rom(uint16_t offset) {
    return ROM_BASE[offset];
}

// the firmware's memory, from our side (acp_fw.c has the ACP-side definitions)
#define ARAM           ((volatile uint8_t *) 0x3000)
#define ARAM_RD        0x0a1
#define ARAM_TICK_N    0x0f0
#define ARAM_TICK_F    0x0f2
#define ARAM_WR        0x0f3
#define ARAM_RING      0x700
#define ARAM_WAVES     0x8c0

static uint8_t stream_bank;       // 0: nothing (left) to play
static const volatile uint8_t *stream_ptr;
static uint16_t stream_left;      // bytes left in this bank
static uint8_t ring_wr;
static uint8_t loop_bank;
static uint16_t loop_addr;

static void select_bank(uint8_t bank) {
    via_change_rom_bank(bank | 0x80);
}

static void open_stream_bank(void) {
    select_bank(stream_bank);
    stream_left = rom(0) | ((uint16_t) rom(1) << 8);
    stream_ptr = ROM_BASE + 2;
    // 0 ends the song; so does running into an erased bank
    if (stream_left == 0 || stream_left > 0x3FFE) {
        stream_bank = 0;
        stream_left = 0;
    }
}

// the current bank is used up: on to the next one, or back to the loop point after the last
static void next_stream_bank(void) {
    stream_bank++;
    open_stream_bank();
    if (!stream_bank && loop_bank) {
        stream_bank = loop_bank;
        open_stream_bank();
        uint16_t skip = loop_addr - 0x8002;
        if (skip < stream_left) {
            stream_ptr += skip;
            stream_left -= skip;
        } else {
            stream_bank = 0;
        }
    }
}

static void copy_to_aram(uint16_t dst, const volatile uint8_t *src, uint16_t n) {
    volatile uint8_t *d = ARAM + dst;
    while (n) {
        *d++ = *src++;
        n--;
    }
}

__noinline bool music_init(uint8_t restore_bank) {
    // stop the ACP while its RAM changes under it
    scr_reg_audio_cfg_write(0);
    stream_bank = 0;

    select_bank(0);
    if (rom(0) != SONG_MAGIC0 || rom(1) != SONG_MAGIC1 || rom(2) != SONG_MAGIC2 || rom(3) != SONG_MAGIC3) {
        select_bank(restore_bank);
        return false;
    }
    uint8_t rate = rom(SONG_RATE);
    loop_bank = rom(SONG_LOOP_BANK);
    loop_addr = rom(SONG_LOOP_ADDR) | ((uint16_t) rom(SONG_LOOP_ADDR + 1) << 8);
    ARAM[ARAM_TICK_N] = rom(SONG_TICK_N);
    ARAM[ARAM_TICK_N + 1] = rom(SONG_TICK_N + 1);
    ARAM[ARAM_TICK_F] = rom(SONG_TICK_F);
    copy_to_aram(ARAM_WAVES, ROM_BASE + SONG_WAVES, (uint16_t) rom(SONG_NUM_WAVES) << 5);

    select_bank(ACP_FW_ROM_BANK);
    copy_to_aram(ACP_FW_CODE - 0x3000, (const volatile uint8_t *) (ACP_FW_CODE + 0x5000), ACP_FW_CODE_SIZE);
    copy_to_aram(ACP_FW_VECTORS - 0x3000, (const volatile uint8_t *) (ACP_FW_VECTORS + 0x5000), 6);

    // The firmware zeroes its read index too, but we may look at it before it got that far
    ARAM[ARAM_RD] = 0;
    ARAM[ARAM_WR] = 0;
    ring_wr = 0;

    stream_bank = SONG_FIRST_STREAM_BANK;
    open_stream_bank();
    music_pump(restore_bank);

    scr_reg_audio_rst_write(0);
    scr_reg_audio_cfg_write(rate | 0x80);
    return true;
}

__noinline void music_pump(uint8_t restore_bank) {
    if (!stream_bank) {
        return;
    }
    select_bank(stream_bank);

    // The ACP may be writing its read index right now: only trust a value we read twice
    uint8_t rd;
    do {
        rd = ARAM[ARAM_RD];
    } while (rd != ARAM[ARAM_RD]);

    // one slot stays free so that full and empty look different
    while ((uint8_t) (ring_wr + 1) != rd) {
        ARAM[ARAM_RING + ring_wr] = *stream_ptr++;
        ring_wr++;
        stream_left--;
        if (!stream_left) {
            next_stream_bank();
            if (!stream_bank) {
                break;
            }
        }
    }
    // publish only complete bytes: the ACP reads up to, not including, this index
    ARAM[ARAM_WR] = ring_wr;

    select_bank(restore_bank);
}
