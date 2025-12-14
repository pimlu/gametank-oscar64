#pragma once

#include "lut.h"
#include "geof.h"
#include "unitf.h"
#include "types.h"

#include <stdint.h>
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// calculates 1/x for x in [1.0, 17.0)
unitf_t geometry_recip(geof_t x);

#pragma compile("recip_lut.c")
