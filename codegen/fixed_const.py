def geof_raw(value):
    return round(value * 2**8)


SIN_START = 0.0
SIN_STEP = 32.0 / 256.0


MATH_PI = 64.0

def gen_fixed_constants():
    
    return f"""#pragma once

#include "geof.h"
#include "unitf.h"

// Constants for const initializers (Oscar64 doesn't understand macros/function calls)
#define GEOF_ZERO 0
#define GEOF_SIN_START {geof_raw(SIN_START)}
#define GEOF_SIN_STEP {geof_raw(SIN_STEP)}

#define GEOF_PI {geof_raw(MATH_PI)}


#define UNITF_ZERO 0
#define UNITF_ALMOST_ONE 0xffff


#define 

"""
