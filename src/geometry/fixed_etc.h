#pragma once

#include "types.h"
#include "geof.h"
#include "unitf.h"

#include "system/imul.h"

#include "geometry/types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


namespace geometry {

GeoF mulRatio(GeoF x, GeoF num, UnitF den, bool &overflow);
int8_t roundGeoF(GeoF x);

UnitF scaleByUint8(UnitF val, uint8_t scale);

// GeoF scaleByUint8(GeoF val, uint8_t scale);



}

#pragma compile("fixed_etc.cpp")
