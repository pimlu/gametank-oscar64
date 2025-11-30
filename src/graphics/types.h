#pragma once

#include <stdint.h>

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace graphics {

struct ScreenPos {
    int8_t x, y;
    inline ScreenPos transpose() {
        return {y, x};
    }

    inline ScreenPos flip() {
        return {(int8_t)-y, x};
    }
    inline ScreenPos unflip() {
        return {y, (int8_t)-x};
    }
};


}
