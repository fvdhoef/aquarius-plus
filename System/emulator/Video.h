#pragma once

#include "Common.h"
#include "SDL.h"

class Video {
    Video();

public:
    static Video &instance();

    const uint16_t *getFb() {
        return screen;
    }

    void drawLine();

private:
    uint16_t screen[VIDEO_WIDTH * VIDEO_HEIGHT];
};
