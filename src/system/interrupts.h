#pragma once

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

void wait_for_interrupt(void);

#pragma compile("interrupts.c")
