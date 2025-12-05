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

// calculates sin(x/32 * Math.PI/2)
// a half range AKA [-64,64) of GeoF becomes one sin loop
iunitf_t geometry_sin(geof_t x);
iunitf_t geometry_cos(geof_t x);

#pragma compile("sin_lut.c")
