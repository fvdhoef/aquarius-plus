#include "scr.h"

screen_t *scr_get(unsigned mode) {
    switch (mode) {
        default:
        case MODE_CODE: return &scr_code;
        case MODE_SPRITE: return &scr_sprite;
        case MODE_MAP: return &scr_map;
        case MODE_SFX: return &scr_sfx;
        case MODE_MUSIC: return &scr_music;
    }
}

#define MESSAGE_DURATION 150

void scr_show_message(void) {
    edit_state.message_tick = MESSAGE_DURATION;
}

void scr_draw_status(void) {
    fill_rect(&(rect_t){0, 159 - 6, 199, 159}, 2);
    draw_text(edit_state.status_text, 1, 154, 14, true);

    if (edit_state.message_tick > 0) {
        int y = 159 - 6;

        if (edit_state.message_tick > MESSAGE_DURATION - 6) {
            y += 6 - (MESSAGE_DURATION - edit_state.message_tick);
        }

        if (edit_state.message_tick < 6) {
            y += 6 - edit_state.message_tick;
        }

        fill_rect(&(rect_t){0, y, 199, y + 6}, 8);
        draw_text(edit_state.message, 1, y + 1, 15, true);
        edit_state.message_tick--;
    }
}

void scr_common(unsigned bg_col) {
    fill_rect(&(rect_t){0, 0, 199, 6}, 8);
    fill_rect(&(rect_t){0, 7, 199, 152}, bg_col);

    // Mode icons
    {
        int x = 200 - 5 * 8 - 1;

        rect_t r = {x, 0, x + 6, 6};

        for (int i = 0; i <= MODE_MUSIC; i++) {
            // fill_rect(&r, 0);
            if (mouse_lclick(&r))
                edit_state.mode = i;

            draw_icon(r.x0 + 1, 1, i, edit_state.mode == i ? 7 : 2);

            r.x0 += 8;
            r.x1 += 8;
        }

        // draw_icon(x, 1, 0, edit_state.mode == MODE_CODE ? 7 : 2);
        // x += 8;
        // draw_icon(x, 1, 1, edit_state.mode == MODE_SPRITE ? 7 : 2);
        // x += 8;
        // draw_icon(x, 1, 2, edit_state.mode == MODE_MAP ? 7 : 2);
        // x += 8;
        // draw_icon(x, 1, 3, edit_state.mode == MODE_SFX ? 7 : 2);
        // x += 8;
        // draw_icon(x, 1, 4, edit_state.mode == MODE_MUSIC ? 7 : 2);
    }

    const char *title = "";
    switch (edit_state.mode) {
        case MODE_CODE: title = "Code editor"; break;
        case MODE_SPRITE: title = "Sprite editor"; break;
        case MODE_MAP: title = "Map editor"; break;
        case MODE_SFX: title = "Sound effects editor"; break;
        case MODE_MUSIC: title = "Music editor"; break;
    }
    draw_text(title, 1, 1, 15, true);
}

void scr_key(uint16_t key) {
    if (key == 3) {
        edit_state.editing = !edit_state.editing;
        return;
    }

    if (edit_state.editing) {
        if (key == (KEY_MOD_ALT | CH_LEFT)) {
            if (edit_state.mode == MODE_CODE) {
                edit_state.mode = MODE_MUSIC;
            } else {
                edit_state.mode--;
            }
            return;
        }
        if (key == (KEY_MOD_ALT | CH_RIGHT)) {
            if (edit_state.mode == MODE_MUSIC) {
                edit_state.mode = MODE_CODE;
            } else {
                edit_state.mode++;
            }
            return;
        }

        if (key == CH_F1) {
            edit_state.mode = MODE_CODE;
        } else if (key == CH_F2) {
            edit_state.mode = MODE_SPRITE;
        } else if (key == CH_F3) {
            edit_state.mode = MODE_MAP;
        } else if (key == CH_F4) {
            edit_state.mode = MODE_SFX;
        } else if (key == CH_F5) {
            edit_state.mode = MODE_MUSIC;
        } else {
            screen_t *scr = scr_get_current();
            if (scr->on_key)
                scr->on_key(key);
        }
    }
}
