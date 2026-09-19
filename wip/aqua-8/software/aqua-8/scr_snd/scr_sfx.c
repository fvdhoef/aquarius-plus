#include "scr.h"

static void draw_note_row(int x, int y, int row) {
    uint16_t note_code = data_state.sfx[edit_state.sfx_edit.sfx_idx].notes[row];
    unsigned pitch     = note_code & 63;
    unsigned wf        = (note_code >> 6) & 7;
    unsigned vol       = (note_code >> 9) & 7;
    unsigned fx        = (note_code >> 12) & 7;

    static const char *note0 = "CCDDEFFGGAAB";
    static const char *note1 = " # #  # # # ";

    unsigned note   = pitch % 12;
    unsigned octave = pitch / 12;

    bool is_current_row = row == edit_state.sfx_edit.cursor_row;
    int  cursor_col     = is_current_row ? edit_state.sfx_edit.cursor_col : -1;

    // fill_rect(x, y, x + 29, y + 6, 0);

    {
        // fill_rect(x + 1, y + 1, x + 28, y + 7, 1);

        unsigned bg_col = is_current_row ? 1 : 0;

        {
            rect_t r = {x + 1, y + 1, x + 8, y + 7};
            fill_rect(&r, cursor_col == 0 ? 3 : bg_col);
            if (mouse_clicked(&r, 1, MOUSE_SPR_IBEAM)) {
                edit_state.sfx_edit.cursor_row = row;
                edit_state.sfx_edit.cursor_col = 0;
            }
        }
        {
            rect_t r = {x + 9, y + 1, x + 13, y + 7};
            fill_rect(&r, cursor_col == 1 ? 3 : bg_col);
            if (mouse_clicked(&r, 1, MOUSE_SPR_IBEAM)) {
                edit_state.sfx_edit.cursor_row = row;
                edit_state.sfx_edit.cursor_col = 1;
            }
        }
        {
            rect_t r = {x + 14, y + 1, x + 18, y + 7};
            fill_rect(&r, cursor_col == 2 ? 3 : bg_col);
            if (mouse_clicked(&r, 1, MOUSE_SPR_IBEAM)) {
                edit_state.sfx_edit.cursor_row = row;
                edit_state.sfx_edit.cursor_col = 2;
            }
        }
        {
            rect_t r = {x + 19, y + 1, x + 23, y + 7};
            fill_rect(&r, cursor_col == 3 ? 3 : bg_col);
            if (mouse_clicked(&r, 1, MOUSE_SPR_IBEAM)) {
                edit_state.sfx_edit.cursor_row = row;
                edit_state.sfx_edit.cursor_col = 3;
            }
        }
        {
            rect_t r = {x + 24, y + 1, x + 28, y + 7};
            fill_rect(&r, cursor_col == 4 ? 3 : bg_col);
            if (mouse_clicked(&r, 1, MOUSE_SPR_IBEAM)) {
                edit_state.sfx_edit.cursor_row = row;
                edit_state.sfx_edit.cursor_col = 4;
            }
        }
    }

    unsigned col_vol0 = is_current_row ? 2 : 1;

    // fill_rect(x + 1, y + 1, x + 7, y + 7, 10);

    x += 2;
    draw_char_altfont(x, y + 2, vol == 0 ? '.' : note0[note], vol == 0 ? col_vol0 : 7); // .CDEFGAB
    x += 4;
    draw_char_altfont(x, y + 2, vol == 0 ? '.' : note1[note], vol == 0 ? col_vol0 : 7); // #
    x += 4;
    draw_char_altfont(x, y + 2, vol == 0 ? '.' : '0' + octave, vol == 0 ? col_vol0 : 6); // 01234
    x += 5;
    draw_char_altfont(x, y + 2, vol == 0 ? '.' : '0' + wf, vol == 0 ? col_vol0 : 14); // Waveform: 01234567
    x += 5;
    draw_char_altfont(x, y + 2, vol == 0 ? '.' : '0' + vol, vol == 0 ? col_vol0 : 12); // Volume: 01234567
    x += 5;
    draw_char_altfont(x, y + 2, (vol == 0 || fx == 0) ? '.' : '0' + fx, vol == 0 ? col_vol0 : 13); // Effect: 01234567
}

static void draw(void) {
    int x, y;
    scr_common(5);

    sfx_t *sfx = &data_state.sfx[edit_state.sfx_edit.sfx_idx];

    // SFX
    int sfx_x = 2;
    int sfx_y = 9;
    {
        fill_rect(&(rect_t){sfx_x, sfx_y, sfx_x + 42, sfx_y + 140}, 1);
        draw_text("SFX", sfx_x + 16, sfx_y + 3, 7, true);

        y = sfx_y + 11;

        int nr = 0;
        for (int j = 0; j < 16; j++) {
            x = sfx_x + 2;
            for (int i = 0; i < 4; i++) {
                rect_t r = {x, y, x + 8, y + 6};
                if (mouse_lclick(&r))
                    edit_state.sfx_edit.sfx_idx = nr;

                fill_rect(&r, nr == edit_state.sfx_edit.sfx_idx ? 7 : (sfx_is_empty(&data_state.sfx[nr]) ? 13 : 12));
                draw_char_altfont(x + 1, y + 1, '0' + (nr / 10), 5);
                draw_char_altfont(x + 5, y + 1, '0' + (nr % 10), 5);

                x += 10;
                nr++;
            }
            y += 8;
        }
    }

    // Parameters
    char tmp[16];
    int  param_x = 59;
    int  param_y = 9;
    {
        // fill_rect(param_x, param_y, param_x + 121, param_y + 26, 1);

        // Speed
        {
            x = param_x + 2;
            y = param_y + 2;
            draw_text("SPD", x, y, 6, true);
            x += 14;

            rect_t r = {x - 1, y - 1, x + 11, y + 5};
            fill_rect(&r, 0);
            snprintf(tmp, sizeof(tmp), "%03u", sfx->speed);
            draw_text(tmp, x, y, 6, true);

            // Handle value changes
            if (mouse_hover(&r, MOUSE_SPR_HAND)) {
                int new_val = sfx->speed;
                if (edit_state.mouse_ev.clicked_buttons == 1)
                    new_val++;
                else if (edit_state.mouse_ev.clicked_buttons == 2)
                    new_val--;
                new_val += edit_state.mouse_ev.wheel;
                sfx->speed = clamp(new_val, 1, 255);
            }
        }

        // Loop
        {
            x = param_x + 49;
            y = param_y + 2;
            draw_text(sfx->loop_start > 0 && sfx->loop_end == 0 ? "LEN" : "LOOP", x, y, 6, true);
            x += 18;

            // Loop start
            {
                rect_t r = {x - 1, y - 1, x + 7, y + 5};
                fill_rect(&r, 0);
                snprintf(tmp, sizeof(tmp), "%02u", sfx->loop_start);
                draw_text(tmp, x, y, 6, true);

                // Handle value changes
                if (mouse_hover(&r, MOUSE_SPR_HAND)) {
                    int new_val = sfx->loop_start;
                    if (edit_state.mouse_ev.clicked_buttons == 1)
                        new_val++;
                    else if (edit_state.mouse_ev.clicked_buttons == 2)
                        new_val--;
                    new_val += edit_state.mouse_ev.wheel;
                    sfx->loop_start = clamp(new_val, 0, 63);
                }
            }

            x += 11;

            // Loop end
            {
                rect_t r = {x - 1, y - 1, x + 7, y + 5};
                fill_rect(&r, 0);
                snprintf(tmp, sizeof(tmp), "%02u", sfx->loop_end);
                draw_text(tmp, x, y, 6, true);

                // Handle value changes
                if (mouse_hover(&r, MOUSE_SPR_HAND)) {
                    int new_val = sfx->loop_end;
                    if (edit_state.mouse_ev.clicked_buttons == 1)
                        new_val++;
                    else if (edit_state.mouse_ev.clicked_buttons == 2)
                        new_val--;
                    new_val += edit_state.mouse_ev.wheel;
                    sfx->loop_end = clamp(new_val, 0, 63);
                }
            }
        }

        // Octave
        {
            x = param_x + 2;
            y = param_y + 11;
            draw_text("OCT", x, y, 6, true);
            x += 13;
            for (int i = 0; i <= 4; i++) {
                rect_t r = {x, y - 1, x + 4, y + 5};
                fill_rect(&r, (edit_state.sfx_edit.octave == i) ? 7 : 6);
                draw_char_altfont(x + 1, y, '0' + i, 5);

                // Handle value changes
                if (mouse_lclick(&r))
                    edit_state.sfx_edit.octave = i;

                x += 6;
            }
        }

        // Volume
        {
            x = param_x + 2;
            y = param_y + 20;
            draw_text("VOL", x, y, 6, true);
            x += 13;
            y -= 1;
            for (int i = 0; i <= 7; i++) {
                rect_t r = {x, y, x + 2, y + 6};

                // Black
                if (i < 7)
                    fill_rect(&(rect_t){x, y, x + 1, y + (6 - i)}, edit_state.sfx_edit.volume == i ? 13 : 0);

                // White
                if (i > 0)
                    fill_rect(&(rect_t){x, y + (7 - i), x + 1, y + 6}, edit_state.sfx_edit.volume == i ? 7 : 6);

                // Handle value changes
                if (mouse_lclick(&r))
                    edit_state.sfx_edit.volume = i;

                x += 3;
            }
        }

        // Waveforms
        {
            x = param_x + 49;
            y = param_y + 10;
            for (int i = 0; i < 8; i++) {
                rect_t r = {x, y, x + 7, y + 5};
                fill_rect(&r, i == edit_state.sfx_edit.waveform ? i + 8 : 6);
                draw_icon(x, y, 48 + i, 7);

                // Handle value changes
                if (mouse_lclick(&r))
                    edit_state.sfx_edit.waveform = i;

                x += 9;
            }
        }

        // Effects
        {
            x = param_x + 49;
            y = param_y + 20;
            for (int i = 0; i < 8; i++) {
                rect_t r = {x, y, x + 7, y + 5};
                fill_rect(&r, i == edit_state.sfx_edit.effect ? 7 : 13);
                draw_icon(x, y, 64 + i, 5);

                // Handle value changes
                if (mouse_lclick(&r))
                    edit_state.sfx_edit.effect = i;

                x += 9;
            }
        }
    }

    int notes_x = 59;
    int notes_y = 39;

    // Notes
    unsigned idx = 0;
    for (int row = 0; row < 4; row++) {
        x = notes_x + row * 32;
        y = notes_y;
        draw_rect(&(rect_t){x, y, x + 29, y + 57}, 0);
        for (int i = 0; i < 8; i++) {
            draw_note_row(x, y, idx++);
            y += 7;
        }
    }

    // x = 56;
    // y = 90;
    // fill_rect(x, y, x + 128, y + 64, 0);
}

static void on_key(uint16_t code) {
    sfx_t *sfx = &data_state.sfx[edit_state.sfx_edit.sfx_idx];

    if ((code & KEY_IS_SCANCODE) == 0) {
        unsigned key = (code & (KEY_MODIFIERS | KEY_CODE_MASK));

        switch (key) {
            case CH_UP: edit_state.sfx_edit.cursor_row--; break;
            case CH_UP | KEY_MOD_CTRL: edit_state.sfx_edit.cursor_row -= 4; break;
            case CH_DOWN: edit_state.sfx_edit.cursor_row++; break;
            case CH_DOWN | KEY_MOD_CTRL: edit_state.sfx_edit.cursor_row += 4; break;
            case CH_LEFT: {
                if (edit_state.sfx_edit.cursor_col == 0) {
                    edit_state.sfx_edit.cursor_col = 4;
                    edit_state.sfx_edit.cursor_row -= 8;
                } else {
                    edit_state.sfx_edit.cursor_col--;
                }
                break;
            }
            case CH_LEFT | KEY_MOD_CTRL: edit_state.sfx_edit.cursor_row -= 8; break;

            case CH_RIGHT: {
                if (edit_state.sfx_edit.cursor_col == 4) {
                    edit_state.sfx_edit.cursor_col = 0;
                    edit_state.sfx_edit.cursor_row += 8;
                } else {
                    edit_state.sfx_edit.cursor_col++;
                }
                break;
            }
            case CH_RIGHT | KEY_MOD_CTRL: edit_state.sfx_edit.cursor_row += 8; break;

            case CH_HOME: edit_state.sfx_edit.cursor_row = 0; break;
            case CH_END: edit_state.sfx_edit.cursor_row = 31; break;
            case CH_PAGEUP: edit_state.sfx_edit.cursor_row -= 4; break;
            case CH_PAGEDOWN: edit_state.sfx_edit.cursor_row += 4; break;
            case CH_DELETE: {

                break;
            }
            default: break;
        }

        switch (code & KEY_CODE_MASK) {
            case ',': sfx->speed--; break;
            case '.': sfx->speed++; break;
            case '<': sfx->speed -= 4; break;
            case '>': sfx->speed += 4; break;
            default: break;
        }

        edit_state.sfx_edit.cursor_row &= 31;
    }
}

screen_t scr_sfx = {
    .draw   = draw,
    .on_key = on_key,
};
