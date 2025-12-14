#include "camera.h"

#include "system/scr.h"
#include "geof.h"
#include "constants.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

const geof_t geometry_pan_speed = {GEOF_CAMERA_PAN_SPEED};
const geof_t geometry_pitch_speed = {GEOF_CAMERA_PITCH_SPEED};
const geof_t geometry_travel_speed = {GEOF_CAMERA_TRAVEL_SPEED};

void camera_tick_frame(struct camera *cam) {
    cam->proj_frame++;
}

void camera_update_from_gamepad(struct camera *cam, uint16_t pad) {
    if (pad & INPUT_MASK_LEFT) {
        angle_adjust(&cam->rotation.heading, geof_neg(geometry_pan_speed));
    }
    if (pad & INPUT_MASK_RIGHT) {
        angle_adjust(&cam->rotation.heading, geometry_pan_speed);
    }
    if (pad & INPUT_MASK_UP) {
        angle_adjust(&cam->rotation.pitch, geof_neg(geometry_pitch_speed));
    } 
    if (pad & INPUT_MASK_DOWN) {
        angle_adjust(&cam->rotation.pitch, geometry_pitch_speed);
    }
    if (pad & INPUT_MASK_B) {
        struct coord delta = rotation_apply_neg(&cam->rotation, (struct coord){{GEOF_ZERO}, {GEOF_ZERO}, geof_neg(geometry_travel_speed)});
        cam->position.x = geof_add(cam->position.x, delta.x);
        cam->position.y = geof_add(cam->position.y, delta.y);
        cam->position.z = geof_add(cam->position.z, delta.z);
    }
    if (pad & INPUT_MASK_A) {
        struct coord delta = rotation_apply_neg(&cam->rotation, (struct coord){{GEOF_ZERO}, {GEOF_ZERO}, geometry_travel_speed});
        cam->position.x = geof_add(cam->position.x, delta.x);
        cam->position.y = geof_add(cam->position.y, delta.y);
        cam->position.z = geof_add(cam->position.z, delta.z);
    }
}

struct coord camera_project(const struct camera *cam, struct coord cc) {
    struct coord translated;
    translated.x = geof_sub(cc.x, cam->position.x);
    translated.y = geof_sub(cc.y, cam->position.y);
    translated.z = geof_sub(cc.z, cam->position.z);
    
    struct coord rotated = rotation_apply(&cam->rotation, translated);
    struct coord projected = projection_matrix_project(&cam->mat, rotated);
    return projected;
}

geof_t camera_get_horizon_pos(const struct camera *cam, struct rotation angle) {
    geof_t z_val = {GEOF_CAMERA_HORIZON_Z};
    struct coord p = projection_matrix_project(&cam->mat, rotation_apply(&angle, (struct coord){{GEOF_ZERO}, {GEOF_ZERO}, z_val}));
    return p.y;
}

