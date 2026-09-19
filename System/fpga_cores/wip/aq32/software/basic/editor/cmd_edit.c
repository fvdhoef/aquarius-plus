#include "editor.h"
#include "editor_internal.h"

void cmd_edit_cut(void) {
    if (!has_selection())
        return;

    scr_status_msg("Writing to clipboard file...");
    update_selection_range();
    if (editbuf_save_range(state.editbuf, state.loc_selection_from, state.loc_selection_to, CLIPBOARD_PATH)) {
        editbuf_delete_range(state.editbuf, state.loc_selection_from, state.loc_selection_to);
        clear_selection();
    }
}

void cmd_edit_copy(void) {
    if (!has_selection())
        return;

    scr_status_msg("Writing to clipboard file...");
    update_selection_range();
    editbuf_save_range(state.editbuf, state.loc_selection_from, state.loc_selection_to, CLIPBOARD_PATH);
}

bool cmd_edit_cut_copy_active(void) { return has_selection(); }

void cmd_edit_paste(void) {
    clear_selection();
    update_cursor_pos();
    editbuf_insert_from_file(state.editbuf, &state.loc_cursor, CLIPBOARD_PATH);
}

void cmd_edit_select_all(void) {
    state.loc_selection.line = 0;
    state.loc_selection.pos  = 0;
    state.loc_cursor.line    = editbuf_get_line_count(state.editbuf) - 1;
    state.loc_cursor.pos     = editbuf_get_line(state.editbuf, state.loc_cursor.line, NULL);
}

static int search_in_line(location_t loc, const char *val, size_t val_len) {
    const uint8_t *p_line;
    int            len = editbuf_get_line(state.editbuf, loc.line, &p_line);

    while (loc.pos + (int)val_len <= len) {
        if (strncasecmp((const char *)p_line + loc.pos, val, val_len) == 0)
            return loc.pos;
        loc.pos++;
    }
    return -1;
}

static void search_next(void) {
    size_t search_val_len = strlen(state.search_value);
    if (search_val_len == 0)
        return;

    clear_selection();
    update_cursor_pos();

    location_t loc = state.loc_cursor;
    loc.pos++;

    bool wrapped = false;

    while (1) {
        if (wrapped && loc_lt(state.loc_cursor, loc))
            break;

        int result = search_in_line(loc, state.search_value, search_val_len);
        if (result >= 0) {
            state.loc_selection.line = loc.line;
            state.loc_selection.pos  = result;
            state.loc_cursor.line    = loc.line;
            state.loc_cursor.pos     = result + search_val_len;
            return;
        }

        loc.pos = 0;
        loc.line++;
        if (loc.line >= editbuf_get_line_count(state.editbuf)) {
            if (wrapped)
                break;

            loc.line = 0;
            wrapped  = true;
        }
    }

    editor_redraw_screen();
    dialog_message(NULL, "Match not found!");
}

void cmd_edit_find(void) {
    state.search_value[0] = 0;
    if (!dialog_edit_field("Find", state.search_value, sizeof(state.search_value))) {
        state.search_value[0] = 0;
        return;
    }
    search_next();
}

void cmd_edit_find_next(void) {
    if (has_selection()) {
        // Use selection as search value
    }
    search_next();
}
