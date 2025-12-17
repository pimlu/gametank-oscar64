#include "system/boot.h"

#include <stdint.h>

#include "system/bcr.h"
#include "system/imul.h"
#include "system/interrupts.h"
#include "system/scr.h"

#include "graphics/screen.h"
#include "graphics/triangle.h"

#include "geometry/types.h"
#include "geometry/geof.h"
#include "geometry/sin_lut.h"
#include "geometry/camera.h"
#include "geometry/cube.h"
#include "geometry/angle.h"
#include "geometry/projection.h"

#include "fish.h"
#include "horizon.h"
#include "constants.h"

#pragma code(code62)
#pragma data(data62)
#pragma bss(bss)

void init(void);
void tick(void);

static camera_t camera;
static cube_t cube;
static fish_t fish1;

void game_start(void) {
    init();

    for (;;) {
        scr_set_enable_vblank_nmi(false);

        tick();
        scr_flip_framebuffer();
        
        scr_set_enable_vblank_nmi(true);
        wait_for_interrupt();
    }
}

void init(void) {
    mul_init();

    bcr_reset_irq();

    scr_set_default_video_flags();
    
    scr_flip_framebuffer();

    // hue has to be nonzero for some reason to make the blitter work
    uint8_t black = (uint8_t) ~0b00100000;
    graphics_clear_border(black);
    scr_flip_framebuffer();
    graphics_clear_border(black);

    camera.mat = projection_matrix_default();
    camera.proj_frame = 0;
    geof_t heading_theta = {GEOF_CAMERA_HEADING_THETA};
    geof_t pitch_theta = {GEOF_CAMERA_PITCH_THETA};
    angle_set_theta(&camera.rotation.heading, heading_theta);
    angle_set_theta(&camera.rotation.pitch, pitch_theta);

    camera.position.x = (geof_t){GEOF_CAMERA_POS_X};
    camera.position.y = (geof_t){GEOF_CAMERA_POS_Y};
    camera.position.z = (geof_t){GEOF_CAMERA_POS_Z};

    uint8_t cube_colors[6] = {
        (uint8_t) ~0b11101110,
        (uint8_t) ~0b11101110,
        (uint8_t) ~0b11101011,
        (uint8_t) ~0b11101111,
        (uint8_t) ~0b11101110,
        (uint8_t) ~0b11101101
    };

    cube.lo.x = (geof_t){GEOF_ZERO};
    cube.lo.y = (geof_t){GEOF_ZERO};
    cube.lo.z = (geof_t){GEOF_ZERO};
    cube.hi.x = (geof_t){GEOF_CUBE_HI};
    cube.hi.y = (geof_t){GEOF_CUBE_HI};
    cube.hi.z = (geof_t){GEOF_CUBE_HI};
    for (uint8_t i = 0; i < 6; i++) {
        cube.colors[i] = cube_colors[i];
    }

    uint8_t fish_colors[3] = {
        (uint8_t) ~0b01011010,
        (uint8_t) ~0b01011100,
        (uint8_t) ~0b01000000,
    };
    
    fish1.base_pos.x = (geof_t){GEOF_ZERO};
    fish1.base_pos.y = (geof_t){GEOF_FISH_BASE_Y};
    fish1.base_pos.z = (geof_t){GEOF_ZERO};
    geof_t zero_angle = (geof_t){GEOF_ZERO};
    angle_set_theta(&fish1.angle, zero_angle);
    for (uint8_t i = 0; i < 3; i++) {
        fish1.colors[i] = fish_colors[i];
    }
}

void tick(void) {
    uint16_t gamepad1 = scr_read_gamepad1();

    camera_tick_frame(&camera);
    camera_update_from_gamepad(&camera, gamepad1);

    geof_t pitch_theta_val = angle_get_theta(&camera.rotation.pitch);
    geof_t neg_four = {GEOF_HORIZON_NEG_FOUR};
    geof_t horizon_pos_val = geof_mul(pitch_theta_val, neg_four);
    geof_t five = {GEOF_HORIZON_FIVE};
    geof_t pos_y_contrib = geof_mul(camera.position.y, five);
    geof_t horizon_pos = geof_sub(horizon_pos_val, pos_y_contrib);
    draw_horizon(horizon_pos);

    geof_t cube_dist = cube_calc_distance(&cube, &camera);
    geof_t fish_dist = fish_calc_distance(&fish1, &camera);

    if (geof_lt(cube_dist, fish_dist)) {
        fish_paint(&fish1, &camera);
        cube_paint(&cube, &camera);
    } else {
        cube_paint(&cube, &camera);
        fish_paint(&fish1, &camera);
    }
}
