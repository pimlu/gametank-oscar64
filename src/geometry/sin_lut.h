#pragma once

#include "sin_lut_data.h"
#include "lut.h"


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace geometry {




constexpr double SIN_START = 0.0;
constexpr double SIN_STEP = 32.0 / 256;

constexpr uint16_t SIN_START_RAW = GeoF(SIN_START).getRaw();
constexpr uint16_t SIN_STEP_RAW = GeoF(SIN_STEP).getRaw();


// 1 is slightly outside the range lol
constexpr UnitF almostOne = UnitF::fromRaw(0xffff);

constexpr Lut<GeoF, UnitF> lut(sinLutData);

}
