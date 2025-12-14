#pragma once

#include "types.h"
#include "fixed_etc.h"
#include "graphics/screen.h"
#include "graphics/types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

// near vs far - dynamic range of the frustum
#define NEAR 1.0
// chosen to make FAR-NEAR a power of 2
#define FAR 17.0
// Math.tan(60 / 2 * (1/360 * 2 * Math.PI))
#define TAN_FOV2 0.5773502691896257
#define ASPECT_RATIO ((double)GRAPHICS_FRAME_W / (double)GRAPHICS_FRAME_H)

#define DEF_PX ((GRAPHICS_FRAME_W / 2.0) / (ASPECT_RATIO * TAN_FOV2))
#define DEF_PY ((GRAPHICS_FRAME_H / 2.0) / TAN_FOV2)
#define DEF_DYRANGE (1.0 / (FAR - NEAR))

struct projection_matrix {
    // the values range from [0,4] ish
    // this contains the formulas from L to R of:
    // https://gamedev.stackexchange.com/a/120355
    geof_t px, py, near, far;
};

struct coord projection_matrix_project(const struct projection_matrix *mat, struct coord c);
struct projection_matrix projection_matrix_default(void);

#pragma compile("projection.c")
