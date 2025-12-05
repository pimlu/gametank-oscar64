#include "types.h"


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


namespace geometry {

GeoF scaleGeoF(GeoF val, UnitF scale) {
    int16_t raw = val.getRaw();
    bool negated = raw < 0;
    uint16_t posRaw = negated ? -(int32_t) raw : raw;
    uint32_t product = mul16To32(posRaw, scale.getRaw());
    GeoF result = GeoF::fromRaw(product >> 16);
    return negated ? -result : result;
}

}

