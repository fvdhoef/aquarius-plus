#include "Video.h"
#include "EmuState.h"

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
};

Video::Video() {
}

void Video::drawLine() {
    int line = emuState.videoLine;
    if (line < 0 || line >= VIDEO_HEIGHT)
        return;

    // Render text
    uint8_t  lineText[512];
    unsigned idx = 512 - 16;

    bool vActive = line >= 16 && line < 216;

    for (int i = 0; i < VIDEO_WIDTH; i++) {
        // Draw text character
        uint8_t ch;
        uint8_t color;
        if (vActive && idx < 320) {
            int row    = (line - 16) / 8;
            int column = (i - 16) / 8;
            ch         = emuState.screenRam[row * 40 + column];
            color      = emuState.colorRam[row * 40 + column];

        } else {
            unsigned borderAddr = (emuState.videoCtrl & VCTRL_REMAP_BORDER_CHAR) ? 0x3FF : 0;

            ch    = emuState.screenRam[borderAddr];
            color = emuState.colorRam[borderAddr];
        }

        uint8_t charBm = emuState.charRam[ch * 8 + (line & 7)];
        lineText[idx]  = (charBm & (1 << (7 - (i & 7)))) ? (color >> 4) : (color & 0xF);
        idx            = (idx + 1) & 511;
    }

    // Render bitmap/tile layer
    uint8_t lineGfx[512];
    if (vActive) {
        int bmline = line - 16;
        switch (emuState.videoCtrl & VCTRL_MODE_MASK) {
            case VCTRL_MODE_BITMAP: {
                // Bitmap mode 1bpp
                for (int i = 0; i < 320; i++) {
                    int     row    = bmline / 8;
                    int     column = i / 8;
                    uint8_t col    = emuState.videoRam[0x2000 + row * 40 + column];
                    uint8_t bm     = emuState.videoRam[0x0000 + bmline * 40 + column];
                    uint8_t color  = (bm & (1 << (7 - (i & 7)))) ? (col >> 4) : (col & 0xF);

                    lineGfx[i] = (1 << 4) | color;
                }
                break;
            }

            case VCTRL_MODE_BITMAP_4BPP: {
                // Bitmap mode 4bpp
                for (int i = 0; i < 80; i++) {
                    uint8_t col = emuState.videoRam[bmline * 80 + i];

                    lineGfx[i * 4 + 0] = (1 << 4) | (col >> 4);
                    lineGfx[i * 4 + 1] = (1 << 4) | (col >> 4);
                    lineGfx[i * 4 + 2] = (1 << 4) | (col & 0xF);
                    lineGfx[i * 4 + 3] = (1 << 4) | (col & 0xF);
                }
                break;
            }

            case VCTRL_MODE_TILEMAP: {
                // Tile mode
                idx               = (-(emuState.videoScrX & 7)) & 511;
                unsigned tileLine = (bmline + emuState.videoScrY) & 255;
                unsigned row      = (tileLine >> 3) & 31;
                unsigned col      = emuState.videoScrX >> 3;

                for (int i = 0; i < 41; i++) {
                    // Tilemap is 64x32 (2 bytes per entry)

                    // Fetch tilemap entry
                    uint8_t entryL = emuState.videoRam[(row << 7) | (col << 1)];
                    uint8_t entryH = emuState.videoRam[(row << 7) | (col << 1) | 1];

                    unsigned tileIdx = ((entryH & 1) << 8) | entryL;
                    bool     hFlip   = (entryH & (1 << 1)) != 0;
                    bool     vFlip   = (entryH & (1 << 2)) != 0;
                    uint8_t  attr    = entryH & 0x70;

                    unsigned patOffs = (tileIdx << 5) | ((tileLine & 7) << 2);
                    if (vFlip)
                        patOffs ^= (7 << 2);

                    // Next column
                    col = (col + 1) & 63;

                    for (int n = 0; n < 4; n++) {
                        int m = n;
                        if (hFlip)
                            m ^= 3;

                        uint8_t data = emuState.videoRam[patOffs + m];

                        if (!hFlip) {
                            uint8_t val = data >> 4;
                            val |= (attr & ((val == 0) ? 0x30 : 0x70));
                            lineGfx[idx++] = val;
                            idx &= 511;
                        }
                        {
                            uint8_t val = data & 0xF;
                            val |= (attr & ((val == 0) ? 0x30 : 0x70));
                            lineGfx[idx++] = val;
                            idx &= 511;
                        }
                        if (hFlip) {
                            uint8_t val = data >> 4;
                            val |= (attr & ((val == 0) ? 0x30 : 0x70));
                            lineGfx[idx++] = val;
                            idx &= 511;
                        }
                    }
                }
                break;
            }

            default: {
                for (int i = 0; i < 320; i++) {
                    lineGfx[i] = 0;
                }
                break;
            }
        }

        // Render sprites
        if ((emuState.videoCtrl & (1 << 3)) != 0) {
            for (int i = 0; i < 64; i++) {
                unsigned sprAttr = emuState.videoSprAttr[i];

                // Check if sprite enabled
                bool enabled = (sprAttr & (1 << 7)) != 0;
                if (!enabled)
                    continue;

                // Check if sprite is visible on this line
                bool h16     = (sprAttr & (1 << 3)) != 0;
                int  sprLine = (bmline - emuState.videoSprY[i]) & 0xFF;
                if (sprLine >= (h16 ? 16 : 8))
                    continue;

                int      sprX     = emuState.videoSprX[i];
                unsigned tileIdx  = emuState.videoSprIdx[i];
                bool     hFlip    = (sprAttr & (1 << 1)) != 0;
                bool     vFlip    = (sprAttr & (1 << 2)) != 0;
                uint8_t  palette  = sprAttr & 0x30;
                bool     priority = (sprAttr & (1 << 6)) != 0;

                idx = sprX;

                if (vFlip)
                    sprLine ^= (h16 ? 15 : 7);

                tileIdx ^= (sprLine >> 3);

                unsigned patOffs = (tileIdx << 5) | ((sprLine & 7) << 2);

                for (int n = 0; n < 4; n++) {
                    int m = n;
                    if (hFlip)
                        m ^= 3;

                    uint8_t data = emuState.videoRam[patOffs + m];

                    if (!hFlip) {
                        if (priority || (lineGfx[idx] & (1 << 6)) == 0) {
                            unsigned colIdx = (data >> 4);

                            if (colIdx != 0)
                                lineGfx[idx] = colIdx | palette;
                        }
                        idx++;
                        idx &= 511;
                    }
                    if (priority || (lineGfx[idx] & (1 << 6)) == 0) {
                        unsigned colIdx = (data & 0xF);

                        if (colIdx != 0)
                            lineGfx[idx] = colIdx | palette;
                    }
                    idx++;
                    idx &= 511;

                    if (hFlip) {
                        if (priority || (lineGfx[idx] & (1 << 6)) == 0) {
                            unsigned colIdx = (data >> 4);

                            if (colIdx != 0)
                                lineGfx[idx] = colIdx | palette;
                        }
                        idx++;
                        idx &= 511;
                    }
                }
            }
        }
    }

    // Compose layers
    uint16_t *pd = &screen[line * VIDEO_WIDTH];
    idx          = 512 - 16;

    for (int i = 0; i < VIDEO_WIDTH; i++) {
        bool active       = idx < 320 && vActive;
        bool textPriority = (emuState.videoCtrl & VCTRL_TEXT_PRIORITY) != 0;
        bool textEnable   = (emuState.videoCtrl & VCTRL_TEXT_ENABLE) != 0;
        bool gfxEnable    = (emuState.videoCtrl & VCTRL_MODE_MASK) != 0;

        uint8_t colIdx = 0;
        if (!active) {
            if (textEnable)
                colIdx = lineText[idx];
        } else {
            if (textEnable && !textPriority)
                colIdx = lineText[idx];
            if (gfxEnable && (!textEnable || textPriority || (lineGfx[idx] & 0xF) != 0))
                colIdx = lineGfx[idx];
            if (textEnable && textPriority && (lineText[idx] & 0xF) != 0)
                colIdx = lineText[idx];
        }

        pd[i] = emuState.videoPalette[colIdx & 0x3F];
        idx   = (idx + 1) & 511;
    }
}
