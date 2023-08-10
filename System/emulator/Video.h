#pragma once

#include "Common.h"
#include "SDL.h"

class Video {
public:
    Video();
    const uint16_t *getFb() {
        return screen;
    }

    void drawLine();

private:
    uint16_t screen[VIDEO_WIDTH * VIDEO_HEIGHT];
};
