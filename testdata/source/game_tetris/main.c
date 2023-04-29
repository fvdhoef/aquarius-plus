#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "file_io.h"
#include "regs.h"

extern const uint8_t tile_palette[32];
extern const uint8_t tile_data[];
extern const uint8_t tile_data_end[];

static uint16_t *vram_tilemap  = (uint16_t *)0xC000;
static uint8_t  *vram_tiledata = (uint8_t *)0xE000;

static uint8_t  level = 1;
static uint16_t lines = 0;
static uint32_t score = 0;

static uint8_t *bgtiles_dst;
static uint8_t *bgtiles_src;
static uint8_t *bgtiles_src2;
static uint8_t  bgtiles_idx = 0;
static uint8_t  bgdelay     = 0;

static char tmpstr[16];

static inline void wait_vsync(void) {
    // Wait for line 216
    IO_VIRQLINE = 216;
    IO_IRQSTAT  = 2;
    while ((IO_IRQSTAT & 2) == 0) {
    }
}

static void set_tile(uint8_t i, uint8_t j, uint8_t idx) {
    vram_tilemap[((uint16_t)j << 6) | i] = 0x1100 | idx;
}

static void draw_text(uint8_t i, uint8_t j, const char *str) {
    while (*str) {
        uint8_t val = *(str++);
        uint8_t idx = 1;

        if (val >= '0' && val <= '9') {
            idx = 64 + (val - '0');
        } else if (val >= 'A' && val <= 'Z') {
            idx = 80 + (val - 'A');
        } else if (val >= 'a' && val <= 'z') {
            idx = 80 + (val - 'a');
        }
        set_tile(i, j, idx);
        i++;
    }
}

static void draw_static_screen(void) {
    // Draw background
    for (uint8_t j = 0; j < 25; j++) {
        for (uint8_t i = 0; i < 40; i++) {
            set_tile(i, j, ((i ^ j) & 1) ? 47 : 46);
        }
    }

    // Draw playfield borders
    for (uint8_t j = 0; j < 20; j++) {
        set_tile(10, j + 2, 32);
        set_tile(21, j + 2, 32);
    }
    for (uint8_t i = 0; i < 12; i++) {
        set_tile(i + 10, 22, 33);
    }

    // Draw tetromino preview borders
    set_tile(26, 4, 33);
    set_tile(27, 4, 34);
    set_tile(28, 4, 35);
    set_tile(29, 4, 35);
    set_tile(30, 4, 36);
    set_tile(31, 4, 33);

    set_tile(26, 5, 40);
    set_tile(26, 6, 41);
    set_tile(26, 7, 41);
    set_tile(26, 8, 42);

    set_tile(31, 5, 43);
    set_tile(31, 6, 44);
    set_tile(31, 7, 44);
    set_tile(31, 8, 45);

    set_tile(26, 9, 33);
    set_tile(27, 9, 37);
    set_tile(28, 9, 38);
    set_tile(29, 9, 38);
    set_tile(30, 9, 39);
    set_tile(31, 9, 33);

    // Draw texts
    draw_text(24, 2, " MARATHON ");
    draw_text(25, 11, "  SCORE ");
    draw_text(25, 14, "  LEVEL ");
    draw_text(25, 17, "  LINES ");
}

static void draw_dynamic_screen(void) {
    // Draw playfield content
    for (uint8_t j = 0; j < 20; j++) {
        for (uint8_t i = 0; i < 10; i++) {
            set_tile(i + 11, j + 2, 13);
        }
    }

    // Draw tetromino preview content
    for (uint8_t j = 0; j < 4; j++) {
        for (uint8_t i = 0; i < 4; i++) {
            set_tile(i + 27, j + 5, 1);
        }
    }

    // Draw stats
    sprintf(tmpstr, "%7lu ", 201792);
    draw_text(25, 12, tmpstr);
    sprintf(tmpstr, "%7u ", level);
    draw_text(25, 15, tmpstr);
    sprintf(tmpstr, "%7u ", lines);
    draw_text(25, 18, tmpstr);
}

int main(void) {
    // Map video RAM to 0xC000
    IO_BANK3 = 20;

    // Set tile data palette
    for (uint8_t i = 0; i < 32; i++) {
        IO_VPALSEL  = 32 + i;
        IO_VPALDATA = tile_palette[i];
    }
    // Copy tile data into VRAM
    memcpy(vram_tiledata, tile_data, tile_data_end - tile_data);

    // Init video
    IO_VSCRX_L = 0;
    IO_VSCRX_H = 0;
    IO_VSCRY   = 0;

    // Initial screen drawing
    draw_static_screen();
    draw_dynamic_screen();

    // Switch video mode to tile mode
    IO_VCTRL = VCTRL_MODE_TILE;

    bgtiles_dst  = vram_tiledata + 32 * 46;
    bgtiles_src  = vram_tiledata + 32 * 48;
    bgtiles_src2 = bgtiles_src;

    while (1) {
        wait_vsync();
        if (bgdelay == 0) {
            memcpy(bgtiles_dst, bgtiles_src2, 64);
        }
        if (bgdelay == 0) {
            bgdelay = 2;

            if (bgtiles_idx == 7) {
                bgtiles_idx  = 0;
                bgtiles_src2 = bgtiles_src;
            } else {
                bgtiles_idx++;
                bgtiles_src2 += 64;
            }
        } else {
            bgdelay--;
        }
    }

    // return 0;
}
