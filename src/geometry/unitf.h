#include <stdint.h>

#include "double.h"
#include "util.h"

#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace geometry {

class UnitF {
public:
    typedef uint16_t Data;
    uint16_t data;
private:
    constexpr UnitF(uint16_t data) : data(data) {}
public:
    constexpr UnitF() : data(0) {}
    constexpr UnitF(const UnitF& o) : data(o.data) {}
    constexpr UnitF(double value) : data(round<uint16_t>(value * (((uint32_t)1) << 16))) {}
    constexpr static UnitF fromRaw(uint16_t data) {
        return UnitF(data);
    }
    constexpr uint16_t getRaw() const {
        return data;
    }

    constexpr uint8_t lsb() const {
        return data & 0xff;
    }

    constexpr uint8_t msb() const {
        return (data >> 8) & 0xff;
    }
    constexpr static UnitF fromRaw(uint8_t lsb, uint8_t msb) {
        uint16_t data = (msb << 8) | lsb;
        return UnitF(data);
    }
    
    constexpr bool operator==(const UnitF& rhs) const {
        return data == rhs.data;
    }
    constexpr bool operator<(const UnitF& rhs) const {
        return data < rhs.data;
    }
    constexpr bool operator>(const UnitF& rhs) const {
        return data > rhs.data;
    }
    constexpr bool operator<=(const UnitF& rhs) const { return !(*this > rhs); }
    constexpr bool operator>=(const UnitF& rhs) const { return !(*this < rhs); }
    constexpr bool operator!=(const UnitF& rhs) const { return !(*this == rhs); }

    constexpr UnitF operator+(const UnitF &r) const {
        UnitF res = *this;
        res += r;
        return res;
    }

    constexpr UnitF& operator+=(const UnitF &r) {
        data += r.data;
        return *this;
    }

    
    constexpr UnitF operator-() const {
        return fromRaw(-data);
    }
    constexpr void negate() {
        data = -data;
    }

    constexpr UnitF operator-(const UnitF &r) const {
        UnitF res = *this;
        res -= r;
        return res;
    }

    constexpr UnitF& operator-=(const UnitF &r) {
        data -= r.data;
        return *this;
    }


    constexpr UnitF operator*(const UnitF &r) const {
        UnitF res = *this;
        res *= r;
        return res;
    }

    UnitF& operator*=(const UnitF &r) {
        uint32_t res = mul16To32(data, r.data);
        data = (uint16_t) (res >> 16);
        return *this;
    }
};

}
