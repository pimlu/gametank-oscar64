#pragma once

#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace geometry {


template<class Inp, class Out>
struct LutConfig {
    Inp start;
    Inp step;

    Out last;

    Out loMiss;
    Out hiMiss;
};

template<class Inp, class Out>
struct Lut {
    uint8_t lsb[256];
    uint8_t msb[256];

    constexpr Lut(const Out input[256]) {
        for (uint16_t i = 0; i < 256; i++) {
            lsb[i] = input[i].lsb();
            msb[i] = input[i].msb();
        }        
    }

    constexpr Out lookup(uint8_t i) const {
        return Out::fromRaw(lsb[i], msb[i]);
    }


    Out lookupOrConst(const LutConfig<Inp, Out> config, Inp x) const {
        // ...
        Out foo;
        return foo;
    }
};

}