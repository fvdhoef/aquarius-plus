#include "edit_ops.h"

void update_selection_range(void) {
    code_edit_t *state = &edit_state.code_edit;

    if (loc_lt(state->loc_cursor, state->loc_selection)) {
        state->loc_selection_to   = state->loc_selection;
        state->loc_selection_from = state->loc_cursor;
    } else {
        state->loc_selection_to   = state->loc_cursor;
        state->loc_selection_from = state->loc_selection;
    }
}

bool in_selection(location_t loc) {
    code_edit_t *state = &edit_state.code_edit;

    if (!has_selection())
        return false;

    update_selection_range();
    return (loc_lt(loc, state->loc_selection_to) && !loc_lt(loc, state->loc_selection_from));
}

static int get_leading_spaces(int line) {
    code_edit_t *state = &edit_state.code_edit;

    const uint8_t *p;
    int            line_len = editbuf_get_line(state->editbuf, line, &p);
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

static void delete_selection(void) {
    code_edit_t *state = &edit_state.code_edit;

    if (has_selection()) {
        update_selection_range();
        editbuf_delete_range(state->editbuf, state->loc_selection_from, state->loc_selection_to);
        state->loc_cursor = state->loc_selection_from;
    }
}

bool loc_dec(location_t *loc) {
    code_edit_t *state = &edit_state.code_edit;

    if (loc->pos > 0) {
        loc->pos--;
    } else if (loc->line > 0) {
        loc->line--;
        loc->pos = editbuf_get_line(state->editbuf, loc->line, NULL);
    } else {
        return false;
    }
    return true;
}

void loc_inc(location_t *loc) {
    code_edit_t *state = &edit_state.code_edit;

    int line_len = editbuf_get_line(state->editbuf, loc->line, NULL);
    if (loc->pos < line_len) {
        loc->pos++;
    } else if (loc->line + 1 < editbuf_get_line_count(state->editbuf)) {
        loc->line++;
        loc->pos = 0;
    }
}

static void forward_delete(void) {
    code_edit_t *state = &edit_state.code_edit;

    location_t loc_to = state->loc_cursor;
    loc_inc(&loc_to);
    editbuf_delete_range(state->editbuf, state->loc_cursor, loc_to);
}

static void backward_delete(void) {
    code_edit_t *state = &edit_state.code_edit;

    if (loc_dec(&state->loc_cursor))
        forward_delete();
}

void op_insert_ch(uint8_t ch) {
    code_edit_t *state = &edit_state.code_edit;

    update_cursor_pos();
    if (editbuf_insert_ch(state->editbuf, state->loc_cursor, ch)) {
        state->loc_cursor.pos++;
    }
}

static int indent_line(int line) {
    code_edit_t *state = &edit_state.code_edit;

    int spaces = get_leading_spaces(line);

    spaces %= TAB_SIZE;
    int count = TAB_SIZE - spaces;

    for (int i = 0; i < count; i++)
        editbuf_insert_ch(state->editbuf, (location_t){line, 0}, ' ');

    return count;
}

static int unindent_line(int line) {
    code_edit_t *state = &edit_state.code_edit;

    int spaces = get_leading_spaces(line);
    int count  = (spaces % TAB_SIZE != 0) ? (spaces % TAB_SIZE) : TAB_SIZE;
    if (count > spaces)
        count = spaces;

    for (int i = 0; i < count; i++)
        editbuf_delete_ch(state->editbuf, (location_t){line, 0});

    return count;
}

void op_cursor_up(void) {
    code_edit_t *state = &edit_state.code_edit;
    state->loc_cursor.line--;
}

void op_cursor_down(void) {
    code_edit_t *state = &edit_state.code_edit;
    state->loc_cursor.line++;
}

void op_cursor_left(void) {
    code_edit_t *state = &edit_state.code_edit;
    update_cursor_pos();
    loc_dec(&state->loc_cursor);
}

void op_cursor_right(void) {
    code_edit_t *state = &edit_state.code_edit;
    update_cursor_pos();
    loc_inc(&state->loc_cursor);
}

void op_cursor_home(bool ctrl_pressed) {
    code_edit_t *state = &edit_state.code_edit;
    if (ctrl_pressed) {
        state->loc_cursor.line = 0;
        state->loc_cursor.pos  = 0;
    } else {
        int leading_spaces    = get_leading_spaces(state->loc_cursor.line);
        state->loc_cursor.pos = (state->loc_cursor.pos == leading_spaces) ? 0 : leading_spaces;
    }
}

void op_cursor_end(bool ctrl_pressed) {
    code_edit_t *state = &edit_state.code_edit;
    if (ctrl_pressed)
        state->loc_cursor.line = editbuf_get_line_count(state->editbuf) - 1;
    state->loc_cursor.pos = editbuf_get_line(state->editbuf, state->loc_cursor.line, NULL);
}

void op_cursor_page_up(void) {
    code_edit_t *state = &edit_state.code_edit;
    state->loc_cursor.line -= (EDITOR_ROWS - 1);
}

void op_cursor_page_down(void) {
    code_edit_t *state = &edit_state.code_edit;
    state->loc_cursor.line += (EDITOR_ROWS - 1);
}

void op_delete(void) {
    update_cursor_pos();
    if (has_selection()) {
        delete_selection();
    } else {
        forward_delete();
    }
}

void op_backspace(void) {
    code_edit_t *state = &edit_state.code_edit;
    update_cursor_pos();
    if (has_selection()) {
        delete_selection();
        return;
    }
    if (state->loc_cursor.line <= 0 && state->loc_cursor.pos <= 0)
        return;

    // Unindent?
    if (state->loc_cursor.pos > 0 && state->loc_cursor.pos == get_leading_spaces(state->loc_cursor.line)) {
        location_t loc_old = state->loc_cursor;
        do {
            loc_dec(&state->loc_cursor);
        } while (state->loc_cursor.pos % TAB_SIZE != 0);
        editbuf_delete_range(state->editbuf, state->loc_cursor, loc_old);
        return;
    }

    backward_delete();
}

void op_enter(void) {
    code_edit_t *state = &edit_state.code_edit;
    update_cursor_pos();
    int leading_spaces = min(state->loc_cursor.pos, get_leading_spaces(state->loc_cursor.line));
    if (editbuf_insert_ch(state->editbuf, state->loc_cursor, '\n')) {
        state->loc_cursor.line++;
        state->loc_cursor.pos = 0;

        // Auto indent
        while (leading_spaces > 0) {
            leading_spaces--;
            op_insert_ch(' ');
        }
    }
}

bool op_tab(bool shift_pressed) {
    code_edit_t *state = &edit_state.code_edit;

    bool keep_selection = false;
    update_cursor_pos();
    if (has_selection()) {
        int from_line = state->loc_selection_from.line;
        int to_line   = state->loc_selection_to.line;
        if (state->loc_selection_to.pos == 0)
            to_line--;

        for (int line = from_line; line <= to_line; line++) {
            if (!shift_pressed)
                indent_line(line);
            else
                unindent_line(line);
        }
        keep_selection = true;

    } else {
        if (!shift_pressed) {
            int pos = state->loc_cursor.pos;
            do {
                pos++;
            } while (pos % TAB_SIZE != 0);

            int count = pos - state->loc_cursor.pos;
            for (int i = 0; i < count; i++)
                op_insert_ch(' ');

        } else {
            state->loc_cursor.pos -= unindent_line(state->loc_cursor.line);
        }
    }
    return keep_selection;
}
