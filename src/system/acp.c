#include "acp.h"

#include "scr.h"
#include "via.h"

// ---------------------------------------------------------------------------------------------
// Firmware. Runs on the ACP, never call any of this from the main CPU.
//
// No C in here: the oscar64 runtime expects its own zero page registers and stack. And keep the
// asm optimizer away from it (it can't know that the main CPU reads and writes this memory).
// oscar64's assembler only knows NMOS 6502 opcodes; 65C02 ones need `byt`.
// ---------------------------------------------------------------------------------------------

// ACP zero page, seen from the ACP / from the main CPU
#define ACP_ZP_PHASE 0x00 // 16 bit phase accumulator
#define ACP_ZP_INC   0x02 // 16 bit phase increment, written by the main CPU
#define ACP_MAIN_ZP  ((volatile uint8_t *) 0x3000)

// Any ACP write to $8000+ goes to the DAC buffer register, but it *also* still lands in RAM at
// (address & $0FFF). So $8000 would trash zero page $00. Use the mirror of the bottom of the
// stack page, which the stack (starting at $01FF) never reaches.
#define ACP_DAC 0x8100

#pragma code(acpcode)
#pragma data(acpcode)
#pragma optimize(push)
#pragma optimize(noasm)

__asm acp_fw_reset {
    ldx #$ff
    txs
    lda #0
    sta ACP_ZP_PHASE
    sta ACP_ZP_PHASE + 1
    cli
idle:
    byt 0xcb // wai
    jmp idle
}

// Timer IRQ = one sample. The hardware copies the DAC buffer register into the DAC at the *next*
// IRQ, so output timing doesn't depend on how long this takes.
__asm acp_fw_irq {
    clc
    lda ACP_ZP_PHASE
    adc ACP_ZP_INC
    sta ACP_ZP_PHASE
    lda ACP_ZP_PHASE + 1
    adc ACP_ZP_INC + 1
    sta ACP_ZP_PHASE + 1
    // sawtooth at 1/4 of full scale, centered on $80
    lsr
    lsr
    clc
    adc #$60
    sta ACP_DAC
    rti
}

__asm acp_fw_nmi {
    rti
}

#pragma data(acpvec)

__export struct AcpVectors {
    void *nmi, *reset, *irq;
} acp_fw_vectors = { acp_fw_nmi, acp_fw_reset, acp_fw_irq };

#pragma optimize(pop)

// ---------------------------------------------------------------------------------------------
// Main CPU side
// ---------------------------------------------------------------------------------------------

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void acp_init(uint8_t rate, uint8_t restore_bank) {
    // stop the ACP while its RAM changes under it
    scr_reg_audio_cfg_write(0);

    via_change_rom_bank(ACP_FW_ROM_BANK);
    const uint8_t *src = (const uint8_t *) 0x8200;
    volatile uint8_t *dst = (volatile uint8_t *) 0x3200;
    for (uint16_t i = 0; i < 0x0E00; i++) {
        dst[i] = src[i];
    }
    via_change_rom_bank(restore_bank);

    ACP_MAIN_ZP[ACP_ZP_INC] = 0;
    ACP_MAIN_ZP[ACP_ZP_INC + 1] = 0;

    scr_reg_audio_rst_write(0);
    scr_reg_audio_cfg_write(rate | 0x80);
}

void acp_set_pitch(uint16_t inc) {
    // Two writes, so the ACP can see a half-updated value for one sample. Inaudible for a test
    // tone; the real driver won't share multi-byte values this way.
    ACP_MAIN_ZP[ACP_ZP_INC] = (uint8_t) inc;
    ACP_MAIN_ZP[ACP_ZP_INC + 1] = (uint8_t) (inc >> 8);
}
