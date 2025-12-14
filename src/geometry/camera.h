#pragma once

#include "types.h"
#include "rotation.h"
#include "projection.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct camera {
    struct rotation rotation;
    struct coord position;
    struct projection_matrix mat;
    uint8_t proj_frame;
};

void camera_tick_frame(struct camera *cam);
void camera_update_from_gamepad(struct camera *cam, uint16_t pad);
struct coord camera_project(const struct camera *cam, struct coord cc);
geof_t camera_get_horizon_pos(const struct camera *cam, struct rotation angle);

#pragma compile("camera.c")
