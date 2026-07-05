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

geof_t cube_calc_distance(cube_t *cube, camera_t *cam) {
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

void cube_paint(cube_t *cube, camera_t *cam) {
    struct coord pos = cam->position;
    struct quad quad;
    // Each face was previously two triangles (A,S1,S2) and (B,S1,S2) sharing
    // edge S1-S2; as a single convex quad the boundary order is A, S1, B, S2.
    #define QUAD(i0, i1, i2, i3) {quad.a = cube->verts[i0]; quad.b = cube->verts[i1]; quad.c = cube->verts[i2]; quad.d = cube->verts[i3];}
    if (geof_lt(pos.x, cube->lo.x)) {
        QUAD(0, 2, 6, 4);
        geometry_fill_convex_quad(cam, &quad, cube->colors[0]);
    } else if (geof_gt(pos.x, cube->hi.x)) {
        QUAD(1, 3, 7, 5);
        geometry_fill_convex_quad(cam, &quad, cube->colors[1]);
    }

    if (geof_lt(pos.y, cube->lo.y)) {
        QUAD(0, 1, 5, 4);
        geometry_fill_convex_quad(cam, &quad, cube->colors[2]);
    } else if (geof_gt(pos.y, cube->hi.y)) {
        QUAD(2, 3, 7, 6);
        geometry_fill_convex_quad(cam, &quad, cube->colors[3]);
    }

    if (geof_lt(pos.z, cube->lo.z)) {
        QUAD(0, 1, 3, 2);
        geometry_fill_convex_quad(cam, &quad, cube->colors[4]);
    } else if (geof_gt(pos.z, cube->hi.z)) {
        QUAD(4, 5, 7, 6);
        geometry_fill_convex_quad(cam, &quad, cube->colors[5]);
    }
}

struct coord cube_debug_get_vert(const cube_t *cube, uint8_t i) {
    return cube->verts[i];
}

