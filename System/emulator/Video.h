#pragma once

#include "Common.h"
#include "SDL.h"

enum {
    VCTRL_TEXT_ENABLE       = (1 << 0),
    VCTRL_MODE_OFF          = (0 << 1),
    VCTRL_MODE_TILEMAP      = (1 << 1),
    VCTRL_MODE_BITMAP       = (2 << 1),
    VCTRL_MODE_BITMAP_4BPP  = (3 << 1),
    VCTRL_MODE_MASK         = (3 << 1),
    VCTRL_SPRITES_ENABLE    = (1 << 3),
    VCTRL_TEXT_PRIORITY     = (1 << 4),
    VCTRL_REMAP_BORDER_CHAR = (1 << 5),
    VCTRL_80_COLUMNS        = (1 << 6),
    VCTRL_TRAM_PAGE         = (1 << 7),
};

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
