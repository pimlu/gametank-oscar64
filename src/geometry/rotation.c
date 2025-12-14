#include "rotation.h"

#include "system/scr.h"
#include "sin_lut.h"
#include "geof.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// Helper function to multiply iunitf_t by geof_t
static geof_t iunitf_mul_geof(iunitf_t iu, geof_t g) {
    geof_t result = geometry_scale_geof(g, iu.val);
    if (iu.negated) {
        return geof_neg(result);
    }
    return result;
}

struct coord rotation_apply(const struct rotation *rot, struct coord c) {
    // first, rotate to heading
    iunitf_t heading_cos = geometry_cos(rot->heading.theta);
    iunitf_t heading_sin = geometry_sin(rot->heading.theta);
    
    geof_t x = geof_sub(iunitf_mul_geof(heading_cos, c.x), iunitf_mul_geof(heading_sin, c.z));
    geof_t z = geof_add(iunitf_mul_geof(heading_sin, c.x), iunitf_mul_geof(heading_cos, c.z));

    // then, tilt according to pitch
    iunitf_t pitch_cos = geometry_cos(rot->pitch.theta);
    iunitf_t pitch_sin = geometry_sin(rot->pitch.theta);
    
    geof_t y = geof_add(iunitf_mul_geof(pitch_sin, z), iunitf_mul_geof(pitch_cos, c.y));
    z = geof_sub(iunitf_mul_geof(pitch_cos, z), iunitf_mul_geof(pitch_sin, c.y));

    struct coord result = {x, y, z};
    return result;
}

struct coord rotation_apply_neg(const struct rotation *rot, struct coord c) {
    // first, rotate to heading
    iunitf_t heading_cos = geometry_cos(rot->heading.theta);
    iunitf_t heading_sin = geometry_sin(rot->heading.theta);
    
    geof_t x = geof_add(iunitf_mul_geof(heading_cos, c.x), iunitf_mul_geof(heading_sin, c.z));
    geof_t z = geof_add(geof_neg(iunitf_mul_geof(heading_sin, c.x)), iunitf_mul_geof(heading_cos, c.z));

    // then, tilt according to pitch
    iunitf_t pitch_cos = geometry_cos(rot->pitch.theta);
    iunitf_t pitch_sin = geometry_sin(rot->pitch.theta);
    
    geof_t y = geof_add(geof_neg(iunitf_mul_geof(pitch_sin, z)), iunitf_mul_geof(pitch_cos, c.y));
    z = geof_add(iunitf_mul_geof(pitch_cos, z), iunitf_mul_geof(pitch_sin, c.y));

    struct coord result = {x, y, z};
    return result;
}

