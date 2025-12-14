#include "types.h"

#include "system/imul.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

geof_t geometry_scale_geof(geof_t val, unitf_t scale) {
    int16_t raw = geof_get_raw(val);
    bool negated = raw < 0;
    uint16_t pos_raw = negated ? -(int32_t) raw : raw;
    uint32_t product = mul16_to_32(pos_raw, unitf_get_raw(scale));
    geof_t result = geof_from_raw(product >> 16);
    return geof_cond(negated, geof_neg(result), result);
}

geof_t geof_cond(bool cond, geof_t true_val, geof_t false_val) {
    if (cond) {
        return true_val;
    } else {
        return false_val;
    }
}