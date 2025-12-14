#include "projection.h"

#include "recip_lut.h"
#include "fixed_etc.h"
#include "geof.h"
#include "graphics/screen.h"
#include "constants.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Constants for projection matrix (DEF_PX and DEF_PY are now in constants.h)

struct coord geometry_oob = {{GEOF_ZERO}, {GEOF_ZERO}, {GEOF_ZERO}};

struct coord projection_matrix_project(const struct projection_matrix *mat, struct coord c) {
    geof_t z = geof_neg(c.z);
    if (geof_lt(z, mat->near) || geof_gt(z, mat->far)) {
        return geometry_oob;
    }
    unitf_t div = geometry_recip(z);
    
    bool overflow = false;
    geof_t x = geometry_mul_ratio(c.x, mat->px, div, &overflow);
    if (overflow) return geometry_oob;
    geof_t y = geometry_mul_ratio(c.y, mat->py, div, &overflow);
    if (overflow) return geometry_oob;

    struct coord result = {x, y, z};
    return result;
}

struct projection_matrix projection_matrix_default(void) {
    // formulas from https://gamedev.stackexchange.com/a/120355
    geof_t px = {GEOF_PROJ_DEF_PX};
    geof_t py = {GEOF_PROJ_DEF_PY};
    geof_t near = {GEOF_PROJ_NEAR};
    geof_t far = {GEOF_PROJ_FAR};
    
    struct projection_matrix result = {px, py, near, far};
    return result;
}

