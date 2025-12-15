#pragma once

#include "types.h"
#include "rotation.h"
#include "projection.h"
#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

typedef struct {
    struct rotation rotation;
    struct coord position;
    struct projection_matrix mat;
    uint8_t proj_frame;
} camera_t;

void camera_tick_frame(camera_t *cam);
void camera_update_from_gamepad(camera_t *cam, uint16_t pad);
struct coord camera_project(const camera_t *cam, struct coord cc);
geof_t camera_get_horizon_pos(const camera_t *cam, struct rotation angle);

#pragma compile("camera.c")
