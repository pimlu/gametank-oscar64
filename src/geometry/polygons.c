#include "polygons.h"

#include "constants.h"
#include "projection.h"
#include "fixed_etc.h"
#include "triangle.h"
#include "camera.h"

#include "graphics/types.h"
#include "graphics/triangle.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct graphics_screen_pos geometry_to_screen(struct coord c) {
    int8_t sx = geometry_round_geof(c.x);
    int8_t sy = geometry_round_geof(c.y);
    struct graphics_screen_pos result = {sx, sy};
    return result;
}

void geometry_fill_triangle(const struct camera *cam, struct triangle *t, uint8_t color) {
    geof_t zero = {GEOF_ZERO};
    if (geof_le(t->a.z, zero) || geof_eq(t->b.z, zero) || geof_eq(t->c.z, zero)) {
        return;
    }

    struct graphics_screen_pos a = geometry_to_screen(t->a);
    struct graphics_screen_pos b = geometry_to_screen(t->b);
    struct graphics_screen_pos c = geometry_to_screen(t->c);
    
    // if (color == 0b000'01'110) {
    //     *(volatile int16_t*) 0x2008 = 0xbeef;
    //     *(volatile int16_t*) 0x2008 = *(uint16_t*)&a;
    //     *(volatile int16_t*) 0x2008 = *(uint16_t*)&b;
    //     *(volatile int16_t*) 0x2008 = *(uint16_t*)&c;
    // }

    graphics_fill_triangle(a, b, c, color);
}

