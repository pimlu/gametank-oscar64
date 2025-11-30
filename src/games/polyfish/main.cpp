#include "system/boot.h"


#include "system/bcr.h"
#include "system/interrupts.h"
#include "system/scr.h"
#include "graphics/screen.h"

#include "geometry/types.h"


#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)



void init();
void tick();

void gameStart() {

    init();
    
    geometry::GeoF foo = geometry::GeoF::fromRaw(123);

    *(volatile int16_t*) 0x2008 = foo.data;

    for (;;) {
        scr.setEnableVblankNmi(false);

        tick();
        scr.flipFramebuffer();
        
        scr.setEnableVblankNmi(true);
        waitForInterrupt();
    }
}


uint8_t blue = (uint8_t) ~0b11011100;
uint8_t red  = (uint8_t) ~0b01011110;

uint8_t size = 10;

uint8_t x, y;
int8_t dx, dy;

void init() {
    x = 20;
    y = 70;

    dx = 1;
    dy = 1;

}


// draw a bouncing box on the screen
void tick() {
    graphics::clearScreen(blue);

    bcr.drawBox(x, y, size, size, red);
    waitForInterrupt();

    if (x + dx < graphics::FRAME_X_LO || x + size + dx >= graphics::FRAME_X_HI) {
        dx = -dx;
    }

    if (y + dy < graphics::FRAME_Y_LO || y + size + dy >= graphics::FRAME_Y_HI) {
        dy = -dy;
    }

    x += dx;
    y += dy;
}