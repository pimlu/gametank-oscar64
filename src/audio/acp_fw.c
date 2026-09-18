// ACP firmware: six channel wavetable/noise synth that plays a GTS stream.
//
// The spec is music/gts.py (format of the stream, and a bit-exact model of what this code
// computes). Change them together; the tests in the gametank-tracker repo run this firmware in
// an emulator and compare the two sample for sample.
//
// Everything in this file runs on the ACP, never on the main CPU. It is stored in ROM bank 125
// and linked for $3200+, where music_init() uploads it. On the ACP that address wraps to $0200+
// (see music.h), and zero page has to be addressed as $00xx to get zero page opcodes. No C in
// here (the oscar64 runtime would want its own zero page), and no asm optimizer: it can't know
// which memory the IRQ handler, the main loop and the main CPU share.
// oscar64's assembler only knows NMOS opcodes and needs ';' between instructions on one line.
// Labels are all called L_something: a label that happens to have the name of a C global
// anywhere in the game (polyfish has a function tick()) silently assembles to a wrong branch.
//
// Structure:
//  - fw_irq runs once per sample: steps the six oscillators and sums their tables into the DAC.
//    It also counts samples into ticks. At a tick boundary it copies the "shadow" parameters
//    over the live ones in one go, *if* the main loop has them ready. Otherwise music time
//    stands still until it has (a starved stream makes the song drag instead of glitch).
//    Doing all of that in one sample period is already tight at 22 kHz, so phase resets
//    (SET_PHASE) happen one sample later.
//  - fw_reset/main loop: reads the commands of the next tick from the ring buffer into the
//    shadow parameters, renders the tables of channels whose wave or gain changed into that
//    channel's spare buffer, flags the tick as ready and waits for the IRQ to take it.

#include "acp_fw.h"

#pragma code(acpcode)
#pragma data(acpcode)
#pragma optimize(push)
#pragma optimize(noasm)

// --- zero page ---------------------------------------------------------------------------------
// live oscillator state, [6] each
#define Z_PH0      0x00 // phase, 5.16 fixed point: PH2 is the table index (kept masked to 0..31)
#define Z_PH1      0x06
#define Z_PH2      0x0c
#define Z_INC0     0x12 // phase increment per sample
#define Z_INC1     0x18
#define Z_INC2     0x1e
#define Z_WPTR     0x24 // [6] pointers to the table being played
// noise, [2] each (channels 4 and 5)
#define Z_NMODE    0x30
#define Z_NINC     0x32 // 16 bit words
#define Z_NACC     0x36
#define Z_LFSR     0x3a
// shadow parameters: what the live ones become at the next tick boundary. [6] each
#define Z_SH_INC0  0x40
#define Z_SH_INC1  0x46
#define Z_SH_INC2  0x4c
#define Z_SH_WLO   0x52 // low byte of WPTR
#define Z_SH_NMODE 0x58 // [4], [5] only
#define Z_SH_NINC0 0x5e
#define Z_SH_NINC1 0x64
#define Z_SH_PH2   0xb0 // SET_PHASE: new table index...
#define Z_SH_PHSET 0xb6 // ...if this is set
#define Z_SH_PHANY 0xbc // any of them is
// main loop state, [6] each
#define Z_GAIN0    0x6a
#define Z_GAIN1    0x70
#define Z_WAVE     0x76
#define Z_DIRTY    0x7c // table needs rendering
#define Z_CURLO    0x82 // low byte of the newest table (each channel has two, $20 apart)
// main loop scratch
#define Z_ROW      0x88 // [16] row[m] = round((m + 0.5) * gain / 256)
#define Z_SRC      0x98
#define Z_DST      0x9a
#define Z_CH       0x9c
#define Z_CMD      0x9d
#define Z_ACC      0x9e // word
#define Z_WAIT     0xae
// control
#define Z_READY    0xa0 // main loop -> irq: shadow parameters are complete
#define Z_PHPEND   0xbd // irq: phases still have to be set, on the sample after the tick boundary
#define Z_RD       0xa1 // ring read index. The only byte in here the main CPU reads.
#define Z_STALL    0xa3 // word: samples spent waiting for a late tick (should stay 0)
#define Z_DELAY    0xa5 // word: samples before the first tick (tests use this to align)
#define Z_STARTED  0xa7
#define Z_CNT_LO   0xa8 // samples until the next tick, as hi * 256 + (lo ? lo : 256)
#define Z_CNT_HI   0xa9
#define Z_FRAC     0xaa
#define Z_IRQ_A    0xab
#define Z_IRQ_Y    0xac
#define Z_DACDUMMY 0xad
// parameters, written by the main CPU. Not cleared by fw_reset.
#define P_TICK_N   0xf0 // word: samples per tick
#define P_TICK_F   0xf2 // + this / 256
#define P_WR       0xf3 // ring write index

// A write to $8000+ goes to the DAC buffer register but also still to RAM at (address & $0FFF).
// (No parentheses in operand macros: oscar64 would take them as an indirect addressing mode,
// and silently assemble garbage for instructions that don't have one.)
#define DAC 0x8000 + Z_DACDUMMY

#define RING  0x0700
// tables: channels 0-2 share page 1 with the stack ($01C0+), channels 3-5 are at $0800
#define WAVES 0x08c0

// indexed by channel
const unsigned char fw_tab_hi[6] = { 0x01, 0x01, 0x01, 0x08, 0x08, 0x08 };
const unsigned char fw_tab_lo[6] = { 0x00, 0x40, 0x80, 0x00, 0x40, 0x80 };

#define FW_OSC(ch) \
    clc ; \
    lda Z_PH0 + ch ; adc Z_INC0 + ch ; sta Z_PH0 + ch ; \
    lda Z_PH1 + ch ; adc Z_INC1 + ch ; sta Z_PH1 + ch ; \
    lda Z_PH2 + ch ; adc Z_INC2 + ch ; and #$1f ; sta Z_PH2 + ch ;

// n = ch - 4. In noise mode the table only has entries 0 and 1, picked by the LFSR's low bit.
#define FW_OSC_NOISE(ch, n) \
    lda Z_NMODE + n ; bne L_noise##n ; \
    FW_OSC(ch) \
    jmp L_done##n ; \
L_noise##n: \
    clc ; \
    lda Z_NACC + 2 * n ; adc Z_NINC + 2 * n ; sta Z_NACC + 2 * n ; \
    lda Z_NACC + 2 * n + 1 ; adc Z_NINC + 2 * n + 1 ; sta Z_NACC + 2 * n + 1 ; \
    bcc L_nostep##n ; \
    lsr Z_LFSR + 2 * n + 1 ; ror Z_LFSR + 2 * n ; bcc L_nostep##n ; \
    lda Z_LFSR + 2 * n + 1 ; eor #$b4 ; sta Z_LFSR + 2 * n + 1 ; \
L_nostep##n: \
    lda Z_LFSR + 2 * n ; and #1 ; sta Z_PH2 + ch ; \
L_done##n:

#define FW_COMMIT(ch) \
    lda Z_SH_INC0 + ch ; sta Z_INC0 + ch ; \
    lda Z_SH_INC1 + ch ; sta Z_INC1 + ch ; \
    lda Z_SH_INC2 + ch ; sta Z_INC2 + ch ; \
    lda Z_SH_WLO + ch ; sta Z_WPTR + 2 * ch ;

#define FW_SETPHASE(ch) \
    lda Z_SH_PHSET + ch ; beq L_nophase##ch ; \
    lda Z_SH_PH2 + ch ; sta Z_PH2 + ch ; \
    lda #0 ; sta Z_PH0 + ch ; sta Z_PH1 + ch ; sta Z_SH_PHSET + ch ; \
L_nophase##ch:

#define FW_COMMIT_NOISE(ch, n) \
    lda Z_SH_NMODE + ch ; sta Z_NMODE + n ; \
    lda Z_SH_NINC0 + ch ; sta Z_NINC + 2 * n ; \
    lda Z_SH_NINC1 + ch ; sta Z_NINC + 2 * n + 1 ;

// The tables are signed and the sum wraps; the converter guarantees the total fits a byte.
#define FW_MIX(ch) \
    clc ; ldy Z_PH2 + ch ; adc (Z_WPTR + 2 * ch),y ;

__asm fw_irq {
    sta Z_IRQ_A
    sty Z_IRQ_Y

    dec Z_CNT_LO
    bne L_nottick
    jmp L_lowzero
L_nottick:
    lda Z_PHPEND
    beq L_synth
    jmp L_setphases

L_synth:
    FW_OSC(0)
    FW_OSC(1)
    FW_OSC(2)
    FW_OSC(3)
    FW_OSC_NOISE(4, 0)
    FW_OSC_NOISE(5, 1)

    ldy Z_PH2
    lda (Z_WPTR),y
    FW_MIX(1)
    FW_MIX(2)
    FW_MIX(3)
    FW_MIX(4)
    FW_MIX(5)
    eor #$80
    sta DAC

    lda Z_IRQ_A
    ldy Z_IRQ_Y
    rti

    // out of line, to keep the branches in range
L_lowzero:
    lda Z_CNT_HI
    beq L_tick
    dec Z_CNT_HI
    jmp L_nottick

L_tick:
    lda Z_READY
    bne L_commit
    // the next tick isn't ready: try again at the next sample
    inc Z_CNT_LO
    lda Z_STARTED
    beq L_starting
    inc Z_STALL
    bne L_stalled
    inc Z_STALL + 1
L_stalled:
    jmp L_synth
L_starting:
    inc Z_DELAY
    bne L_delayed
    inc Z_DELAY + 1
L_delayed:
    jmp L_synth

L_commit:
    FW_COMMIT(0)
    FW_COMMIT(1)
    FW_COMMIT(2)
    FW_COMMIT(3)
    FW_COMMIT(4)
    FW_COMMIT(5)
    FW_COMMIT_NOISE(4, 0)
    FW_COMMIT_NOISE(5, 1)
    lda Z_SH_PHANY
    sta Z_PHPEND
    lda #0
    sta Z_SH_PHANY
    sta Z_READY
    lda #1
    sta Z_STARTED
    clc
    lda Z_FRAC
    adc P_TICK_F
    sta Z_FRAC
    lda P_TICK_N
    adc #0
    sta Z_CNT_LO
    lda P_TICK_N + 1
    sta Z_CNT_HI
    jmp L_synth

L_setphases:
    FW_SETPHASE(0)
    FW_SETPHASE(1)
    FW_SETPHASE(2)
    FW_SETPHASE(3)
    FW_SETPHASE(4)
    FW_SETPHASE(5)
    lda #0
    sta Z_PHPEND
    jmp L_synth
}

__asm fw_nmi {
    rti
}

// -> A. Blocks while the ring is empty. Keeps X.
__asm fw_getbyte {
L_wait:
    // the main CPU may be writing P_WR right now: only trust a value we read twice
    lda P_WR
    cmp P_WR
    bne L_wait
    cmp Z_RD
    beq L_wait
    ldy Z_RD
    lda RING,y
    inc Z_RD
    rts
}

// Render the table of channel X from its wave and gain (or noise mode) into the channel's spare
// buffer, and make that the one to play from the next tick on. Keeps X.
__asm fw_render {
    stx Z_CH

    // row[m] = (gain / 2 + m * gain + $80) >> 8
    lda Z_GAIN1,x
    lsr
    sta Z_ACC + 1
    lda Z_GAIN0,x
    ror
    sta Z_ACC
    ldy #0
L_row:
    clc
    lda Z_ACC
    adc #$80
    lda Z_ACC + 1
    adc #0
    sta $0000 + Z_ROW,y
    clc
    lda Z_ACC
    adc Z_GAIN0,x
    sta Z_ACC
    lda Z_ACC + 1
    adc Z_GAIN1,x
    sta Z_ACC + 1
    iny
    cpy #16
    bne L_row

    lda Z_CURLO,x
    eor #$20
    sta Z_CURLO,x
    sta Z_SH_WLO,x
    sta Z_DST
    lda fw_tab_hi,x
    sta Z_DST + 1

    lda Z_SH_NMODE,x
    beq L_wavetable
    ldy #0
    sec
    lda #0
    sbc Z_ROW + 15
    sta (Z_DST),y
    iny
    lda Z_ROW + 15
    sta (Z_DST),y
    jmp L_done

L_wavetable:
    // src = WAVES + wave * 32
    lda #0
    sta Z_SRC + 1
    lda Z_WAVE,x
    asl
    rol Z_SRC + 1
    asl
    rol Z_SRC + 1
    asl
    rol Z_SRC + 1
    asl
    rol Z_SRC + 1
    asl
    rol Z_SRC + 1
    clc
    adc #<WAVES
    sta Z_SRC
    lda Z_SRC + 1
    adc #>WAVES
    sta Z_SRC + 1

    ldy #31
L_entry:
    lda (Z_SRC),y
    cmp #16
    bcc L_negative
    sbc #16
    tax
    lda Z_ROW,x
    sta (Z_DST),y
    jmp L_next
L_negative:
    eor #$0f
    tax
    sec
    lda #0
    sbc Z_ROW,x
    sta (Z_DST),y
L_next:
    dey
    bpl L_entry

L_done:
    ldx Z_CH
    lda #0
    sta Z_DIRTY,x
    rts
}

__asm fw_reset {
    ldx #$ff
    txs

    lda #0
    ldx #$ef
L_clearzp:
    sta $00,x
    dex
    bne L_clearzp
    sta $00

    ldx #$bf
L_cleartables:
    sta $0100,x
    sta $0800,x
    dex
    cpx #$ff
    bne L_cleartables

    ldx #5
    ldy #10
L_pointers:
    lda fw_tab_lo,x
    sta Z_CURLO,x
    sta Z_SH_WLO,x
    sta $0000 + Z_WPTR,y
    lda fw_tab_hi,x
    sta $0001 + Z_WPTR,y
    dey
    dey
    dex
    bpl L_pointers

    lda #1
    sta Z_LFSR
    sta Z_LFSR + 2
    sta Z_CNT_LO
    cli

L_nexttick:
    // wait until the IRQ has taken everything of the previous tick
    lda Z_READY
    ora Z_PHPEND
    bne L_nexttick

L_command:
    jsr fw_getbyte
    cmp #$fd
    bcs L_special
    sta Z_CMD
    and #7
    tax
    lda Z_CMD
    lsr
    lsr
    lsr
    beq L_cmd_inc
    cmp #1
    beq L_cmd_gain
    cmp #2
    beq L_cmd_wave
    cmp #3
    beq L_cmd_noise

    // SET_PHASE
    jsr fw_getbyte
    sta Z_SH_PH2,x
    lda #1
    sta Z_SH_PHSET,x
    sta Z_SH_PHANY
    jmp L_command

L_cmd_noise:
    jsr fw_getbyte
    sta Z_SH_NINC0,x
    jsr fw_getbyte
    sta Z_SH_NINC1,x
    ora Z_SH_NINC0,x
    beq L_nmode
    lda #1
L_nmode:
    sta Z_SH_NMODE,x
    jmp L_dirty

L_cmd_inc:
    jsr fw_getbyte
    sta Z_SH_INC0,x
    jsr fw_getbyte
    sta Z_SH_INC1,x
    jsr fw_getbyte
    sta Z_SH_INC2,x
    jmp L_command

L_cmd_gain:
    jsr fw_getbyte
    sta Z_GAIN0,x
    jsr fw_getbyte
    sta Z_GAIN1,x
    jmp L_dirty

L_cmd_wave:
    jsr fw_getbyte
    sta Z_WAVE,x
L_dirty:
    lda #1
    sta Z_DIRTY,x
    jmp L_command

L_special:
    beq L_end
    cmp #$ff
    beq L_endtick
    // FE n: end of tick, then n ticks without changes
    jsr fw_getbyte
    sta Z_WAIT

L_endtick:
    ldx #5
L_render:
    lda Z_DIRTY,x
    beq L_clean
    jsr fw_render
L_clean:
    dex
    bpl L_render

    lda #1
    sta Z_READY
L_idle:
    lda Z_WAIT
    bne L_taken
    jmp L_nexttick
L_taken:
    lda Z_READY
    bne L_taken
    dec Z_WAIT
    lda #1
    sta Z_READY
    jmp L_idle

L_end:
    // silence, then keep the (now empty) ticks coming so this doesn't count as stalling
    ldx #5
    lda #0
L_silence:
    sta Z_GAIN0,x
    sta Z_GAIN1,x
    sta Z_SH_NMODE,x
    dex
    bpl L_silence
    ldx #5
L_render_silence:
    jsr fw_render
    dex
    bpl L_render_silence
L_forever:
    lda #1
    sta Z_READY
L_ended:
    lda Z_READY
    bne L_ended
    jmp L_forever
}

#pragma data(acpvec)

__export struct FwVectors {
    void *nmi, *reset, *irq;
} fw_vectors = { fw_nmi, fw_reset, fw_irq };

#pragma optimize(pop)
