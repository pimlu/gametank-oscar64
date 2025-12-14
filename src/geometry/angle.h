#pragma once

#include "types.h"
#include "sin_lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct angle {
    geof_t theta;
    iunitf_t sin_val, cos_val;
};

geof_t angle_get_theta(const struct angle *ang);
void angle_set_theta(struct angle *ang, geof_t val);
void angle_adjust(struct angle *ang, geof_t val);
iunitf_t angle_sin(const struct angle *ang);
iunitf_t angle_cos(const struct angle *ang);
struct coord2d angle_apply(const struct angle *ang, struct coord2d p);
struct coord2d angle_apply_neg(const struct angle *ang, struct coord2d p);

#pragma compile("angle.c")
