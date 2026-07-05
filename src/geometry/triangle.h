#pragma once

#include "types.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

struct triangle {
    struct coord a, b, c;
};

// A convex quadrilateral, corners in boundary order (either winding).
struct quad {
    struct coord a, b, c, d;
};

