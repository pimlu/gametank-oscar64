#pragma once

#include "geof.h"
#include "unitf.h"
#include "graphics/types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


namespace geometry {


GeoF scaleGeoF(GeoF val, UnitF scale);

// this lame thing is because UnitF is unsigned
struct iUnitF {
    UnitF val;
    bool negated;
    

    GeoF operator*(const GeoF &r) const {
        // *(volatile int16_t*) 0x2008 = 0xbeAf;
        // *(volatile int16_t*) 0x2008 = r.getRaw();
        // *(volatile uint16_t*) 0x2008 = val.getRaw();
        // if (r.getRaw()) {
        //     *(volatile int16_t*) 0x2008 = r.getRaw();
        // }
        GeoF res = scaleGeoF(r, val);
        // *(volatile int16_t*) 0x2008 = res.getRaw();
        // *(volatile int16_t*) 0x2008 = 0xdead;
        return negated ? -res : res; 
    }
    iUnitF operator-() const {
        return {val, !negated};
    }
};

struct Coord {
    GeoF x, y, z;

    constexpr Coord& operator+=(const Coord &r) {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }
    constexpr Coord operator+(const Coord &r) const {
        Coord res = *this;
        res += r;
        return res;
    }
    constexpr Coord operator-(const Coord &r) const {
        return {x - r.x, y - r.y, z - r.z};
    }
};

struct Coord2d {
    GeoF x, y;
};

}

