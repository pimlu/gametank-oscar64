import math

from .util import lsb, msb

def gen_sin_lut_data():
    lut = gen_lut("sin_lut", sin_value, sin_comment)

    return f"""#include "sin_lut_data.h"

#include "lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

{lut}

#pragma align(sin_lut, 256)

"""

def gen_lut(name, value, comment):
    values = [value(i) for i in range(256)]
    comments = [comment(i) for i in range(256)]

    lsbs = [f"{f'{lsb(v)},'.ljust(4)} // {c}" for v,c in zip(values, comments)]
    msbs = [f"{f'{msb(v)},'.ljust(4)} // {c}" for v,c in zip(values, comments)]

    return f"""const struct lut {name} = {{
    .lsb = {{
        {'\n        '.join(lsbs)}
    }},
    .msb = {{
        {'\n        '.join(msbs)}
    }}
}};"""


def sin_calc(i):
    lo = 0.0
    hi = math.pi/2
    SCALE = (2**16-2) / (2**16)
    
    return SCALE*math.sin(lo + (hi - lo) * (i / 256))

def sin_value(i):
    return round(sin_calc(i) * 2**16)

def sin_comment(i):
    val = sin_calc(i)
    return f"{val:.6f}"

def gen_recip_lut_data():
    lut = gen_lut("recip_lut", recip_value, recip_comment)

    return f"""#include "recip_lut_data.h"

#include "lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

{lut}

#pragma align(recip_lut, 256)

"""


def recip_value(i):
    START = 1.0
    STEP = 16.0 / 256.0
    
    if i == 0:
        # First entry is almost one (0xffff)
        return 0xffff
    
    cur = START + i * STEP
    return round((1.0 / cur) * 2**16)


def recip_comment(i):
    START = 1.0
    STEP = 16.0 / 256.0
    
    if i == 0:
        return "1.0 (almost one)"
    
    cur = START + i * STEP
    recip = 1.0 / cur
    return f"1 / {cur:.6f} = {recip:.6f}"
