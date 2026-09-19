#pragma once

#include "common.h"
#include "state.h"

#define CLIPBOARD_PATH "/.editor-clipboard"

#define EDITOR_ROWS    21
#define EDITOR_COLUMNS 50
#define TAB_SIZE       2

static inline bool has_selection(void) { return edit_state.code_edit.loc_selection.line >= 0; }
static inline void clear_selection(void) { edit_state.code_edit.loc_selection = (location_t){-1, -1}; }
static inline int  get_cursor_pos(void) { return min(edit_state.code_edit.loc_cursor.pos, max(0, editbuf_get_line(edit_state.code_edit.editbuf, edit_state.code_edit.loc_cursor.line, NULL))); }
static inline void update_cursor_pos(void) { edit_state.code_edit.loc_cursor.pos = get_cursor_pos(); }

static inline void code_edit_reset_state(void) {
    code_edit_t *state = &edit_state.code_edit;

    editbuf_reset(state->editbuf);
    state->loc_cursor     = (location_t){0, 0};
    state->loc_selection  = (location_t){-1, -1};
    state->scr_first_line = 0;
    state->scr_first_pos  = 0;
}

void update_selection_range(void);

bool in_selection(location_t loc);

void op_cursor_up(void);
void op_cursor_down(void);
void op_cursor_left(void);
void op_cursor_right(void);
void op_cursor_home(bool ctrl_pressed);
void op_cursor_end(bool ctrl_pressed);
void op_cursor_page_up(void);
void op_cursor_page_down(void);
void op_delete(void);
void op_backspace(void);
void op_enter(void);
bool op_tab(bool shift_pressed);
void op_insert_ch(uint8_t ch);
