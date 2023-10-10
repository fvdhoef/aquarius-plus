#include <aqplus.h>

// #define SHOWPERF

// Playfield dimensions
#define PLAYFIELD_W 10
#define PLAYFIELD_H 20
#define PLAYFIELD_YT 2
#define PLAYFIELD_YB (PLAYFIELD_YT + PLAYFIELD_H - 1)
#define PLAYFIELD_XL 11
#define PLAYFIELD_XR (PLAYFIELD_XL + PLAYFIELD_W - 1)

#define MARKER_TILE (1)
#define DELAY_LEFT_RIGHT (5)
#define DELAY_DOWN (3)

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
static uint32_t score;
static uint8_t  level;
static uint16_t lines;
static bool     gameover;
static bool     quit;

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
static uint8_t next_tetromino;

// Random selection for next tetromino
static uint8_t tetromino_random = 0;

// Temporary string variables for drawing stats
static char tmpstr[16];
static char tmpstr2[16];

// Level speed curve (number of frames delay per gravity drop)
static const uint8_t speed_curve[21] = {52, 48, 44, 40, 36, 32, 27, 21, 16, 10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2};

static const uint16_t gameover_playfield[20][10] = {
    {0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09},
    {0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09},
    {0x09, 0x4A, 0x4B, 0x4C, 0x4C, 0x4C, 0x4C, 0x24B, 0x24A, 0x09},
    {0x09, 0x6A, 0x6B, 0x6C, 0x6C, 0x6C, 0x6C, 0x26B, 0x26A, 0x09},
    {0x09, 0x6D, 0x6E, 0x0F + 'G', 0x0F + 'A', 0x0F + 'M', 0x0F + 'E', 0x26E, 0x26D, 0x09},
    {0x09, 0x6D, 0x6E, 0x01, 0x01, 0x01, 0x01, 0x26E, 0x26D, 0x09},
    {0x09, 0x6D, 0x6E, 0x0F + 'O', 0x0F + 'V', 0x0F + 'E', 0x0F + 'R', 0x26E, 0x26D, 0x09},
    {0x09, 0x46A, 0x46B, 0x46C, 0x46C, 0x46C, 0x46C, 0x66B, 0x66A, 0x09},
    {0x09, 0x44A, 0x44B, 0x44C, 0x44C, 0x44C, 0x44C, 0x64B, 0x64A, 0x09},
    {0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09},
    {0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x01, 0x01, 0x0F + 'P', 0x0F + 'L', 0x0F + 'E', 0x0F + 'A', 0x0F + 'S', 0x0F + 'E', 0x01, 0x01},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x01, 0x01, 0x01, 0x0F + 'T', 0x0F + 'R', 0x0F + 'Y', 0x01, 0x01, 0x01, 0x01},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x01, 0x01, 0x0F + 'A', 0x0F + 'G', 0x0F + 'A', 0x0F + 'I', 0x0F + 'N', 0x4F, 0x01, 0x01},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
};

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
typedef struct {
    int8_t x, y;
} offset_t;

static const offset_t wall_kick_offsets_jlstz[8][5] = {
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}, // CCW:0->L
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}}, // CW :0->R
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}}, // CCW:R->0
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}}, // CW :R->2
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}}, // CCW:2->R
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}, // CW :2->L
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}}, // CCW:L->2
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}}, // CW :L->0
};

// Offsets used for rotation
static const offset_t wall_kick_offsets_i[8][5] = {
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, +2}, {+2, -1}}, // CCW:0->L
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, -1}, {+1, +2}}, // CW :0->R
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, +1}, {-1, -2}}, // CCW:R->0
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, +2}, {+2, -1}}, // CW :R->2
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, -2}, {-2, +1}}, // CCW:2->R
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, +1}, {-1, -2}}, // CW :2->L
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, -1}, {+1, +2}}, // CCW:L->2
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, -2}, {-2, +1}}, // CW :L->0
};

// Set tile 'tile_idx' at position i,j
static void set_tile(uint8_t i, uint8_t j, uint8_t tile_idx) {
    vram_tilemap[((uint16_t)j << 6) | i] = 0x1100 | tile_idx;
}

static void set_tile2(uint8_t i, uint8_t j, uint16_t val) {
    vram_tilemap[((uint16_t)j << 6) | i] = 0x1100 | val;
}

// Get tile at position i,j
static uint8_t get_tile(uint8_t i, uint8_t j) {
    if (j < PLAYFIELD_YT && (i >= PLAYFIELD_XL && i <= PLAYFIELD_XR))
        return bg_tile;

    return vram_tilemap[((uint16_t)j << 6) | i] & 0xFF;
}

// This function will draw the given tetromino at coordinate i,j. If check is set,
// instead of drawing the function will return if the tetromino can be drawn without
// intersecting with existing blocks (returns false on collision).
static bool draw_tetromino(uint8_t i, uint8_t j, uint8_t tetromino, uint8_t rot, bool check, bool lock) {
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
    } else if (sz == 4) {
        i -= 1;
        j -= 1;
    }

    if (check) {
        for (uint8_t y = 0; y < sz; y++) {
            for (uint8_t x = 0; x < sz; x++) {
                uint8_t val = *(p++);
                if (val && get_tile(i + x, j + y) != bg_tile)
                    return false;
            }
        }
        return true;

    } else {
        for (uint8_t y = 0; y < sz; y++) {
            for (uint8_t x = 0; x < sz; x++) {
                uint8_t val = *(p++);
                if (val) {
                    uint8_t jy = j + y;
                    if (jy < PLAYFIELD_YT) {
                        return false;
                    }
                    set_tile(i + x, jy, lock ? MARKER_TILE : val);
                }
            }
        }
        return true;
    }
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
    } else if (sz == 4) {
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
        x = 28;
        y = 6;
    }

    draw_tetromino(x, y, next_tetromino, 0, false, false);
}

static uint8_t get_joystick(void) {
    uint8_t old_addr = IO_PSG1ADDR;
    IO_PSG1ADDR      = 14;
    uint8_t joyval   = IO_PSG1DATA;
    IO_PSG1ADDR      = old_addr;
    return joyval;
}

// Compose keys bitmap with only the keys used by the game
static uint8_t getkeys(void) {
    uint8_t result = 0;
    if (kb_pressing(KEY_A) || kb_pressing(KEY_LEFT))
        result |= KBM_LEFT;
    if (kb_pressing(KEY_D) || kb_pressing(KEY_RIGHT))
        result |= KBM_RIGHT;
    if (kb_pressing(KEY_W) || kb_pressing(KEY_UP))
        result |= KBM_UP;
    if (kb_pressing(KEY_S) || kb_pressing(KEY_DOWN))
        result |= KBM_DOWN;
    if (kb_pressing(KEY_N) || kb_pressing(KEY_Z))
        result |= KBM_ROTATE_CCW;
    if (kb_pressing(KEY_M) || kb_pressing(KEY_X))
        result |= KBM_ROTATE_CW;

    uint8_t joyval = ~get_joystick();
    if (joyval & (1 << 0))
        result |= KBM_DOWN;
    if (joyval & (1 << 1))
        result |= KBM_RIGHT;
    if (joyval & (1 << 2))
        result |= KBM_UP;
    if (joyval & (1 << 3))
        result |= KBM_LEFT;
    if (joyval & (1 << 6))
        result |= KBM_ROTATE_CW;
    if (joyval & (1 << 7))
        result |= KBM_ROTATE_CCW;

    return result;
}

// Move tetromino left
static void move_left(void) {
    if (draw_tetromino(cur_posx - 1, cur_posy, cur_tetromino, cur_rot, true, false)) {
        cur_posx--;
    }
}

// Move tetromino right
static void move_right(void) {
    if (draw_tetromino(cur_posx + 1, cur_posy, cur_tetromino, cur_rot, true, false)) {
        cur_posx++;
    }
}

// Move tetromino down
static bool move_down(void) {
    if (draw_tetromino(cur_posx, cur_posy + 1, cur_tetromino, cur_rot, true, false)) {
        cur_posy++;
        return true;
    } else {
        return false;
    }
}

// Rotate tetromino either clockwise (cw=true) or counter-clockwise (cw=false)
static void rotate(bool cw) {
    uint8_t new_rot = (cur_rot + (cw ? 1 : -1)) & 3;

    uint8_t         idx               = (cur_rot << 1) | (cw ? 1 : 0);
    const offset_t *wall_kick_offsets = (cur_tetromino == TM_I) ? wall_kick_offsets_i[idx] : wall_kick_offsets_jlstz[idx];
    uint8_t         max_i             = cur_tetromino == TM_O ? 0 : 4;

    for (uint8_t i = 0; i <= max_i; i++) {
        uint8_t new_posx = cur_posx + wall_kick_offsets[i].x;
        uint8_t new_posy = cur_posy + wall_kick_offsets[i].y;
        if (draw_tetromino(new_posx, new_posy, cur_tetromino, new_rot, true, false)) {
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
    video_wait_eof();

    pt3play_play();

#ifdef SHOWPERF
    IO_VPALSEL  = 0;
    IO_VPALDATA = 0xFF;
#endif

    // Scan keys
    kb_scan();

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

// Check if given line is full
static bool check_line(uint8_t line) {
    for (uint8_t i = 0; i < PLAYFIELD_W; i++) {
        if (get_tile(i + PLAYFIELD_XL, line + PLAYFIELD_YT) == bg_tile)
            return false;
    }
    return true;
}

// Check for full lines
static void check_lines(void) {
    uint8_t lines_full = 0;

    for (uint8_t j = 0; j < PLAYFIELD_H; j++) {
        if (check_line(j)) {
            for (uint8_t i = 0; i < PLAYFIELD_W; i++) {
                set_tile(i + PLAYFIELD_XL, j + PLAYFIELD_YT, MARKER_TILE);
            }
            lines_full++;
        }
    }

    if (lines_full == 0) {
        return;
    }

    // Small delay
    for (uint8_t i = 0; i < 5; i++)
        frame();

    // Remove all marked lines
    int8_t line = PLAYFIELD_H - 1;
    while (line >= 0) {
        if (get_tile(PLAYFIELD_XL, line + PLAYFIELD_YT) == MARKER_TILE) {
            // Remove line by copying all lines above it one line down
            for (int8_t j = line; j >= 0; j--) {
                for (uint8_t i = 0; i < PLAYFIELD_W; i++) {
                    set_tile(PLAYFIELD_XL + i, PLAYFIELD_YT + j, get_tile(PLAYFIELD_XL + i, PLAYFIELD_YT + j - 1));
                }
            }
            lines++;
            if (lines % 10 == 0) {
                level++;
                if (level > 20)
                    level = 20;
            }
            frame();

        } else {
            line--;
        }
    }

    switch (lines_full) {
        case 1: score += 40 * (level + 1); break;
        case 2: score += 100 * (level + 1); break;
        case 3: score += 300 * (level + 1); break;
        case 4: score += 1200 * (level + 1); break;
    }
    if (score > 9999999)
        score = 9999999;
}

// Lock current tetromino in place and switch to the next one
static void lock_piece(void) {
    // Lock in place
    if (!draw_tetromino(cur_posx, cur_posy, cur_tetromino, cur_rot, false, true)) {
        gameover = true;
    }

    // Hide sprites
    IO_VCTRL &= ~VCTRL_SPR_EN;

    // Small delay
    for (uint8_t i = 0; i < 5; i++)
        frame();
    draw_tetromino(cur_posx, cur_posy, cur_tetromino, cur_rot, false, false);

    // Switch to next piece
    next_piece();

    // Show sprites
    IO_VCTRL |= VCTRL_SPR_EN;

    // Check for full lines
    check_lines();
}

static void init(void) {
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

    bgtiles_dst  = vram_tiledata + 32 * 46;
    bgtiles_src  = vram_tiledata + 32 * 48;
    bgtiles_src2 = bgtiles_src;

    tetromino_random = IO_VLINE % 7;

    // Draw background
    for (uint8_t j = 0; j < 25; j++) {
        for (uint8_t i = 0; i < 40; i++) {
            set_tile(i, j, ((i ^ j) & 1) ? 47 : 46);
        }
    }
}

static void play_marathon(void) {
    uint8_t prev_keys = 0;

    score    = 0;
    level    = 0;
    lines    = 0;
    gameover = false;

    uint8_t gravity_delay = speed_curve[level];
    uint8_t left_delay    = 0;
    uint8_t right_delay   = 0;
    uint8_t down_delay    = 0;

    cur_tetromino = tetromino_random;

    // Disable video
    // IO_VCTRL = 0;

    // Initial screen drawing
    draw_static_screen();
    draw_preview();

    // Switch video mode to tile mode
    IO_VCTRL = VCTRL_SPR_EN | VCTRL_MODE_TILE;
    next_piece();

    bool wait_down_release = false;

    while (!gameover) {
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
                left_delay = DELAY_LEFT_RIGHT;
                move_left();
            } else {
                left_delay--;
            }
        } else {
            left_delay = 0;
        }
        if (keys & KBM_RIGHT) {
            if (right_delay == 0) {
                right_delay = DELAY_LEFT_RIGHT;
                move_right();
            } else {
                right_delay--;
            }
        } else {
            right_delay = 0;
        }
        // if (keys & KBM_UP) {
        //     if (cur_posy > PLAYFIELD_YT - 1)
        //         cur_posy--;
        //     return;
        // }
        if (keys & KBM_DOWN) {
            if (!wait_down_release) {
                if (down_delay == 0) {
                    down_delay = 3;
                    if (move_down()) {
                        gravity_delay = speed_curve[level];
                        score++;
                    } else {
                        lock_piece();
                        wait_down_release = true;
                    }
                } else {
                    down_delay--;
                }
            }
        } else {
            wait_down_release = false;
            down_delay        = 0;
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

    // Disable sprites
    IO_VCTRL = VCTRL_MODE_TILE;

    // Animation

    // Draw playfield content
    for (int8_t j = PLAYFIELD_H - 1; j >= 0; j--) {
        for (uint8_t i = 0; i < PLAYFIELD_W; i++) {
            set_tile(i + PLAYFIELD_XL, j + PLAYFIELD_YT, 31);
        }
        frame();
        frame();
    }
    for (uint8_t j = 0; j < PLAYFIELD_H; j++) {
        for (uint8_t i = 0; i < PLAYFIELD_W; i++) {
            set_tile2(i + PLAYFIELD_XL, j + PLAYFIELD_YT, gameover_playfield[j][i] | 0x1100);
        }
        frame();
        frame();
    }

    bool blink = false;
    bool wait  = true;
    while (wait) {
        // Short blink delay
        for (int i = 0; i < 20; i++) {
            frame();

            // Quit game on CTRL-C (or ESCAPE)
            if (kb_pressed(KEY_C) && kb_pressed(KEY_CTRL)) {
                quit = true;
                wait = false;
            }

            // Start a new game when one of the game keys is pressed
            uint8_t keys = getkeys();
            if (~prev_keys & keys)
                wait = false;
            prev_keys = keys;
        }
        set_tile2(7 + PLAYFIELD_XL, 17 + PLAYFIELD_YT, blink ? 0x4F : 0x01);
        blink = !blink;
    }
}

int main(void) {
    uint8_t iobank3_old = IO_BANK3;

    init();

    extern const unsigned char __21_2F_pt3[];
    pt3play_init(__21_2F_pt3);

    while (!quit)
        play_marathon();

    IO_BANK3 = iobank3_old;
    IO_VCTRL = VCTRL_TEXT_EN;

    return 0;
}
