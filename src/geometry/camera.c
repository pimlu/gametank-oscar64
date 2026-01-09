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

void camera_tick_frame(camera_t *cam) {
    cam->proj_frame++;
}

static void camera_update_from_gamepad_standard(camera_t *cam, uint16_t pad) {
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


static void camera_update_from_gamepad_strafe(camera_t *cam, uint16_t pad) {
    struct coord dir = {{GEOF_ZERO}, {GEOF_ZERO}, {GEOF_ZERO}};
    if (pad & INPUT_MASK_LEFT) {
        dir.x = geof_sub(dir.x, geometry_travel_speed);
        if (pad & INPUT_MASK_A) {
            angle_adjust(&cam->rotation.heading, geof_neg(geometry_pan_speed));
        }
    }
    if (pad & INPUT_MASK_RIGHT) {
        dir.x = geof_add(dir.x, geometry_travel_speed);
        if (pad & INPUT_MASK_A) {
            angle_adjust(&cam->rotation.heading, geometry_pan_speed);
        }
    }
    if (pad & INPUT_MASK_UP) {
        dir.y = geof_sub(dir.y, geometry_travel_speed);
    } 
    if (pad & INPUT_MASK_DOWN) {
        dir.y = geof_add(dir.y, geometry_travel_speed);
    }

    struct coord delta = rotation_apply_neg(&cam->rotation, dir);
    cam->position.x = geof_add(cam->position.x, delta.x);
    cam->position.y = geof_add(cam->position.y, delta.y);
    cam->position.z = geof_add(cam->position.z, delta.z);
}

void camera_update_from_gamepad(camera_t *cam, uint16_t pad) {
    struct coord dir = {{GEOF_ZERO}, {GEOF_ZERO}, {GEOF_ZERO}};
    geof_t d_heading = {GEOF_ZERO}, d_pitch = {GEOF_ZERO};

    bool strafe = pad & INPUT_MASK_C;
    bool rot_strafe = strafe && (pad & INPUT_MASK_B);

    if (!strafe || rot_strafe) {
        if (pad & INPUT_MASK_LEFT) {
            d_heading = geof_sub(d_heading, geometry_pan_speed);
        }
        if (pad & INPUT_MASK_RIGHT) {
            d_heading = geof_add(d_heading, geometry_pitch_speed);
        }
        if (pad & INPUT_MASK_UP) {
            d_pitch = geof_sub(d_pitch, geometry_pitch_speed);
        } 
        if (pad & INPUT_MASK_DOWN) {
            d_pitch = geof_add(d_pitch, geometry_pitch_speed);
        }
    }
    if (strafe) {
        if (pad & INPUT_MASK_LEFT) {
            dir.x = geof_sub(dir.x, geometry_travel_speed);
        }
        if (pad & INPUT_MASK_RIGHT) {
            dir.x = geof_add(dir.x, geometry_travel_speed);
        }
        if (pad & INPUT_MASK_UP) {
            dir.y = geof_add(dir.y, geometry_travel_speed);
        } 
        if (pad & INPUT_MASK_DOWN) {
            dir.y = geof_sub(dir.y, geometry_travel_speed);
        }
    } else {
        if (pad & INPUT_MASK_B) {
            dir.z = geof_sub(dir.z, geometry_travel_speed);
        }
        if (pad & INPUT_MASK_A) {
            dir.z = geof_add(dir.z, geometry_travel_speed);
        }
    }

    if (rot_strafe) {
        geof_t strafe_mult = { GEOF_CAMERA_STRAFE_MULT };
        d_heading = geof_mul(d_heading, strafe_mult);
    }
    angle_adjust(&cam->rotation.heading, d_heading);
    angle_adjust(&cam->rotation.pitch, d_pitch);
    if (dir.x.data || dir.y.data || dir.z.data) {
        struct coord delta = rotation_apply_neg(&cam->rotation, dir);
        cam->position.x = geof_add(cam->position.x, delta.x);
        cam->position.y = geof_add(cam->position.y, delta.y);
        cam->position.z = geof_add(cam->position.z, delta.z);
    }
}

struct coord camera_project(const camera_t *cam, struct coord cc) {
    struct coord translated;
    translated.x = geof_sub(cc.x, cam->position.x);
    translated.y = geof_sub(cc.y, cam->position.y);
    translated.z = geof_sub(cc.z, cam->position.z);
    
    struct coord rotated = rotation_apply(&cam->rotation, translated);
    struct coord projected = projection_matrix_project(&cam->mat, rotated);
    return projected;
}

geof_t camera_get_horizon_pos(const camera_t *cam, struct rotation angle) {
    geof_t z_val = {GEOF_CAMERA_HORIZON_Z};
    struct coord p = projection_matrix_project(&cam->mat, rotation_apply(&angle, (struct coord){{GEOF_ZERO}, {GEOF_ZERO}, z_val}));
    return p.y;
}

