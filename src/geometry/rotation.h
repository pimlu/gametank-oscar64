#pragma once

#include "angle.h"
#include "types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct rotation {
    struct angle heading, pitch;
};

struct coord rotation_apply(const struct rotation *rot, struct coord c);
struct coord rotation_apply_neg(const struct rotation *rot, struct coord c);

#pragma compile("rotation.c")
