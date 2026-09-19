#pragma once

#include "common.h"
#include "scr_code/editbuf.h"
#include "aq8lua/aq8lua.h"

enum {
    MODE_CODE = 0,
    MODE_SPRITE,
    MODE_MAP,
    MODE_SFX,
    MODE_MUSIC,
};

#define MOUSE_SPR_POINTER   0
#define MOUSE_SPR_HAND      1
#define MOUSE_SPR_IBEAM     2
#define MOUSE_SPR_CROSSHAIR 3

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t buttons;
    uint8_t clicked_buttons;
    int8_t  wheel;
} mouse_event_t;

typedef struct {
    uint8_t  speed;
    uint8_t  loop_start;
    uint8_t  loop_end;
    uint16_t notes[32];
} sfx_t;

enum {
    TOOL_PIXEL,
    TOOL_LINE,
    TOOL_FILL,
    TOOL_SELECT,
    TOOL_ROTATE,
    TOOL_COLORPICK,
    TOOL_CIRCLE,
    TOOL_RECT,
    TOOL_STAMP,
    TOOL_HFLIP,
    TOOL_VFLIP,
    TOOL_DELETE,
};

typedef struct {
    uint8_t spr_idx;
    uint8_t color;
    uint8_t tool;
} sprite_edit_t;

typedef struct {
    uint8_t sfx_idx;
    uint8_t octave;
    uint8_t volume;
    uint8_t waveform;
    uint8_t effect;
    uint8_t cursor_row;
    uint8_t cursor_col;
} sfx_edit_t;

typedef struct {
    editbuf_t *editbuf;
    location_t loc_cursor;
    location_t loc_selection;
    int        scr_first_line;
    int        scr_first_pos;
    location_t loc_selection_from;
    location_t loc_selection_to;
} code_edit_t;

typedef struct {
    bool          editing;
    uint8_t       mode;
    mouse_event_t mouse_ev;
    uint8_t       mouse_spr;
    char          status_text[51];
    char          message[51];
    uint8_t       message_tick;
    uint16_t      modifiers;
    sprite_edit_t spr_edit;
    sfx_edit_t    sfx_edit;
    code_edit_t   code_edit;
} edit_state_t;

typedef struct {
    uint32_t sprites[16 * 16 * 8];
    sfx_t    sfx[64];
} data_state_t;

typedef struct {
    int     x;
    int     y;
    uint8_t color;
    void   *lua_state;
    int16_t camera_x;
    int16_t camera_y;
} game_state_t;

extern edit_state_t edit_state;
extern data_state_t data_state;
extern game_state_t game_state;

extern volatile bool frame60;
extern volatile bool frame30;
extern unsigned      frame_cnt;

int state_load_cart(const char *path);
int state_save_cart(const char *path);

static inline bool sfx_is_empty(const sfx_t *sfx) {
    for (int i = 0; i < 32; i++) {
        if (sfx->notes[i] & (7 << 9))
            return false;
    }
    return true;
}
