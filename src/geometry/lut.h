#pragma once

#include <stdint.h>

#include "fixed_etc.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace geometry {

constexpr uint8_t bitShift(int16_t x, uint8_t shift = 0) {
    return x <= 1 ? shift : bitShift(x >> 1, shift + 1);
}

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

    constexpr Out lookup(uint8_t i) const {
        *(volatile uint8_t*) 0x2008 = i;
        return Out::fromRaw(lsb[i], msb[i]);
    }

    Out lerp(const LutConfig<Inp, Out> config, Out a, Out b, uint8_t frac) const {
        int16_t stepRaw = config.step.getRaw();
        uint8_t stepBits = bitShift(stepRaw);
        uint8_t remBits = 8 - stepBits;
        uint8_t invFrac = (1 << stepBits) - frac;

        // this only exists for UnitF right now...
        Out aPart = scaleByUint8(a, invFrac << remBits);
        Out bPart = scaleByUint8(b, frac << remBits);
        return aPart + bPart;
    }

    Out lookupOrConst(const LutConfig<Inp, Out> config, Inp x) const {
        if (x < config.start) return config.loMiss;
        Inp end = config.start + config.step.smallIntMult(256);
        if (end != config.start) {
            if (x == end) return config.last;
            else if (x > end) return config.hiMiss;
        }
        Inp shifted = x - config.start;
        int16_t shiftedBits = shifted.getRaw();
        int16_t stepRaw = config.step.getRaw();
        uint8_t stepBits = bitShift(stepRaw);
        uint8_t stepMask = stepRaw - 1;
        uint8_t idx = shiftedBits >> stepBits;
        uint8_t frac = shiftedBits & stepMask;

        if (!frac) {
            return lookup(idx);
        }
        Out next = config.last;
        if (idx != 255) {
            next = lookup(idx+1);
        }
        Out res = lerp(config, lookup(idx), next, frac);

        return res;
    }
};

typedef Lut<GeoF, UnitF> StandardLut;
typedef LutConfig<GeoF, UnitF> StandardLutConfig;

}