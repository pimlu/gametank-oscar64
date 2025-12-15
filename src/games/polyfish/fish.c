#include "fish.h"

#include "geometry/polygons.h"
#include "graphics/screen.h"
#include "system/bcr.h"
#include "geometry/angle.h"
#include "geometry/geof.h"
#include "constants.h"

#pragma code(code62)
#pragma data(data62)
#pragma bss(bss)

static void fish_vertex(fish_t *fish, camera_t *cam, uint8_t i, geof_t x, geof_t y) {
    // Calculate px, pz from angle.apply({3.0, 0.0})
    struct coord2d applied_main = angle_apply(&fish->angle, (struct coord2d){{GEOF_FISH_MAIN_OFFSET}, {GEOF_ZERO}});
    geof_t px = applied_main.x;
    geof_t pz = applied_main.y;
    
    // Calculate dx, dz from angle.apply({0.0, -(x)})
    geof_t neg_x = geof_neg(x);
    struct coord2d applied_offset = angle_apply(&fish->angle, (struct coord2d){{GEOF_ZERO}, neg_x});
    geof_t dx = applied_offset.x;
    geof_t dz = applied_offset.y;
    
    // basePos + Coord({pz + dz, (y), px + dx})
    struct coord offset;
    offset.x = geof_add(pz, dz);
    offset.y = y;
    offset.z = geof_add(px, dx);
    
    struct coord world_pos;
    world_pos.x = geof_add(fish->base_pos.x, offset.x);
    world_pos.y = geof_add(fish->base_pos.y, offset.y);
    world_pos.z = geof_add(fish->base_pos.z, offset.z);
    
    fish->verts[i] = camera_project(cam, world_pos);
}

geof_t fish_calc_distance(fish_t *fish, camera_t *cam) {
    geof_t adjust_val = {GEOF_FISH_ADJUST};
    angle_adjust(&fish->angle, adjust_val);
    
    fish_vertex(fish, cam, 0, (geof_t){GEOF_FISH_VERTEX_X_NEG_HALF}, (geof_t){GEOF_ZERO});
    fish_vertex(fish, cam, 1, (geof_t){GEOF_ZERO}, (geof_t){GEOF_FISH_VERTEX_Y_075});
    fish_vertex(fish, cam, 2, (geof_t){GEOF_ZERO}, (geof_t){GEOF_FISH_VERTEX_Y_NEG_075});
    fish_vertex(fish, cam, 3, (geof_t){GEOF_FISH_VERTEX_X_1}, (geof_t){GEOF_FISH_VERTEX_Y_HALF});
    fish_vertex(fish, cam, 4, (geof_t){GEOF_FISH_VERTEX_X_1}, (geof_t){GEOF_FISH_VERTEX_Y_NEG_HALF});
    fish_vertex(fish, cam, 5, (geof_t){GEOF_FISH_VERTEX_X_1_5}, (geof_t){GEOF_FISH_VERTEX_Y_1});
    fish_vertex(fish, cam, 6, (geof_t){GEOF_FISH_VERTEX_X_1_5}, (geof_t){GEOF_FISH_VERTEX_Y_NEG_1});
    fish_vertex(fish, cam, 7, (geof_t){GEOF_ZERO}, (geof_t){GEOF_FISH_VERTEX_Y_01});
    
    return fish->verts[7].z;
}

#define TRIANGLE(a, b, c) ((struct triangle){fish->verts[a], fish->verts[b], fish->verts[c]})

void fish_paint(fish_t *fish, camera_t *cam) {
    // front
    geometry_fill_triangle(cam, &TRIANGLE(0, 1, 2), fish->colors[0]);

    // middle
    geometry_fill_triangle(cam, &TRIANGLE(0, 1, 2), fish->colors[0]);
    geometry_fill_triangle(cam, &TRIANGLE(1, 2, 3), fish->colors[0]);
    geometry_fill_triangle(cam, &TRIANGLE(2, 3, 4), fish->colors[0]);

    // tail
    geometry_fill_triangle(cam, &TRIANGLE(3, 4, 5), fish->colors[1]);
    geometry_fill_triangle(cam, &TRIANGLE(3, 4, 6), fish->colors[1]);

    // eye
    geof_t zero = {GEOF_ZERO};
    if (geof_le(fish->verts[7].z, zero)) return;
    
    struct graphics_screen_pos screen_pos = geometry_to_screen(fish->verts[7]);
    int8_t x = screen_pos.x;
    int8_t y = -screen_pos.y;

    if (y < GRAPHICS_SCENE_Y_LO || y >= GRAPHICS_SCENE_Y_HI || x < GRAPHICS_SCENE_X_LO || x >= GRAPHICS_SCENE_X_HI) {
        return;
    }
    
    bcr_draw_box(x + GRAPHICS_SCENE_X_TO_FRAME, y + GRAPHICS_SCENE_Y_TO_FRAME, 1, 1, fish->colors[2]);
}

