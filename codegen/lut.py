import math

from .util import lsb, msb

def gen_sin_lut_data():
    lut = gen_lut("sin_lut", lambda i: round(sin_value(i) * 2**16), sin_value)

    return f"""#include "sin_lut_data.h"

#include "lut.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

{lut}
// pragma align doesn't work inside namespaces for some reason
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


def sin_value(i):
    lo = 0.0
    hi = math.pi/2
    SCALE = (2**16-2) / (2**16)
    
    return SCALE*math.sin(lo + (hi - lo) * (i / 256))
