#include "sin_lut.h"

#include "sin_lut_data.h"
#include "geof.h"
#include "unitf.h"
#include "types.h"
#include "constants.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

static const struct lut_config sin_lut_config = {
    .start = {GEOF_SIN_START},
    .step = {GEOF_SIN_STEP},
    // 1 is slightly outside the range lol
    .last = {UNITF_ALMOST_ONE},
    .lo_miss = {UNITF_ZERO},
    .hi_miss = {UNITF_ALMOST_ONE},
};

static unitf_t sin_lookup(geof_t x) {
    return lut_lookup_or_const(&sin_lut, &sin_lut_config, x);
}

static unitf_t sin_wrap(geof_t x) {
    geof_t threshold = {GEOF_SIN_THRESHOLD};
    if (geof_le(x, threshold)) {
        return sin_lookup(x);
    } else {
        geof_t val = {GEOF_SIN_RANGE};
        return sin_lookup(geof_sub(val, x));
    }
}

iunitf_t geometry_sin(geof_t x) {
    geof_t zero = {GEOF_ZERO};
    iunitf_t result;
    if (geof_ge(x, zero)) {
        result.val = sin_wrap(x);
        result.negated = false;
    } else {
        result.val = sin_wrap(geof_neg(x));
        result.negated = true;
    }
    return result;
}

iunitf_t geometry_cos(geof_t x) {
    // prevent overflow by looping back around
    geof_t threshold = {GEOF_SIN_NEG_THRESHOLD};
    if (geof_le(x, threshold)) {
        geof_t val1 = {GEOF_SIN_VAL1};
        return geometry_sin(geof_sub(val1, x));
    }
    geof_t val2 = {GEOF_SIN_THRESHOLD};
    return geometry_sin(geof_sub(val2, x));
}

