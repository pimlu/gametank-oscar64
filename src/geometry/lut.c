#include "lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// returns log2(x), so for x = 2^n
static uint8_t bit_length(int16_t x) {
    uint8_t shift = 0;
    while (x > 1) {
        x >>= 1;
        shift++;
    }
    return shift;
}

unitf_t lut_lookup(const struct lut *lut, uint8_t i) {
    return unitf_from_raw_bytes(lut->lsb[i], lut->msb[i]);
}

unitf_t lut_lerp(const struct lut *lut, const struct lut_config *config, unitf_t a, unitf_t b, uint8_t frac) {
    (void)lut;  // Unused parameter - kept for API consistency
    int16_t step_raw = geof_get_raw(config->step);
    uint8_t step_bits = bit_length(step_raw);
    uint8_t rem_bits = 8 - step_bits;
    uint8_t inv_frac = (1 << step_bits) - frac;

    // this only exists for UnitF right now...
    unitf_t a_part = unitf_scale_by_uint8(a, inv_frac << rem_bits);
    unitf_t b_part = unitf_scale_by_uint8(b, frac << rem_bits);
    return unitf_add(a_part, b_part);
}

unitf_t lut_lookup_or_const(const struct lut *lut, const struct lut_config *config, geof_t x) {
    if (geof_lt(x, config->start)) {
        return config->lo_miss;
    }
    
    geof_t end = geof_add(config->start, geof_small_int_mult(config->step, 256));
    
    if (geof_ne(end, config->start)) {
        if (geof_eq(x, end)) {
            return config->last;
        } else if (geof_gt(x, end)) {
            return config->hi_miss;
        }
    }
    
    geof_t shifted = geof_sub(x, config->start);
    int16_t shifted_bits = geof_get_raw(shifted);
    int16_t step_raw = geof_get_raw(config->step);
    uint8_t step_bits = bit_length(step_raw);
    uint8_t step_mask = step_raw - 1;
    uint8_t idx = shifted_bits >> step_bits;
    uint8_t frac = shifted_bits & step_mask;

    if (!frac) {
        return lut_lookup(lut, idx);
    }
    
    unitf_t next = config->last;
    if (idx != 255) {
        next = lut_lookup(lut, idx + 1);
    }
    
    unitf_t res = lut_lerp(lut, config, lut_lookup(lut, idx), next, frac);
    return res;
}

