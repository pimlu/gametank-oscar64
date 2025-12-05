#include "sin_lut.h"

#include "sin_lut_data.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace geometry {


static constexpr double SIN_START = 0.0;
static constexpr double SIN_STEP = 32.0 / 256;


static constexpr StandardLutConfig sinLutConfig = {
    .start = GeoF(SIN_START),
    .step = GeoF(SIN_STEP),

    // 1 is slightly outside the range lol
    .last = UnitF::fromRaw(0xffff),

    .loMiss = UnitF(0.0),
    .hiMiss = UnitF::fromRaw(0xffff),
};

UnitF sinLookup(GeoF x) {
    return sinLut.lookupOrConst(sinLutConfig, x);
}

UnitF sinWrap(GeoF x)  {
    if (x <= 32.0) {
        return sinLookup(x);
    } else {
        return sinLookup(GeoF(64.0) - x);
    }
}
iUnitF sin(GeoF x) {
    if (x >= 0.0) {
        iUnitF result;
        result.val = sinWrap(x);
        result.negated = false;
        return result;
    } else {
        iUnitF result;
        result.val = sinWrap(-x);
        result.negated = true;
        return result;
    }
}
iUnitF cos(GeoF x) {
    // prevent overflow by looping back around
    if (x <= -32.0) {
        return sin(GeoF(32.0 - 128.0) - x);
    }
    return sin(GeoF(32.0) - x);
}

}

