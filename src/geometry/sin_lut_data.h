#pragma once

#include "lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

extern const geometry::StandardLut sinLut;

#pragma compile("sin_lut_data.cpp")
