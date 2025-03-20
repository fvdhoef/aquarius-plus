#pragma once

#include "Common.h"

class Video {
public:
    void init();
    void render(void *pixels, int pitch);

    void     writeReg(unsigned reg, uint32_t val);
    uint32_t readReg(unsigned reg);

    alignas(4) uint16_t textram[4096];
    alignas(4) uint16_t cursor_ram[32 * 32];

    uint32_t fb_addr;
    bool     gfx_mode = false;

    unsigned cursor_x = 0;
    unsigned cursor_y = 0;
};
