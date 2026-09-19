#pragma once

#include "Common.h"

class AqpVideo {
public:
    enum {
        VCTRL_TEXT_ENABLE         = (1 << 0),
        VCTRL_GFXMODE_OFF         = (0 << 1),
        VCTRL_GFXMODE_TILEMAP     = (1 << 1),
        VCTRL_GFXMODE_BITMAP      = (2 << 1),
        VCTRL_GFXMODE_BITMAP_4BPP = (3 << 1),
        VCTRL_GFXMODE_MASK        = (3 << 1),
        VCTRL_SPRITES_ENABLE      = (1 << 3),
        VCTRL_TEXT_PRIORITY       = (1 << 4),
        VCTRL_REMAP_BORDER_CHAR   = (1 << 5),
        VCTRL_80_COLUMNS          = (1 << 6),
        VCTRL_TRAM_PAGE           = (1 << 7),
    };

    AqpVideo();
    const uint16_t *getFb() { return screen; }

    static const int activeWidth  = 704;
    static const int activeHeight = 240;

    bool isOnVideoIrqLine() { return videoLine == videoIrqLine; }
    bool isOnStartOfVBlank() { return videoLine == 240; }

    void reset();
    void drawLine(int line);

    void dbgDrawIoRegs();
    void dbgDrawSpriteRegs();
    void dbgDrawPaletteRegs();

    void    writeReg(uint8_t r, uint8_t v);
    uint8_t readReg(uint8_t r);

    uint8_t readScreenOrColorRam(unsigned addr) {
        addr &= 0x7FF;

        if (videoCtrl & VCTRL_80_COLUMNS) {
            if (videoCtrl & VCTRL_TRAM_PAGE) {
                return colorRam[addr];
            } else {
                return screenRam[addr];
            }

        } else {
            unsigned offset = (videoCtrl & VCTRL_TRAM_PAGE) ? 0x400 : 0;
            if (addr < 0x400) {
                return screenRam[offset | (addr & 0x3FF)];
            } else {
                return colorRam[offset | (addr & 0x3FF)];
            }
        }
    }

    void writeScreenOrColorRam(unsigned addr, uint8_t data) {
        addr &= 0x7FF;

        if (videoCtrl & VCTRL_80_COLUMNS) {
            if (videoCtrl & VCTRL_TRAM_PAGE) {
                colorRam[addr] = data;
            } else {
                screenRam[addr] = data;
            }

        } else {
            unsigned offset = (videoCtrl & VCTRL_TRAM_PAGE) ? 0x400 : 0;
            if (addr < 0x400) {
                screenRam[offset | (addr & 0x3FF)] = data;
            } else {
                colorRam[offset | (addr & 0x3FF)] = data;
            }
        }
    }

    uint8_t screenRam[2048];     // $3000-33FF: Screen RAM for text mode
    uint8_t colorRam[2048];      // $3400-37FF: Color RAM for text mode
    uint8_t videoRam[16 * 1024]; // Video RAM
    uint8_t charRam[2048];       // Character RAM

    uint16_t videoLine = 0; // $EC   : Current line number

private:
    uint8_t  videoCtrl        = 0;   // $E0   : Video control register
    uint16_t videoScrX        = 0;   // $E1/E2: Tile map horizontal scroll register
    uint8_t  videoScrY        = 0;   // $E3   : Tile map horizontal scroll register
    uint8_t  videoSprSel      = 0;   // $E4   : Sprite select
    uint16_t videoSprX[64]    = {0}; // $E5/E6: Sprite X-position
    uint8_t  videoSprY[64]    = {0}; // $E7   : Sprite Y-position
    uint16_t videoSprIdx[64]  = {0}; // $E8/E9: Sprite tile index
    uint8_t  videoSprAttr[64] = {0}; // $E9   : Sprite attributes
    uint8_t  videoPalSel      = 0;   // $EA   : Palette entry select
    uint16_t videoPalette[64] = {0}; // $EB   : Video palette
    uint8_t  videoIrqLine     = 0;   // $ED   : Line number at which to generate IRQ

    uint16_t screen[activeWidth * activeHeight];
};
