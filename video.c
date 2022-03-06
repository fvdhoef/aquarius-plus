#include "video.h"
#include "emustate.h"

// Original Aquarius palette
// static const uint32_t palette[16] = {
//     0x101010, 0xf71010, 0x10f710, 0xf7ef10,
//     0x2121de, 0xf710f7, 0x31c6c6, 0xf7f7f7,
//     0xc6c6c6, 0x29adad, 0xc621c6, 0x42108c,
//     0xf7f773, 0x21ce42, 0xad2121, 0x313131};

// 2-Bit Aquarius palette
// optimized for a better color range.
// static const uint32_t palette[16] = {
//     0x000000, 0xff0000, 0x55ff55, 0xffaa00,
//     0x0000ff, 0xff00ff, 0x55aaff, 0xffffff,
//     0xaaaaaa, 0x0055aa, 0xaa00aa, 0x000055,
//     0xffff55, 0x55aa55, 0xaa0000, 0x555555};

// 2-Bit Aquarius palette
// optimized to match STOCK color.
// static const uint32_t palette[16] = {
//     0x000000, 0xff0000, 0x55ff55, 0xffff00,
//     0x0000ff, 0xff00ff, 0x00ffff, 0xffffff,
//     0xaaaaaa, 0x00aaaa, 0xaa00aa, 0x0000aa,
//     0xffffaa, 0x55aa55, 0xaa0000, 0x555555};

static uint8_t screen[VIDEO_WIDTH * VIDEO_HEIGHT];

const uint8_t *video_get_fb(void) {
    return screen;
}

enum {
    VCTRL_TEXT_ENABLE    = (1 << 0),
    VCTRL_MODE_OFF       = (0 << 1),
    VCTRL_MODE_TILEMAP   = (1 << 1),
    VCTRL_MODE_BITMAP    = (2 << 1),
    VCTRL_MODE_MASK      = (3 << 1),
    VCTRL_SPRITES_ENABLE = (1 << 3),
};

void video_draw_line(void) {
    int line = emustate.video_line;
    if (line < 0 || line >= VIDEO_HEIGHT)
        return;

    // Render text
    uint8_t line_text[VIDEO_WIDTH];
    for (int i = 0; i < VIDEO_WIDTH; i++) {
        // Draw text character
        uint8_t ch;
        uint8_t color;
        if (line >= 16 && line < 216 && i >= 16 && i < 336) {
            int row    = (line - 16) / 8;
            int column = (i - 16) / 8;
            ch         = emustate.screenram[row * 40 + column];
            color      = emustate.colorram[row * 40 + column];
        } else {
            ch    = emustate.screenram[0];
            color = emustate.colorram[0];
        }

        uint8_t charbm = emustate.charram[ch * 8 + (line & 7)];
        line_text[i]   = (charbm & (1 << (7 - (i & 7)))) ? (color >> 4) : (color & 0xF);
    }

    // Render bitmap/tile layer
    uint8_t line_bitmap[VIDEO_WIDTH];
    if ((emustate.video_ctrl & VCTRL_MODE_MASK) == VCTRL_MODE_BITMAP) {
        // Bitmap mode
        for (int i = 0; i < VIDEO_WIDTH; i++) {
            uint8_t color;

            if (line >= 16 && line < 216 && i >= 16 && i < 336) {
                int     row    = (line - 16) / 8;
                int     column = (i - 16) / 8;
                uint8_t col    = emustate.videoram[0x2000 + row * 40 + column];
                uint8_t bm     = emustate.videoram[0x0000 + (line - 16) * 40 + column];

                color = (bm & (1 << (7 - (i & 7)))) ? (col >> 4) : (col & 0xF);

            } else {
                color = 0;
            }
            line_bitmap[i] = color | (1 << 4);
        }
    } else {
        // Tile mode
        for (int i = 0; i < VIDEO_WIDTH; i++) {
            uint8_t color;

            if (line >= 16 && line < 216 && i >= 16 && i < 336) {
                color = 0;

                // int     row    = (line - 16) / 8;
                // int     column = (i - 16) / 8;
                // uint8_t col    = emustate.videoram[0x2000 + row * 40 + column];
                // uint8_t bm     = emustate.videoram[0x0000 + (line - 16) * 40 + column];

                // color = (bm & (1 << (7 - (i & 7)))) ? (col >> 4) : (col & 0xF);

            } else {
                color = 0;
            }
            line_bitmap[i] = color;
        }
    }

    // Compose layers
    uint8_t *pd = &screen[line * VIDEO_WIDTH];
    for (int i = 0; i < VIDEO_WIDTH; i++) {
        uint8_t colidx = 0;
        if (emustate.video_ctrl & VCTRL_TEXT_ENABLE) {
            colidx = line_text[i];
        }
        if ((emustate.video_ctrl & VCTRL_MODE_MASK) != VCTRL_MODE_OFF && colidx == 0) {
            colidx = line_bitmap[i];
        }

        uint8_t color = 0;
        switch (colidx >> 4) {
            case 0: color = emustate.video_palette_text[colidx & 0xF]; break;
            case 1: color = emustate.video_palette_tile[colidx & 0xF]; break;
            case 2: color = emustate.video_palette_sprite[colidx & 0xF]; break;
            default: break;
        }
        pd[i] = color;
    }
}
