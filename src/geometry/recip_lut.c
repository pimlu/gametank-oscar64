#include "recip_lut.h"

#include "recip_lut_data.h"
#include "geof.h"
#include "unitf.h"
#include "types.h"
#include "constants.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

static const struct lut_config recip_lut_config = {
    .start = {GEOF_RECIP_START},
    .step = {GEOF_RECIP_STEP},
    .last = {UNITF_RECIP_LAST},
    .lo_miss = {UNITF_ZERO},
    .hi_miss = {UNITF_ZERO},
};

unitf_t geometry_recip(geof_t x) {
    return lut_lookup_or_const(&recip_lut, &recip_lut_config, x);
}

