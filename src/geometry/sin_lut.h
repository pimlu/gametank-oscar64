#pragma once

#include "lut.h"
#include "types.h"

#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace geometry {

// calculates sin(x/32 * Math.PI/2)
// a half range AKA [-64,64) of GeoF becomes one sin loop
iUnitF sin(GeoF x);
iUnitF cos(GeoF x);

}

#pragma compile("sin_lut.cpp")
