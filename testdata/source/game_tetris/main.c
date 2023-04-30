#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "file_io.h"
#include "regs.h"

#define SHOWPERF

// Platfield dimensions
#define PLAYFIELD_W 10
#define PLAYFIELD_H 20
#define PLAYFIELD_YT 2
#define PLAYFIELD_YB (PLAYFIELD_YT + PLAYFIELD_H - 1)
#define PLAYFIELD_XL 11
#define PLAYFIELD_XR (PLAYFIELD_XL + PLAYFIELD_W - 1)

// Tetrominoes enum
enum {
    TM_I = 0,
    TM_J,
    TM_L,
    TM_O,
    TM_S,
    TM_T,
    TM_Z,
    TM_TOTAL,
};

// Tile data
extern const uint8_t tile_palette[32];
extern const uint8_t tile_data[];
extern const uint8_t tile_data_end[];

// VRAM pointers
static uint16_t *vram_tilemap  = (uint16_t *)0xC000;
static uint8_t  *vram_tiledata = (uint8_t *)0xE000;

// Game statistics
static uint32_t score = 0;
static uint8_t  level = 0;
static uint16_t lines = 0;

// Background animation variables
static uint8_t *bgtiles_dst;
static uint8_t *bgtiles_src;
static uint8_t *bgtiles_src2;
static uint8_t  bgtiles_idx = 0;
static uint8_t  bgdelay     = 0;

// Current tetromino variables
static char    cur_tetromino;
static uint8_t cur_rot;
static uint8_t cur_posx;
static uint8_t cur_posy;

// Current background tile
static uint8_t bg_tile = 13;

// Pressed keys array
static uint8_t pressed_keys[8];

// Next tetromino (shown in preview area)
static char next_tetromino;

// Random selection for next tetromino
static uint8_t tetromino_random = 0;

// Temporary string variables for drawing stats
static char tmpstr[16];
static char tmpstr2[16];

// Level speed curve (number of frames delay per gravity drop)
static const uint8_t speed_curve[21] = {52, 48, 44, 40, 36, 32, 27, 21, 16, 10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2};

// Keyboard bitmap
enum {
    KBM_LEFT       = (1 << 0),
    KBM_RIGHT      = (1 << 1),
    KBM_UP         = (1 << 2),
    KBM_DOWN       = (1 << 3),
    KBM_ROTATE_CCW = (1 << 4),
    KBM_ROTATE_CW  = (1 << 5),
};

// Tetromino I shape in 4 rotations
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

// Tetromino J shape in 4 rotations
static const uint8_t tetromino_j[4][9] = {
    {25, 0, 0,
     25, 25, 25,
     0, 0, 0},
    {0, 25, 25,
     0, 25, 0,
     0, 25, 0},
    {0, 0, 0,
     25, 25, 25,
     0, 0, 25},
    {0, 25, 0,
     0, 25, 0,
     25, 25, 0},
};

// Tetromino L shape in 4 rotations
static const uint8_t tetromino_l[4][9] = {
    {0, 0, 26,
     26, 26, 26,
     0, 0, 0},
    {0, 26, 0,
     0, 26, 0,
     0, 26, 26},
    {0, 0, 0,
     26, 26, 26,
     26, 0, 0},
    {26, 26, 0,
     0, 26, 0,
     0, 26, 0},
};

// Tetromino O shape (just 1 rotation)
static const uint8_t tetromino_o[4] = {
    27, 27,
    27, 27};

// Tetromino S shape in 4 rotations
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

// Tetromino T shape in 4 rotations
static const uint8_t tetromino_t[4][9] = {
    {0, 22, 0,
     22, 22, 22,
     0, 0, 0},
    {0, 22, 0,
     0, 22, 22,
     0, 22, 0},
    {0, 0, 0,
     22, 22, 22,
     0, 22, 0},
    {0, 22, 0,
     22, 22, 0,
     0, 22, 0},
};

// Tetromino Z shape in 4 rotations
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

// Offsets used for rotation
static const int8_t wall_kick_offsets_jlstz[8][5][2] = {
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}}, // 0->R
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}}, // R->0
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}}, // R->2
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}}, // 2->R
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}, // 2->L
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}}, // L->2
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}}, // L->0
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}, // 0->L
};

// Offsets used for rotation
static const int8_t wall_kick_offsets_i[8][5][2] = {
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, -1}, {+1, +2}}, // 0->R
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, +1}, {-1, -2}}, // R->0
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, +2}, {+2, -1}}, // R->2
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, -2}, {-2, +1}}, // 2->R
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, +1}, {-1, -2}}, // 2->L
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, -1}, {+1, +2}}, // L->2
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, -2}, {-2, +1}}, // L->0
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, +2}, {+2, -1}}, // 0->L
};

// Set tile 'tile_idx' at position i,j
static void set_tile(uint8_t i, uint8_t j, uint8_t tile_idx) {
    vram_tilemap[((uint16_t)j << 6) | i] = 0x1100 | tile_idx;
}

// Get tile at position i,j
static uint8_t get_tile(uint8_t i, uint8_t j) {
    if (j < PLAYFIELD_YT && (i >= PLAYFIELD_XL && i <= PLAYFIELD_XR))
        return bg_tile;

    return vram_tilemap[((uint16_t)j << 6) | i] & 0xFF;
}

// This function will draw the given tetromino at coordinate i,j. If check is set,
// instead of drawing the function will return if the tetromino can be drawn without
// intersecting with existing blocks.
static uint8_t draw_tetromino(uint8_t i, uint8_t j, uint8_t tetromino, uint8_t rot, bool check) {
    rot &= 3;
    const uint8_t *p = NULL;

    uint8_t sz = 0;

    switch (tetromino) {
        default:
        case TM_O:
            sz = 2;
            p  = tetromino_o;
            break;
        case TM_J:
            sz = 3;
            p  = tetromino_j[rot];
            break;
        case TM_L:
            sz = 3;
            p  = tetromino_l[rot];
            break;
        case TM_S:
            sz = 3;
            p  = tetromino_s[rot];
            break;
        case TM_T:
            sz = 3;
            p  = tetromino_t[rot];
            break;
        case TM_Z:
            sz = 3;
            p  = tetromino_z[rot];
            break;
        case TM_I:
            sz = 4;
            p  = tetromino_i[rot];
            break;
    }

    if (sz == 3) {
        i -= 1;
        j -= 1;
    }

    if (check) {
        for (uint8_t y = 0; y < sz; y++) {
            for (uint8_t x = 0; x < sz; x++) {
                uint8_t val = *(p++);
                if (val && get_tile(i + x, j + y) != bg_tile)
                    return 1;
            }
        }
        return 0;

    } else {
        for (uint8_t y = 0; y < sz; y++) {
            for (uint8_t x = 0; x < sz; x++) {
                uint8_t val = *(p++);
                if (val)
                    set_tile(i + x, j + y, val);
            }
        }
    }
    return 0;
}

// Update sprites used to display moving tetromino
static void set_tetromino_sprites(uint8_t x, uint8_t y, char tetromino, uint8_t rot) {
    rot &= 3;
    const uint8_t *p = NULL;

    uint8_t sz = 0;

    switch (tetromino) {
        default:
        case TM_O:
            sz = 2;
            p  = tetromino_o;
            break;
        case TM_J:
            sz = 3;
            p  = tetromino_j[rot];
            break;
        case TM_L:
            sz = 3;
            p  = tetromino_l[rot];
            break;
        case TM_S:
            sz = 3;
            p  = tetromino_s[rot];
            break;
        case TM_T:
            sz = 3;
            p  = tetromino_t[rot];
            break;
        case TM_Z:
            sz = 3;
            p  = tetromino_z[rot];
            break;
        case TM_I:
            sz = 4;
            p  = tetromino_i[rot];
            break;
    }

    if (sz == 3) {
        x -= 8;
        y -= 8;
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

// Draw string 'str' at i,j
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

// Draw static part of screen. Only drawn at start
static void draw_static_screen(void) {
    // Draw background
    for (uint8_t j = 0; j < 25; j++) {
        for (uint8_t i = 0; i < 40; i++) {
            set_tile(i, j, ((i ^ j) & 1) ? 47 : 46);
        }
    }

    // Draw playfield borders
    for (uint8_t j = 0; j < PLAYFIELD_H; j++) {
        set_tile(PLAYFIELD_XL - 1, j + PLAYFIELD_YT, 32);
        set_tile(PLAYFIELD_XR + 1, j + PLAYFIELD_YT, 32);
    }
    for (uint8_t i = 0; i < PLAYFIELD_W + 2; i++) {
        set_tile(i + PLAYFIELD_W, PLAYFIELD_YB + 1, 33);
    }

    // Draw playfield content
    for (uint8_t j = 0; j < PLAYFIELD_H; j++) {
        for (uint8_t i = 0; i < PLAYFIELD_W; i++) {
            set_tile(i + PLAYFIELD_XL, j + PLAYFIELD_YT, bg_tile);
        }
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

// sprintf is way too slow, so we make our own specialized function to update the scores
static void u16tos(uint16_t val) {
    __uitoa(val, tmpstr2, 10);

    const char *p = tmpstr2;
    while (*p)
        p++;

    tmpstr[8] = '\0';
    tmpstr[7] = ' ';

    int8_t idx = 6;
    while (p != tmpstr2) {
        tmpstr[idx--] = *(--p);
    }
    while (idx >= 0) {
        tmpstr[idx--] = ' ';
    }
}

static void u32tos(uint32_t val) {
    __ultoa(val, tmpstr2, 10);

    const char *p = tmpstr2;
    while (*p)
        p++;

    tmpstr[8] = '\0';
    tmpstr[7] = ' ';

    int8_t idx = 6;
    while (p != tmpstr2) {
        tmpstr[idx--] = *(--p);
    }
    while (idx >= 0) {
        tmpstr[idx--] = ' ';
    }
}

// Draw score/level/lines stats
static void draw_stats(void) {
    static uint32_t old_score = 0xFFFFFFFFUL;
    static uint8_t  old_level = 0xFF;
    static uint16_t old_lines = 0xFFFF;

    tmpstr[8] = '\0';
    tmpstr[7] = ' ';

    if (old_score != score) {
        old_score = score;
        u32tos(score);
        draw_text(25, 12, tmpstr);
    }

    if (old_level != level) {
        old_level = level;
        u16tos(level);
        draw_text(25, 15, tmpstr);
    }

    if (old_lines != lines) {
        old_lines = lines;
        u16tos(lines);
        draw_text(25, 18, tmpstr);
    }
}

// Draw tetromino preview content
static void draw_preview(void) {
    // Draw tetromino preview content
    for (uint8_t j = 0; j < 4; j++) {
        for (uint8_t i = 0; i < 4; i++) {
            set_tile(i + 27, j + 5, 1);
        }
    }

    uint8_t x = 28;
    uint8_t y = 7;
    if (next_tetromino == TM_O) {
        y = 6;
    } else if (next_tetromino == TM_I) {
        x = 27;
        y = 6;
    }

    draw_tetromino(x, y, next_tetromino, 0, false);
}

// Scan keyboard and store in pressed_keys
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

// Check if key with specified scancode is pressed
static bool key_pressed(uint8_t scancode) {
    return (pressed_keys[scancode / 8] & (1 << (scancode & 7)));
}

// Compose keys bitmap with only the keys used by the game
static uint8_t getkeys(void) {
    uint8_t result = 0;
    if (key_pressed(KEY_A))
        result |= KBM_LEFT;
    if (key_pressed(KEY_D))
        result |= KBM_RIGHT;
    if (key_pressed(KEY_W))
        result |= KBM_UP;
    if (key_pressed(KEY_S))
        result |= KBM_DOWN;
    if (key_pressed(KEY_N))
        result |= KBM_ROTATE_CCW;
    if (key_pressed(KEY_M))
        result |= KBM_ROTATE_CW;
    return result;
}

// Move tetromino left
static void move_left(void) {
    if (draw_tetromino(cur_posx - 1, cur_posy, cur_tetromino, cur_rot, true) == 0) {
        cur_posx--;
    }
}

// Move tetromino right
static void move_right(void) {
    if (draw_tetromino(cur_posx + 1, cur_posy, cur_tetromino, cur_rot, true) == 0) {
        cur_posx++;
    }
}

// Move tetromino down
static bool move_down(void) {
    if (draw_tetromino(cur_posx, cur_posy + 1, cur_tetromino, cur_rot, true) == 0) {
        cur_posy++;
        return true;
    } else {
        return false;
    }
}

// Rotate tetromino either clockwise (cw=true) or counter-clockwise (cw=false)
static void rotate(bool cw) {
    uint8_t new_rot = (cur_rot + (cw ? 1 : -1)) & 3;

    const int8_t(*wall_kick_offsets)[5];

    uint8_t idx = (cur_rot << 1) + (cw ? 0 : 1);
    if (cur_tetromino == TM_I) {
        wall_kick_offsets = wall_kick_offsets_i[idx];
    } else {
        wall_kick_offsets = wall_kick_offsets_jlstz[idx];
    }

    uint8_t max_i = cur_tetromino == TM_O ? 0 : 4;

    for (uint8_t i = 0; i <= max_i; i++) {
        uint8_t new_posx = cur_posx + wall_kick_offsets[i][0];
        uint8_t new_posy = cur_posy + wall_kick_offsets[i][1];
        if (draw_tetromino(new_posx, new_posy, cur_tetromino, new_rot, true) == 0) {
            cur_rot  = new_rot;
            cur_posx = new_posx;
            cur_posy = new_posy;
            break;
        }
    }
}

// Processing that needs to be done at the start of the frame
static void frame(void) {
#ifdef SHOWPERF
    IO_VPALSEL  = 0;
    IO_VPALDATA = 0;
#endif

    // Wait for end of frame (line 216)
    IO_VIRQLINE = 216;
    IO_IRQSTAT  = 2;
    while ((IO_IRQSTAT & 2) == 0) {
    }

#ifdef SHOWPERF
    IO_VPALSEL  = 0;
    IO_VPALDATA = 0xFF;
#endif

    // Scan keys
    scankeys();

    // Update screen during non-visible part
    if (bgdelay == 0) {
        // Animate background by updating 2 tile patterns
        memcpy(bgtiles_dst, bgtiles_src2, 64);
    }
    set_tetromino_sprites(cur_posx << 3, cur_posy << 3, cur_tetromino, cur_rot);

    // Animate background
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

// Switch to next tetromino piece
static void next_piece(void) {
    cur_posx       = (PLAYFIELD_XL + PLAYFIELD_XR) / 2;
    cur_posy       = PLAYFIELD_YT - 1;
    cur_rot        = 0;
    cur_tetromino  = next_tetromino;
    next_tetromino = tetromino_random;
    draw_preview();
}

// Lock current tetromino in place and switch to the next one
static void lock_piece(void) {
    // Lock in place
    draw_tetromino(cur_posx, cur_posy, cur_tetromino, cur_rot, false);

    // Switch to next piece
    next_piece();
}

// Our main function
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
    draw_preview();

    // Switch video mode to tile mode
    IO_VCTRL = VCTRL_SPR_EN | VCTRL_MODE_TILE;

    bgtiles_dst  = vram_tiledata + 32 * 46;
    bgtiles_src  = vram_tiledata + 32 * 48;
    bgtiles_src2 = bgtiles_src;

    uint8_t prev_keys = 0;

    uint8_t gravity_delay = speed_curve[level];

    uint8_t left_delay  = 0;
    uint8_t right_delay = 0;
    uint8_t down_delay  = 0;

    tetromino_random = IO_VLINE % 7;

    next_piece();

    while (1) {
        frame();

        // Move tetromino down when gravity delay expires
        if (gravity_delay == 0) {
            gravity_delay = speed_curve[level];
            if (!move_down()) {
                lock_piece();
            }
        } else {
            gravity_delay--;
        }

        // Update 'random' tetromino index
        tetromino_random++;
        if (tetromino_random >= TM_TOTAL)
            tetromino_random = 0;

        // Handle keyboard interaction
        uint8_t keys    = getkeys();
        uint8_t newkeys = ~prev_keys & keys;

        if (keys & KBM_LEFT) {
            if (left_delay == 0) {
                left_delay = 5;
                move_left();
            } else {
                left_delay--;
            }
        } else {
            left_delay = 0;
        }
        if (keys & KBM_RIGHT) {
            if (right_delay == 0) {
                right_delay = 5;
                move_right();
            } else {
                right_delay--;
            }
        } else {
            right_delay = 0;
        }
        if (keys & KBM_UP) {
            if (cur_posy > PLAYFIELD_YT - 1)
                cur_posy--;
        }
        if (keys & KBM_DOWN) {
            if (down_delay == 0) {
                down_delay = 3;
                if (move_down()) {
                    gravity_delay = speed_curve[level];
                    score++;
                } else {
                    lock_piece();
                }
            } else {
                down_delay--;
            }
        } else {
            down_delay = 0;
        }
        if (newkeys & KBM_ROTATE_CCW) {
            rotate(false);
        }
        if (newkeys & KBM_ROTATE_CW) {
            rotate(true);
        }

        draw_stats();

        // Keep track of keys pressed during this round
        prev_keys = keys;
    }

    // return 0;
}
