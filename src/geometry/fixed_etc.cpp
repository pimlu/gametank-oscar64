#include "fixed_etc.h"

#include "system/imul.h"

// #include <limits>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


// stuff that uses fixed integer sizes and doesn't go in the template

namespace geometry {

GeoF mulRatio(GeoF x, GeoF num, UnitF den, bool &overflow) {
    *(volatile uint16_t*) 0x2008 = 0x1234;
    return GeoF(0.0);
    // int16_t xD = x.getRaw();
    // int16_t numD = num.getRaw();
    // uint16_t denD = den.getRaw();

    
    // int64_t firstProd = imul32To64(xD, numD);
    // int64_t prod = imul32To64(firstProd, denD);
    // int64_t res = prod >> (8 + 16);
    // if (res < std::numeric_limits<int16_t>::min() || res > std::numeric_limits<int16_t>::max()) {
    //     overflow = true;
    // }

    // return GeoF::fromRaw(res);
}


int8_t roundGeoF(GeoF x) {
    int16_t raw = x.getRaw();
    int8_t res = raw >> 8;
    int8_t rem = raw & 0xff;
    // sign bit of rem is the MSB
    if (rem < 0) {
        res++;
    }
    return res;
}


UnitF scaleByUint8(UnitF val, uint8_t scale) {
    uint32_t res = mul16To32(val.getRaw(), scale);
    return UnitF::fromRaw(res >> 8);
}

// GeoF scaleByUint8(GeoF val, uint8_t scale) {
//     int32_t res = imul16To32(val.getRaw(), scale);
//     return GeoF::fromRaw(res >> 8);
// }



}
