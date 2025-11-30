#pragma once

#include "double.h"


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)



template<typename D>
constexpr D round(float x) {
    D res = x >= 0 ? (D)(x+0.5) : (D)(x-0.5);
    return res;
}
