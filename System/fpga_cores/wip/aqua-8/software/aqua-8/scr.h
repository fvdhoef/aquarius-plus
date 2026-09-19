#pragma once

#include "common.h"
#include "draw/draw.h"
#include "state.h"

typedef struct {
    void (*init)(void);
    void (*draw)(void);
    void (*on_key)(uint16_t code);
} screen_t;

screen_t               *scr_get(unsigned mode);
static inline screen_t *scr_get_current(void) { return scr_get(edit_state.mode); }

void scr_init(void);
void scr_draw_status(void);
void scr_common(unsigned bg_col);
void scr_key(uint16_t code);
void scr_show_message(void);

static inline bool mouse_clicked(const rect_t *r, uint8_t button, uint8_t spr) {
    if (rect_contains(r, edit_state.mouse_ev.x, edit_state.mouse_ev.y)) {
        edit_state.mouse_spr = spr;

        if (edit_state.mouse_ev.clicked_buttons == button && edit_state.mouse_ev.buttons == edit_state.mouse_ev.clicked_buttons)
            return true;
    }
    return false;
}

static inline bool mouse_hover(const rect_t *r, uint8_t spr) {
    if (rect_contains(r, edit_state.mouse_ev.x, edit_state.mouse_ev.y)) {
        edit_state.mouse_spr = spr;
        return true;
    }
    return false;
}

static inline bool mouse_lclick(const rect_t *r) { return mouse_clicked(r, 1, MOUSE_SPR_HAND); }
static inline bool mouse_rclick(const rect_t *r) { return mouse_clicked(r, 2, MOUSE_SPR_HAND); }

extern screen_t scr_code;
extern screen_t scr_sprite;
extern screen_t scr_map;
extern screen_t scr_sfx;
extern screen_t scr_music;
