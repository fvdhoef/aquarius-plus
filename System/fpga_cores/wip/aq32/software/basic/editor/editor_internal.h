#pragma once

#include "editor.h"

#define CLIPBOARD_PATH "/.editor-clipboard"

#define EDITOR_ROWS    (TEXT_ROWS - 3)
#define EDITOR_COLUMNS (TEXT_COLUMNS - 2)
#define TAB_SIZE       2

struct editor_state {
    struct editbuf *editbuf;
    char            filename[64];
    location_t      loc_cursor;
    location_t      loc_selection;
    int             scr_first_line;
    int             scr_first_pos;

    location_t loc_selection_from;
    location_t loc_selection_to;

    char search_value[128];
};

extern struct editor_state state;

void reset_state(void);
void update_selection_range(void);
bool loc_dec(location_t *loc);
void loc_inc(location_t *loc);

static inline bool has_selection(void) { return state.loc_selection.line >= 0; }
static inline void clear_selection(void) { state.loc_selection = (location_t){-1, -1}; }
static inline int  get_cursor_pos(void) { return min(state.loc_cursor.pos, max(0, editbuf_get_line(state.editbuf, state.loc_cursor.line, NULL))); }
static inline void update_cursor_pos(void) { state.loc_cursor.pos = get_cursor_pos(); }
