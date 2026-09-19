#include "editor.h"
#include "common.h"
#include "regs.h"
#include "menu.h"
#include "editbuf.h"
#include "editor_internal.h"
#include "ctype2.h"

struct editor_state state;

void reset_state(void) {
    editbuf_reset(state.editbuf);
    state.filename[0]    = 0;
    state.loc_cursor     = (location_t){0, 0};
    state.loc_selection  = (location_t){-1, -1};
    state.scr_first_line = 0;
    state.scr_first_pos  = 0;
}

void update_selection_range(void) {
    if (loc_lt(state.loc_cursor, state.loc_selection)) {
        state.loc_selection_to   = state.loc_selection;
        state.loc_selection_from = state.loc_cursor;
    } else {
        state.loc_selection_to   = state.loc_cursor;
        state.loc_selection_from = state.loc_selection;
    }
}

static bool in_selection(location_t loc) {
    if (!has_selection())
        return false;

    update_selection_range();
    return (loc_lt(loc, state.loc_selection_to) && !loc_lt(loc, state.loc_selection_from));
}

static void render_editor(void) {
    for (int row = 0; row < EDITOR_ROWS; row++) {
        scr_locate(row + 2, 1);
        scr_setcolor(COLOR_EDITOR);

        location_t     loc = (location_t){state.scr_first_line + row, 0};
        const uint8_t *p;
        int            line_len = editbuf_get_line(state.editbuf, loc.line, &p);

        if (line_len > state.scr_first_pos)
            p += state.scr_first_pos;

        line_len = clamp(line_len - state.scr_first_pos, 0, EDITOR_COLUMNS);

        if (loc.line == state.loc_cursor.line) {
            int cpos = get_cursor_pos();

            for (int i = 0; i < EDITOR_COLUMNS; i++) {
                loc.pos = state.scr_first_pos + i;
                scr_setcolor(
                    (i == cpos - state.scr_first_pos)
                        ? COLOR_CURSOR
                        : (in_selection(loc) ? COLOR_SELECTED : COLOR_EDITOR));

                if (i < line_len) {
                    scr_putchar(*(p++));
                } else {
                    if (i > line_len)
                        scr_setcolor(COLOR_EDITOR);
                    scr_putchar(' ');
                }
            }

        } else if (loc.line == state.loc_selection.line) {
            for (int i = 0; i < EDITOR_COLUMNS; i++) {
                loc.pos = state.scr_first_pos + i;
                scr_setcolor(
                    (in_selection(loc) ? COLOR_SELECTED : COLOR_EDITOR));

                if (i < line_len) {
                    scr_putchar(*(p++));
                } else {
                    if (i > line_len)
                        scr_setcolor(COLOR_EDITOR);
                    scr_putchar(' ');
                }
            }

        } else {
            if (in_selection(loc))
                scr_setcolor(COLOR_SELECTED);

            scr_putbuf(p, line_len);
            int fill_sz = EDITOR_COLUMNS - line_len;
            if (fill_sz > 0) {
                scr_putchar(' ');
                fill_sz--;
            }

            scr_setcolor(COLOR_EDITOR);
            scr_fillchar(' ', fill_sz);
        }
    }
}

static void render_editor_border(void) {
    char title[65];
    snprintf(title, sizeof(title), "%s%s", *state.filename ? state.filename : "Untitled", editbuf_get_modified(state.editbuf) ? "\x88" : "");
    scr_draw_border(1, 0, TEXT_COLUMNS, TEXT_ROWS - 1, COLOR_EDITOR, BORDER_FLAG_NO_BOTTOM | BORDER_FLAG_NO_SHADOW | BORDER_FLAG_TITLE_INVERSE, title);
}

static void render_statusbar(void) {
    char tmp[64];

    tmp[0] = 0;

    const uint8_t *p;
    int            line_len = editbuf_get_line(state.editbuf, state.loc_cursor.line, &p);

    // snprintf(
    //     tmp, sizeof(tmp), "ln:%p(%d) bf %p:%p spl %p:%p",
    //     p, line_len,
    //     state.editbuf->p_buf, state.editbuf->p_buf_end,
    //     state.editbuf->p_split_start, state.editbuf->p_split_end);

    // snprintf(tmp, sizeof(tmp), "%p", state.editbuf.p_buf);

    // uint8_t *p = getline_addr(state.loc_cursor.line);
    // snprintf(tmp, sizeof(tmp), "p[0]=%u p[1]=%u lines=%u cursor_line=%d scr_first_line=%d", p[0], p[1], state.num_lines, state.loc_cursor.line, state.scr_first_line);

    scr_status_msg(tmp);
    scr_setcolor(COLOR_STATUS2);
    scr_putchar(26);

    int cpos = min(line_len, state.loc_cursor.pos);
    snprintf(tmp, sizeof(tmp), " %05d:%03d ", state.loc_cursor.line + 1, cpos + 1);
    scr_puttext(tmp);
}

static void menu_redraw_screen(void) {
    render_editor_border();
    render_editor();
}

static int get_leading_spaces(int line) {
    const uint8_t *p;
    int            line_len = editbuf_get_line(state.editbuf, line, &p);
    if (line_len < 0)
        return 0;

    int leading_spaces = 0;
    for (int i = 0; i < line_len; i++) {
        if (p[i] != ' ')
            break;
        leading_spaces++;
    }
    return leading_spaces;
}

void editor_redraw_screen(void) {
    state.loc_cursor.line = clamp(state.loc_cursor.line, 0, editbuf_get_line_count(state.editbuf) - 1);
    state.loc_cursor.pos  = max(0, state.loc_cursor.pos);
    state.scr_first_line  = clamp(state.scr_first_line, max(0, state.loc_cursor.line - (EDITOR_ROWS - 1)), state.loc_cursor.line);
    state.scr_first_pos   = clamp(state.scr_first_pos, max(0, get_cursor_pos() - (EDITOR_COLUMNS - 1)), get_cursor_pos());

    menubar_render(menubar_menus, false, NULL);
    render_editor_border();
    render_editor();
    render_statusbar();
}

static void delete_selection(void) {
    if (has_selection()) {
        update_selection_range();
        editbuf_delete_range(state.editbuf, state.loc_selection_from, state.loc_selection_to);
        state.loc_cursor = state.loc_selection_from;
    }
}

bool loc_dec(location_t *loc) {
    if (loc->pos > 0) {
        loc->pos--;
    } else if (loc->line > 0) {
        loc->line--;
        loc->pos = editbuf_get_line(state.editbuf, loc->line, NULL);
    } else {
        return false;
    }
    return true;
}

void loc_inc(location_t *loc) {
    int line_len = editbuf_get_line(state.editbuf, loc->line, NULL);
    if (loc->pos < line_len) {
        loc->pos++;
    } else if (loc->line + 1 < editbuf_get_line_count(state.editbuf)) {
        loc->line++;
        loc->pos = 0;
    }
}

static void forward_delete(void) {
    location_t loc_to = state.loc_cursor;
    loc_inc(&loc_to);
    editbuf_delete_range(state.editbuf, state.loc_cursor, loc_to);
}

static void backward_delete(void) {
    if (loc_dec(&state.loc_cursor))
        forward_delete();
}

static void insert_ch(uint8_t ch) {
    update_cursor_pos();
    if (editbuf_insert_ch(state.editbuf, state.loc_cursor, ch)) {
        state.loc_cursor.pos++;
    }
}

static int indent_line(int line) {
    int spaces = get_leading_spaces(line);

    spaces %= TAB_SIZE;
    int count = TAB_SIZE - spaces;

    for (int i = 0; i < count; i++)
        editbuf_insert_ch(state.editbuf, (location_t){line, 0}, ' ');

    return count;
}

static int unindent_line(int line) {
    int spaces = get_leading_spaces(line);
    int count  = (spaces % TAB_SIZE != 0) ? (spaces % TAB_SIZE) : TAB_SIZE;
    if (count > spaces)
        count = spaces;

    for (int i = 0; i < count; i++)
        editbuf_delete_ch(state.editbuf, (location_t){line, 0});

    return count;
}

void editor(struct editbuf *eb, const char *path) {
    state.editbuf = eb;

    reinit_video();
    reset_state();

    if (path != NULL) {
        snprintf(state.filename, sizeof(state.filename), "%s", path);
        if (!editbuf_load(state.editbuf, path)) {
            state.editbuf->modified = true;
        }
    }

    while (KEYBUF >= 0) {
    }

    while (1) {
        editor_redraw_screen();

        int key;
        while ((key = KEYBUF) < 0);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            if ((key & KEY_KEYDOWN) && (scancode == SCANCODE_LALT || scancode == SCANCODE_RALT)) {
                menubar_handle(menubar_menus, menu_redraw_screen);
            }

        } else {
            uint8_t ch = key & 0xFF;

            menu_handler_t handler = NULL;
            if ((key & (KEY_MOD_CTRL | KEY_MOD_ALT)) || is_cntrl(key)) {
                uint16_t shortcut = (key & (KEY_MOD_CTRL | KEY_MOD_SHIFT | KEY_MOD_ALT)) | toupper(ch);
                handler           = menubar_find_shortcut(menubar_menus, shortcut);
            }

            if (handler) {
                handler();
            } else {
                bool       check_other     = false;
                location_t prev_loc_cursor = state.loc_cursor;
                switch (ch) {
                    case CH_UP: state.loc_cursor.line--; break;
                    case CH_DOWN: state.loc_cursor.line++; break;
                    case CH_LEFT: {
                        update_cursor_pos();
                        loc_dec(&state.loc_cursor);
                        break;
                    }
                    case CH_RIGHT: {
                        update_cursor_pos();
                        loc_inc(&state.loc_cursor);
                        break;
                    }
                    case CH_HOME: {
                        if (key & KEY_MOD_CTRL) {
                            state.loc_cursor.line = 0;
                            state.loc_cursor.pos  = 0;
                        } else {
                            int leading_spaces   = get_leading_spaces(state.loc_cursor.line);
                            state.loc_cursor.pos = (state.loc_cursor.pos == leading_spaces) ? 0 : leading_spaces;
                        }
                        break;
                    }
                    case CH_END: {
                        if (key & KEY_MOD_CTRL)
                            state.loc_cursor.line = editbuf_get_line_count(state.editbuf) - 1;
                        state.loc_cursor.pos = editbuf_get_line(state.editbuf, state.loc_cursor.line, NULL);
                        break;
                    }
                    case CH_PAGEUP: state.loc_cursor.line -= (EDITOR_ROWS - 1); break;
                    case CH_PAGEDOWN: state.loc_cursor.line += (EDITOR_ROWS - 1); break;
                    default: check_other = true;
                }

                // Start a new selection?
                if (!check_other && !has_selection() && (key & KEY_MOD_SHIFT) != 0) {
                    state.loc_selection = prev_loc_cursor;
                }

                bool keep_selection = false;

                if (check_other) {
                    switch (ch) {
                        case CH_DELETE: {
                            update_cursor_pos();
                            if (has_selection()) {
                                delete_selection();
                            } else {
                                forward_delete();
                            }
                            break;
                        }
                        case CH_BACKSPACE: {
                            update_cursor_pos();
                            if (has_selection()) {
                                delete_selection();
                                break;
                            }
                            if (state.loc_cursor.line <= 0 && state.loc_cursor.pos <= 0)
                                break;

                            // Unindent?
                            if (state.loc_cursor.pos > 0 && state.loc_cursor.pos == get_leading_spaces(state.loc_cursor.line)) {
                                location_t loc_old = state.loc_cursor;
                                do {
                                    loc_dec(&state.loc_cursor);
                                } while (state.loc_cursor.pos % TAB_SIZE != 0);
                                editbuf_delete_range(state.editbuf, state.loc_cursor, loc_old);
                                break;
                            }

                            backward_delete();
                            break;
                        }
                        case CH_ENTER: {
                            update_cursor_pos();
                            int leading_spaces = min(state.loc_cursor.pos, get_leading_spaces(state.loc_cursor.line));
                            if (editbuf_insert_ch(state.editbuf, state.loc_cursor, '\n')) {
                                state.loc_cursor.line++;
                                state.loc_cursor.pos = 0;

                                // Auto indent
                                while (leading_spaces > 0) {
                                    leading_spaces--;
                                    insert_ch(' ');
                                }
                            }
                            break;
                        }
                        case CH_TAB: {
                            update_cursor_pos();
                            bool shift_pressed = (key & KEY_MOD_SHIFT) == 0;
                            if (has_selection()) {
                                int from_line = state.loc_selection_from.line;
                                int to_line   = state.loc_selection_to.line;
                                if (state.loc_selection_to.pos == 0)
                                    to_line--;

                                for (int line = from_line; line <= to_line; line++) {
                                    if (shift_pressed)
                                        indent_line(line);
                                    else
                                        unindent_line(line);
                                }
                                keep_selection = true;

                            } else {
                                if (shift_pressed) {
                                    int pos = state.loc_cursor.pos;
                                    do {
                                        pos++;
                                    } while (pos % TAB_SIZE != 0);

                                    int count = pos - state.loc_cursor.pos;
                                    for (int i = 0; i < count; i++)
                                        insert_ch(' ');

                                } else {
                                    state.loc_cursor.pos -= unindent_line(state.loc_cursor.line);
                                }
                            }
                            break;
                        }
                        default: {
                            if ((key & (KEY_MOD_GUI | KEY_MOD_ALT | KEY_MOD_CTRL)) == 0 && !is_cntrl(ch))
                                insert_ch(ch);
                            break;
                        }
                    }
                }

                // Clear existing selection?
                if ((key & KEY_MOD_SHIFT) == 0 && !keep_selection) {
                    clear_selection();
                }
            }
        }
    }
}

void       editor_set_cursor(location_t loc) { state.loc_cursor = loc; }
location_t editor_get_cursor(void) { return state.loc_cursor; }
