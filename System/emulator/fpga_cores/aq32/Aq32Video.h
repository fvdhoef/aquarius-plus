#pragma once

#include "Common.h"

class Aq32Video {
public:
    enum {
        VCTRL_TEXT_EN      = (1 << 0),
        VCTRL_TEXT_MODE80  = (1 << 1),
        VCTRL_TEXT_PRIO    = (1 << 2),
        VCTRL_GFX_EN       = (1 << 3),
        VCTRL_GFX_TILEMODE = (1 << 4),
        VCTRL_SPR_EN       = (1 << 5),
        VCTRL_LAYER2_EN    = (1 << 6),
        VCTRL_BM_WRAP      = (1 << 7),
    };

    Aq32Video();
    const uint16_t *getFb() { return screen; }

    static const int activeWidth  = 640;
    static const int activeHeight = 240;

    bool isOnVideoIrqLine() { return videoLine == videoIrqLine; }
    bool isOnStartOfVBlank() { return videoLine == 240; }

    void reset();
    void drawLine(int line);

    void dbgDrawIoRegs();
    void dbgDrawSpriteRegs();
    void dbgDrawPaletteRegs();

    alignas(4) uint16_t textRam[4096];      // Screen RAM for text mode
    alignas(4) uint8_t videoRam[32 * 1024]; // Video RAM
    alignas(4) uint8_t charRam[2048];       // Character RAM
    uint16_t videoPalette[128] = {0};       // Video palette

    uint32_t spritePos[256];
    uint32_t spriteAttr[256];

    uint8_t  videoCtrl    = 0; // Video control register
    uint16_t videoLine    = 0; // Current line number
    uint8_t  videoIrqLine = 0; // Line number at which to generate IRQ
    uint16_t videoScrX1   = 0; // Tile layer 1 horizontal scroll register
    uint8_t  videoScrY1   = 0; // Tile layer 1 vertical scroll register
    uint16_t videoScrX2   = 0; // Tile layer 2 horizontal scroll register
    uint8_t  videoScrY2   = 0; // Tile layer 2 vertical scroll register

    uint16_t screen[activeWidth * activeHeight];

private:
    void renderer(unsigned &idx, uint32_t data, bool hFlip, unsigned palette, unsigned zDepth, bool zDepthInit);

    uint8_t lineGfx[512];
    uint8_t lineZ[512];
};
