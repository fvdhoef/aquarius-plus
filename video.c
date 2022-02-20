#include "video.h"
#include "emustate.h"

extern unsigned char charrom_bin[2048]; // Character ROM contents

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

static const uint8_t text_palette[16] = {
    0x00, 0x03, 0x1D, 0x1F,
    0x30, 0x33, 0x29, 0x3F,
    0x2A, 0x28, 0x22, 0x20,
    0x2F, 0x19, 0x02, 0x15};

const uint8_t *video_get_fb(void) {
    return screen;
}

void video_draw_line(void) {
    int line = emustate.video_line;
    if (line < 0 || line >= VIDEO_HEIGHT)
        return;

    uint8_t *pd = &screen[line * VIDEO_WIDTH];

    for (int i = 0; i < VIDEO_WIDTH; i++) {

        // Draw text character
        uint8_t ch;
        uint8_t color;
        if (line >= 16 && line < 208 && i >= 16 && i <= 336) {
            int row    = (line - 16) / 8;
            int column = (i - 16) / 8;
            ch         = emustate.textram[(row + 1) * 40 + column];
            color      = emustate.colorram[(row + 1) * 40 + column];
        } else {
            ch    = emustate.textram[0];
            color = emustate.colorram[0];
        }

        uint8_t fgcol  = text_palette[color >> 4];
        uint8_t bgcol  = text_palette[color & 0xF];
        uint8_t charbm = charrom_bin[ch * 8 + (line & 7)];

        pd[i] = (charbm & (1 << (7 - (i & 7)))) ? fgcol : bgcol;
    }
}
