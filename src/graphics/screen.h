#pragma once

#include <stdint.h>


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)


namespace graphics {

extern constexpr uint8_t SCREEN_WIDTH;
extern constexpr uint8_t SCREEN_HEIGHT;

extern constexpr uint8_t PADDING_TOP;
extern constexpr uint8_t PADDING_BOT;
extern constexpr uint8_t PADDING_LEFT;
extern constexpr uint8_t PADDING_RIGHT;

extern constexpr uint8_t FRAME_X_LO;
extern constexpr uint8_t FRAME_X_HI;
extern constexpr uint8_t FRAME_W;

extern constexpr uint8_t FRAME_Y_LO;
extern constexpr uint8_t FRAME_Y_HI;
extern constexpr uint8_t FRAME_H;

extern constexpr int8_t SCENE_X_LO;
extern constexpr int8_t SCENE_X_HI;
extern constexpr int8_t SCENE_Y_LO;
extern constexpr int8_t SCENE_Y_HI;

extern constexpr int8_t SCENE_X_TO_FRAME;
extern constexpr int8_t SCENE_Y_TO_FRAME;

void clearBorder(uint8_t c);
void clearScreen(uint8_t c);

}

#pragma compile("screen.cpp")
