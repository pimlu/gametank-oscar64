#include "angle.h"

#include "sin_lut.h"
#include "geof.h"
#include "constants.h"

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

geof_t angle_get_theta(const struct angle *ang) {
    return ang->theta;
}

void angle_set_theta(struct angle *ang, geof_t val) {
    ang->theta = val;
    ang->sin_val = geometry_sin(val);
    ang->cos_val = geometry_cos(val);
}

void angle_adjust(struct angle *ang, geof_t val) {
    geof_t next = geof_add(angle_get_theta(ang), val);
    geof_t threshold_high = {GEOF_SIN_RANGE};
    geof_t threshold_low = geof_neg((geof_t){GEOF_SIN_RANGE});
    geof_t wrap_val = {GEOF_ANGLE_WRAP};
    
    if (geof_gt(next, threshold_high)) {
        next = geof_sub(next, wrap_val);
    }
    if (geof_lt(next, threshold_low)) {
        next = geof_add(next, wrap_val);
    }
    angle_set_theta(ang, next);
}

iunitf_t angle_sin(const struct angle *ang) {
    return ang->sin_val;
}

iunitf_t angle_cos(const struct angle *ang) {
    return ang->cos_val;
}

struct coord2d angle_apply(const struct angle *ang, struct coord2d p) {
    iunitf_t cos_val = angle_cos(ang);
    iunitf_t sin_val = angle_sin(ang);
    
    geof_t x = geof_sub(iunitf_mul_geof(cos_val, p.x), iunitf_mul_geof(sin_val, p.y));
    geof_t y = geof_add(iunitf_mul_geof(sin_val, p.x), iunitf_mul_geof(cos_val, p.y));
    
    struct coord2d result = {x, y};
    return result;
}

struct coord2d angle_apply_neg(const struct angle *ang, struct coord2d p) {
    iunitf_t cos_val = angle_cos(ang);
    iunitf_t sin_val = angle_sin(ang);
    
    geof_t x = geof_add(iunitf_mul_geof(cos_val, p.x), iunitf_mul_geof(sin_val, p.y));
    geof_t y = geof_add(geof_neg(iunitf_mul_geof(sin_val, p.x)), iunitf_mul_geof(cos_val, p.y));
    
    struct coord2d result = {x, y};
    return result;
}

