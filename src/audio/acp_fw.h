#pragma once

// ROM placement of the ACP firmware (acp_fw.c); see music.h for the address trick.

// stored in oscar64 bank 61 = GameTank ROM bank 125, linked for where music_init() uploads it
#define ACP_FW_ROM_BANK  125
#define ACP_FW_CODE      0x3200
#define ACP_FW_CODE_SIZE 0x0500
#define ACP_FW_VECTORS   0x3FFA

#pragma section( acpcode, 0 )
#pragma section( acpvec, 0 )
#pragma region( acpcode, 0x8200, 0x8700, , 61, { acpcode }, 0x3200 )
#pragma region( acpvec,  0x8FFA, 0x9000, , 61, { acpvec },  0x3FFA )

#pragma compile("acp_fw.c")
