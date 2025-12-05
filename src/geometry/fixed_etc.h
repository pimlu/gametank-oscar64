#pragma once

#include "types.h"
#include "geof.h"
#include "unitf.h"

#include "system/imul.h"
#include <stdint.h>
#include <stdbool.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

geof_t geometry_mul_ratio(geof_t x, geof_t num, unitf_t den, bool *overflow);
int8_t geometry_round_geof(geof_t x);

#pragma compile("fixed_etc.c")
