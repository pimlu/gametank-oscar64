#pragma once


#pragma section( startup, 0 )
#pragma section( boot, 0 )
#pragma section( random, 0 )

// Note: Zero page 0x00-0x2f is reserved by oscar64 for internal registers
// (IP, ACCU, SP, FP, tmp, etc). User zeropage starts at 0x30+.
#pragma region( zeropage, 0x030, 0x0100, , , { zeropage } )
#pragma region( main, 0x0100, 0x2000, , , { bss, random, heap, stack } )
#pragma stacksize(512)
#pragma heapsize(0)


// banked ROM (GameTank 126) - appears at $8000-$BFFF when bank 126 is selected
#pragma section( code62, 0 )
#pragma section( data62, 0 )
#pragma region( rom62, 0x8000, 0xC000, , 62, { code62, data62 } )

// fixed ROM (GameTank 127) - always visible at $C000-$FFFF
#pragma section( code63, 0 )
#pragma section( data63, 0 )
// startup code at the START of fixed bank (known address 0xC000)
// NOTE: region must be named exactly "startup" for oscar64 to use it
#pragma region( startup, 0xC000, 0xC100, , 63, { startup } )
// fixed code/data area after startup
#pragma region( rom63, 0xC100, 0xFFFA, , 63, { code, data, code63, data63 } )
// 6-byte vector table at the very top ($FFFA-$FFFF)
#pragma region( boot, 0xFFFA, 0x10000, , 63, { boot } )


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void game_start();

#pragma compile("boot.c")
