#include "screen.h"

#include "system/bcr.h"
#include "system/interrupts.h"

#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)

namespace graphics {


inline constexpr uint8_t SCREEN_WIDTH = 128;
inline constexpr uint8_t SCREEN_HEIGHT = 128;

inline constexpr uint8_t PADDING_TOP = 7;
inline constexpr uint8_t PADDING_BOT = 8;
inline constexpr uint8_t PADDING_LEFT = 1;
inline constexpr uint8_t PADDING_RIGHT = 1;



inline constexpr uint8_t FRAME_X_LO = PADDING_LEFT;
inline constexpr uint8_t FRAME_X_HI = SCREEN_WIDTH - PADDING_RIGHT;
inline constexpr uint8_t FRAME_W = FRAME_X_HI - FRAME_X_LO;

inline constexpr uint8_t FRAME_Y_LO = PADDING_TOP;
inline constexpr uint8_t FRAME_Y_HI = SCREEN_HEIGHT - PADDING_BOT;
inline constexpr uint8_t FRAME_H = FRAME_Y_HI - FRAME_Y_LO;


inline constexpr int8_t SCENE_X_LO = 0 - (FRAME_W + 1) / 2;
inline constexpr int8_t SCENE_X_HI = 0 + (FRAME_W    ) / 2;
// Y is inverted so invert the rounding I guess
inline constexpr int8_t SCENE_Y_LO = 0 - (FRAME_H    ) / 2;
inline constexpr int8_t SCENE_Y_HI = 0 + (FRAME_H + 1) / 2;

inline constexpr int8_t SCENE_X_TO_FRAME = ((int8_t)FRAME_X_LO) - SCENE_X_LO;
inline constexpr int8_t SCENE_Y_TO_FRAME = ((int8_t)FRAME_Y_LO) - SCENE_Y_LO;

void clearBorder(uint8_t c) {
    bcr.drawBox(0, 0, SCREEN_WIDTH-PADDING_RIGHT, PADDING_TOP, c);
    waitForInterrupt(); 
    bcr.drawBox(0, PADDING_TOP, PADDING_RIGHT, SCREEN_HEIGHT-PADDING_TOP, c);
    waitForInterrupt(); 
    bcr.drawBox(PADDING_LEFT, SCREEN_HEIGHT-PADDING_BOT, SCREEN_WIDTH-PADDING_LEFT, PADDING_BOT, c);
    waitForInterrupt(); 
    bcr.drawBox(SCREEN_WIDTH-PADDING_RIGHT, 0, PADDING_RIGHT, SCREEN_HEIGHT-PADDING_BOT, c);
    waitForInterrupt();
}
void clearScreen(uint8_t c) {
    bcr.drawBox(
        FRAME_X_LO, FRAME_Y_LO,
        FRAME_W, FRAME_H,
        c);
    waitForInterrupt();
}

}