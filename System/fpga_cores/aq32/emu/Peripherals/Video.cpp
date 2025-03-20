#include "Video.h"
#include "EmuState.h"

#define BGCOL (0x6B7A82)
#define FGCOL (0xFBF6EF)

static const uint32_t palette[16] = {
    0x000000,
    0xC01C28,
    0x26A269,
    0xA2734C,
    0x12488B,
    0xA347BA,
    0x2AA1B3,
    0xD0CFCC,
    0x5E5C64,
    0xF66151,
    0x33D17A,
    0xE9AD0C,
    0x2A7BDE,
    0xC061CB,
    0x33C7DE,
    0xFFFFFF,
};

void Video::init(void) {
    gfx_mode = false;
}

void Video::render(void *pixels, int pitch) {
    if (!gfx_mode) {
        for (int row = 0; row < ((VIDEO_HEIGHT + 15) / 16); row++) {
            for (int col = 0; col < (VIDEO_WIDTH / 8); col++) {
                extern uint8_t IBM_VGA_8x16_bin[];
                uint16_t       val  = textram[row * (VIDEO_WIDTH / 8) + col];
                uint8_t        ch   = val & 0xFF;
                uint8_t        attr = val >> 8;

                for (int j = 0; j < 16; j++) {
                    int     y    = row * 16 + j;
                    uint8_t bits = IBM_VGA_8x16_bin[ch * 16 + j];

                    if (y >= VIDEO_HEIGHT)
                        break;

                    for (int i = 0; i < 8; i++) {
                        ((uint32_t *)((uintptr_t)pixels + y * pitch))[col * 8 + i] =
                            palette[(bits & (1 << (7 - i))) ? (attr & 0xF) : (attr >> 4)];
                    }
                }
            }
        }
    } else {
        const uint16_t *ram = (const uint16_t *)emuState.ram;

        unsigned addr = fb_addr >> 1;

        unsigned cy = ~(cursor_y - 1) & 0xFFF;

        for (int j = 0; j < VIDEO_HEIGHT; j++) {
            uint32_t *pd = (uint32_t *)((uintptr_t)pixels + j * pitch);
            unsigned  cx = ~cursor_x & 0xFFF;

            for (int i = 0; i < VIDEO_WIDTH; i++) {
                uint16_t val = ram[(addr++) & (RAM_SIZE / 2 - 1)];
                cx           = (cx + 1) & 0xFFF;

                if (cx < 32 && cy < 32) {
                    auto cval = cursor_ram[cy * 32 + cx];
                    if (cval & 0x8000)
                        val = cval;
                }

                unsigned r = (val >> 10) & 31;
                unsigned g = (val >> 5) & 31;
                unsigned b = (val >> 0) & 31;

                r = (r << 3) | (r >> 2);
                g = (g << 3) | (g >> 2);
                b = (b << 3) | (b >> 2);

                *(pd++) = (r << 16) | (g << 8) | b;
            }

            cy = (cy + 1) & 0xFFF;
        }
    }
}

void Video::writeReg(unsigned reg, uint32_t val) {
    if (reg == 1) {
        gfx_mode = (val & 1) != 0;
    } else if (reg == 2) {
        fb_addr = val;
    } else if (reg == 4) {
        cursor_x = val & 0xFFF;
        cursor_y = (val >> 16) & 0xFFF;
    } else {
        printf("Video::writeReg(%u, 0x%08X)\n", reg, val);
    }
}

uint32_t Video::readReg(unsigned reg) {
    if (reg == 1) {
        return gfx_mode ? 1 : 0;
    } else if (reg == 2) {
        return fb_addr;
    } else if (reg == 4) {
        return (cursor_y << 16) | cursor_x;
    } else {
        printf("Video::readReg(%u)\n", reg);
    }
    return 0;
}
