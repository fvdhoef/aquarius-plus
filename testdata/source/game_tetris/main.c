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

static const char *tetrominoes = "ijlostz";

static char    cur_tetromino = 't';
static uint8_t cur_rot       = 0;
static uint8_t cur_posx      = 0;
static uint8_t cur_posy      = 0;

static uint8_t pressed_keys[8];

static char next_tetromino = 'j';

static char tmpstr[16];

static const uint8_t tetromino_i[4][16] = {
    {0, 0, 0, 0,
     16, 17, 17, 18,
     0, 0, 0, 0,
     0, 0, 0, 0},
    {0, 0, 19, 0,
     0, 0, 20, 0,
     0, 0, 20, 0,
     0, 0, 21, 0},
    {0, 0, 0, 0,
     0, 0, 0, 0,
     16, 17, 17, 18,
     0, 0, 0, 0},
    {0, 19, 0, 0,
     0, 20, 0, 0,
     0, 20, 0, 0,
     0, 21, 0, 0},
};

static const uint8_t tetromino_j[4][9] = {
    {0, 0, 0,
     25, 25, 25,
     0, 0, 25},
    {0, 25, 0,
     0, 25, 0,
     25, 25, 0},
    {25, 0, 0,
     25, 25, 25,
     0, 0, 0},
    {0, 25, 25,
     0, 25, 0,
     0, 25, 0},
};

static const uint8_t tetromino_l[4][9] = {
    {0, 0, 0,
     26, 26, 26,
     26, 0, 0},
    {26, 26, 0,
     0, 26, 0,
     0, 26, 0},
    {0, 0, 26,
     26, 26, 26,
     0, 0, 0},
    {0, 26, 0,
     0, 26, 0,
     0, 26, 26},
};

static const uint8_t tetromino_o[4] = {
    27, 27,
    27, 27};

static const uint8_t tetromino_s[4][9] = {
    {0, 24, 24,
     24, 24, 0,
     0, 0, 0},
    {0, 24, 0,
     0, 24, 24,
     0, 0, 24},
    {0, 0, 0,
     0, 24, 24,
     24, 24, 0},
    {24, 0, 0,
     24, 24, 0,
     0, 24, 0},
};

static const uint8_t tetromino_t[4][9] = {
    {0, 0, 0,
     22, 22, 22,
     0, 22, 0},
    {0, 22, 0,
     22, 22, 0,
     0, 22, 0},
    {0, 22, 0,
     22, 22, 22,
     0, 0, 0},
    {0, 22, 0,
     0, 22, 22,
     0, 22, 0},
};

static const uint8_t tetromino_z[4][9] = {
    {23, 23, 0,
     0, 23, 23,
     0, 0, 0},
    {0, 0, 23,
     0, 23, 23,
     0, 23, 0},
    {0, 0, 0,
     23, 23, 0,
     0, 23, 23},
    {0, 23, 0,
     23, 23, 0,
     23, 0, 0},
};

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

static void draw_tetromino(uint8_t i, uint8_t j, char tetromino, uint8_t rot) {
    rot &= 3;
    const uint8_t *p = NULL;

    uint8_t sz = 0;

    switch (tetromino) {
        default:
        case 'o':
            sz = 2;
            p  = tetromino_o;
            break;
        case 'j':
            sz = 3;
            p  = tetromino_j[rot];
            break;
        case 'l':
            sz = 3;
            p  = tetromino_l[rot];
            break;
        case 's':
            sz = 3;
            p  = tetromino_s[rot];
            break;
        case 't':
            sz = 3;
            p  = tetromino_t[rot];
            break;
        case 'z':
            sz = 3;
            p  = tetromino_z[rot];
            break;
        case 'i':
            sz = 4;
            p  = tetromino_i[rot];
            break;
    }

    for (uint8_t y = 0; y < sz; y++) {
        for (uint8_t x = 0; x < sz; x++) {
            uint8_t val = *(p++);
            if (val)
                set_tile(i + x, j + y, val);
        }
    }
}

static void set_tetromino_sprites(uint8_t x, uint8_t y, char tetromino, uint8_t rot) {
    rot &= 3;
    const uint8_t *p = NULL;

    uint8_t sz = 0;

    switch (tetromino) {
        default:
        case 'o':
            sz = 2;
            p  = tetromino_o;
            break;
        case 'j':
            sz = 3;
            p  = tetromino_j[rot];
            break;
        case 'l':
            sz = 3;
            p  = tetromino_l[rot];
            break;
        case 's':
            sz = 3;
            p  = tetromino_s[rot];
            break;
        case 't':
            sz = 3;
            p  = tetromino_t[rot];
            break;
        case 'z':
            sz = 3;
            p  = tetromino_z[rot];
            break;
        case 'i':
            sz = 4;
            p  = tetromino_i[rot];
            break;
    }

    IO_VSPRSEL = 0;

    for (uint8_t j = 0; j < sz; j++) {
        for (uint8_t i = 0; i < sz; i++) {
            uint8_t val = *(p++);
            if (val) {
                uint16_t sx = x + (i << 3);
                uint8_t  sy = y + (j << 3);

                IO_VSPRX_L  = sx & 0xFF;
                IO_VSPRX_H  = sx >> 8;
                IO_VSPRY    = sy;
                IO_VSPRIDX  = val;
                IO_VSPRATTR = 0x91;

                IO_VSPRSEL++;
            }
        }
    }
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
    draw_tetromino(27, 5, next_tetromino, 0);

    // Draw stats
    sprintf(tmpstr, "%7lu ", score);
    draw_text(25, 12, tmpstr);
    sprintf(tmpstr, "%7u ", level);
    draw_text(25, 15, tmpstr);
    sprintf(tmpstr, "%7u ", lines);
    draw_text(25, 18, tmpstr);
}

static void scankeys(void) {
    pressed_keys[0] = ~IO_KEYBOARD_COL0;
    pressed_keys[1] = ~IO_KEYBOARD_COL1;
    pressed_keys[2] = ~IO_KEYBOARD_COL2;
    pressed_keys[3] = ~IO_KEYBOARD_COL3;
    pressed_keys[4] = ~IO_KEYBOARD_COL4;
    pressed_keys[5] = ~IO_KEYBOARD_COL5;
    pressed_keys[6] = ~IO_KEYBOARD_COL6;
    pressed_keys[7] = ~IO_KEYBOARD_COL7;
}

static bool key_pressed(uint8_t scancode) {
    return (pressed_keys[scancode / 8] & (1 << (scancode & 7)));
}

static uint8_t getkeys(void) {
    uint8_t result = 0;
    if (key_pressed(KEY_A))
        result |= (1 << 0);
    if (key_pressed(KEY_D))
        result |= (1 << 1);
    if (key_pressed(KEY_W))
        result |= (1 << 2);
    if (key_pressed(KEY_S))
        result |= (1 << 3);
    if (key_pressed(KEY_N))
        result |= (1 << 4);
    if (key_pressed(KEY_M))
        result |= (1 << 5);
    return result;
}

int main(void) {
    // while (1) {
    //     scankeys();
    //     for (int i = 0; i < 8; i++) {
    //         printf("%02X ", pressed_keys[i]);
    //     }
    //     printf("\n");

    //     // printf("%02X\n", getkeys());
    // }

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

    // Disable all sprites
    for (uint8_t i = 0; i < 64; i++) {
        IO_VSPRSEL  = i;
        IO_VSPRATTR = 0;
    }

    // Initial screen drawing
    draw_static_screen();
    draw_dynamic_screen();

    // Switch video mode to tile mode
    IO_VCTRL = VCTRL_SPR_EN | VCTRL_MODE_TILE;

    bgtiles_dst  = vram_tiledata + 32 * 46;
    bgtiles_src  = vram_tiledata + 32 * 48;
    bgtiles_src2 = bgtiles_src;

    // for (int i = 0; i < 4; i++) {
    //     draw_tetromino(0, i * 4, 'j', i);
    // }

    uint8_t prev_keys = 0;

    while (1) {
        wait_vsync();
        scankeys();
        if (bgdelay == 0) {
            memcpy(bgtiles_dst, bgtiles_src2, 64);
        }

        set_tetromino_sprites((cur_posx + 11) << 3, (cur_posy + 2) << 3, cur_tetromino, cur_rot);

        uint8_t keys    = getkeys();
        uint8_t newkeys = ~prev_keys & keys;

        if (newkeys & (1 << 0)) {
            cur_posx--;
        }
        if (newkeys & (1 << 1)) {
            cur_posx++;
        }
        if (newkeys & (1 << 2)) {
            cur_posy--;
        }
        if (newkeys & (1 << 3)) {
            cur_posy++;
        }
        if (newkeys & (1 << 4)) {
            cur_rot = (cur_rot - 1) & 3;
        }
        if (newkeys & (1 << 5)) {
            cur_rot = (cur_rot + 1) & 3;
        }

        prev_keys = keys;

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
