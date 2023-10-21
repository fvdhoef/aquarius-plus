#include "Video.h"
#include "EmuState.h"

Video::Video() {
}

void Video::drawLine() {
    int line = emuState.videoLine;
    if (line < 0 || line >= VIDEO_HEIGHT)
        return;

    bool vActive = line >= 16 && line < 216;

    // Render text
    uint8_t lineText[1024];
    {
        unsigned idx = 1024 - 32;
        for (int i = 0; i < VIDEO_WIDTH; i++) {
            // Draw text character
            unsigned addr   = 0;
            int      pixidx = i;

            if (vActive && idx < 640) {
                int row = (line - 16) / 8;
                if (emuState.videoCtrl & VCTRL_80_COLUMNS) {
                    int column = (i - 32) / 8;
                    addr       = row * 80 + column;
                } else {
                    int column = ((i / 2) - 16) / 8;
                    addr       = row * 40 + column;
                    pixidx /= 2;
                }

            } else if (emuState.videoCtrl & VCTRL_REMAP_BORDER_CHAR) {
                addr = (emuState.videoCtrl & VCTRL_80_COLUMNS) ? 0x7FF : 0x3FF;
            }

            uint8_t ch    = emuState.screenRam[addr];
            uint8_t color = emuState.colorRam[addr];

            uint8_t charBm = emuState.charRam[ch * 8 + (line & 7)];

            lineText[idx] = (charBm & (1 << (7 - (pixidx & 7)))) ? (color >> 4) : (color & 0xF);
            idx           = (idx + 1) & 1023;
        }
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
                unsigned idx      = (-(emuState.videoScrX & 7)) & 511;
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

                unsigned idx = sprX;

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
    {
        uint16_t *pd  = &screen[line * VIDEO_WIDTH];
        unsigned  idx = 1024 - 32;

        for (int i = 0; i < VIDEO_WIDTH; i++) {
            bool active       = idx < 640 && vActive;
            bool textPriority = (emuState.videoCtrl & VCTRL_TEXT_PRIORITY) != 0;
            bool textEnable   = (emuState.videoCtrl & VCTRL_TEXT_ENABLE) != 0;

            uint8_t colIdx = 0;
            if (!active) {
                if (textEnable)
                    colIdx = lineText[idx];
            } else {
                if (textEnable)
                    colIdx = lineText[idx];

                if (textEnable && !textPriority)
                    colIdx = lineText[idx];
                if (!textEnable || textPriority || (lineGfx[idx / 2] & 0xF) != 0)
                    colIdx = lineGfx[idx / 2];
                if (textEnable && textPriority && (lineText[idx] & 0xF) != 0)
                    colIdx = lineText[idx];
            }

            pd[i] = emuState.videoPalette[colIdx & 0x3F];
            idx   = (idx + 1) & 1023;
        }
    }
}
