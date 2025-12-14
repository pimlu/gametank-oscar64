#include "cube.h"

#include "polygons.h"
#include "fixed_etc.h"
#include "geof.h"
#include "constants.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

/*
z   y
|  /
| /
  -----x


    6------7
   /       /
  /       /|
 /       / |
4-------5  |
|   |   |  |
|   2---|--3
|  /    |  /
| /     | /
|/      |/
0-------1
*/

geof_t cube_calc_distance(struct cube *cube, struct camera *cam) {
    geof_t tot_z = {GEOF_ZERO};
    for (uint8_t i = 0; i < 8; i++) {
        geof_t x = geof_cond(i & 1, cube->hi.x, cube->lo.x);
        geof_t y = geof_cond(i & 2, cube->hi.y, cube->lo.y);
        geof_t z = geof_cond(i & 4, cube->hi.z, cube->lo.z);

        cube->verts[i] = camera_project(cam, (struct coord){x, y, z});
        tot_z = geof_add(tot_z, cube->verts[i].z);
    }
    // scaleByUint8(totZ, 256 / 8) = scaleByUint8(totZ, 32)
    // This multiplies by 32 and divides by 256, which is equivalent to dividing by 8
    // totZ * 32 / 256 = totZ / 8
    geof_t scale = {GEOF_CUBE_SCALE};
    return geof_mul(tot_z, scale);
}

// #define TRIANGLE(ai, bi, ci) ((struct triangle){cube->verts[ai], cube->verts[bi], cube->verts[ci]})

void cube_paint(struct cube *cube, struct camera *cam) {
    struct coord pos = cam->position;
    struct triangle tri;
    #define TRIANGLE(ai, bi, ci) {tri.a = cube->verts[ai]; tri.b = cube->verts[bi]; tri.c = cube->verts[ci];}
    if (geof_lt(pos.x, cube->lo.x)) {
        TRIANGLE(0, 2, 4);
        geometry_fill_triangle(cam, &tri, cube->colors[0]);
        TRIANGLE(6, 2, 4);
        geometry_fill_triangle(cam, &tri, cube->colors[0]);
    } else if (geof_gt(pos.x, cube->hi.x)) {
        TRIANGLE(1, 3, 5);
        geometry_fill_triangle(cam, &tri, cube->colors[1]);
        TRIANGLE(7, 3, 5);
        geometry_fill_triangle(cam, &tri, cube->colors[1]);
    }

    if (geof_lt(pos.y, cube->lo.y)) {
        TRIANGLE(0, 1, 4);
        geometry_fill_triangle(cam, &tri, cube->colors[2]);
        TRIANGLE(5, 1, 4);
        geometry_fill_triangle(cam, &tri, cube->colors[2]);
    } else if (geof_gt(pos.y, cube->hi.y)) {
        TRIANGLE(2, 3, 6);
        geometry_fill_triangle(cam, &tri, cube->colors[3]);
        TRIANGLE(7, 3, 6);
        geometry_fill_triangle(cam, &tri, cube->colors[3]);
    }

    if (geof_lt(pos.z, cube->lo.z)) {
        TRIANGLE(0, 1, 2);
        geometry_fill_triangle(cam, &tri, cube->colors[4]);
        TRIANGLE(3, 1, 2);
        geometry_fill_triangle(cam, &tri, cube->colors[4]);
    } else if (geof_gt(pos.z, cube->hi.z)) {
        TRIANGLE(4, 5, 6);
        geometry_fill_triangle(cam, &tri, cube->colors[5]);
        TRIANGLE(7, 5, 6);
        geometry_fill_triangle(cam, &tri, cube->colors[5]);
    }
}

struct coord cube_debug_get_vert(const struct cube *cube, uint8_t i) {
    return cube->verts[i];
}

