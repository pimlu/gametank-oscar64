#pragma once

#include "lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

extern const struct lut recip_lut;

#pragma compile("recip_lut_data.c")

