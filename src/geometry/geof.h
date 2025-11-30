#include <stdint.h>

#include "double.h"
#include "util.h"

#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


namespace geometry {

class GeoF {
public:
    typedef int16_t Data;
    int16_t data;
private:
    constexpr GeoF(int16_t data) : data(data) {}
public:
    constexpr GeoF() : data(0) {}
    constexpr GeoF(const GeoF& o) : data(o.data) {}
    constexpr GeoF(double value) : data(round<int16_t>(value * (((uint32_t)1) << 8))) {}
    constexpr static GeoF fromRaw(int16_t data) {
        return GeoF(data);
    }
    constexpr int16_t getRaw() const {
        return data;
    }

    constexpr uint8_t lsb() const {
        return data & 0xff;
    }

    constexpr uint8_t msb() const {
        return (data >> 8) & 0xff;
    }
    constexpr static GeoF fromRaw(uint8_t lsb, uint8_t msb) {
        int16_t data = (msb << 8) | lsb;
        return GeoF(data);
    }
    
    constexpr bool operator==(const GeoF& rhs) const {
        return data == rhs.data;
    }
    constexpr bool operator<(const GeoF& rhs) const {
        return data < rhs.data;
    }
    constexpr bool operator>(const GeoF& rhs) const {
        return data > rhs.data;
    }
    constexpr bool operator<=(const GeoF& rhs) const { return !(*this > rhs); }
    constexpr bool operator>=(const GeoF& rhs) const { return !(*this < rhs); }
    constexpr bool operator!=(const GeoF& rhs) const { return !(*this == rhs); }

    constexpr GeoF operator+(const GeoF &r) const {
        GeoF res = *this;
        res += r;
        return res;
    }

    constexpr GeoF& operator+=(const GeoF &r) {
        data += r.data;
        return *this;
    }

    
    constexpr GeoF operator-() const {
        return fromRaw(-data);
    }
    constexpr void negate() {
        data = -data;
    }

    constexpr GeoF operator-(const GeoF &r) const {
        GeoF res = *this;
        res -= r;
        return res;
    }

    constexpr GeoF& operator-=(const GeoF &r) {
        data -= r.data;
        return *this;
    }


    constexpr GeoF operator*(const GeoF &r) const {
        GeoF res = *this;
        res *= r;
        return res;
    }

    GeoF& operator*=(const GeoF &r) {
        int32_t res = imul16To32(data, r.data);
        data = (int16_t) (res >> 8);
        return *this;
    }
};

}
