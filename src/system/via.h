#pragma once

#include <stdint.h>

#include "types.h"
#include "reg_macros.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

DECLARE_MEMREG(via_reg_iorb); // output register b
DECLARE_MEMREG(via_reg_iora); // output register a
DECLARE_MEMREG(via_reg_ddrb);
DECLARE_MEMREG(via_reg_ddra);
DECLARE_MEMREG(via_reg_t1cl);
DECLARE_MEMREG(via_reg_t1ch);
DECLARE_MEMREG(via_reg_t2cl);
DECLARE_MEMREG(via_reg_t2ch);
DECLARE_MEMREG(via_reg_sr);
DECLARE_MEMREG(via_reg_acr);
DECLARE_MEMREG(via_reg_pcr);
DECLARE_MEMREG(via_reg_ifr);
DECLARE_MEMREG(via_reg_era);
DECLARE_MEMREG(via_reg_iora_nh);

void via_change_rom_bank(uint8_t bank_num);
void via_profiler_start(uint8_t id);
void via_profiler_end(uint8_t id);

#pragma compile("via.c")
