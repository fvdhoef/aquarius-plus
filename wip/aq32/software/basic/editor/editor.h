#pragma once

#include "menu.h"
#include "dialog.h"
#include "screen.h"
#include "editbuf.h"

extern const struct menu menubar_menus[];

void       editor(struct editbuf *eb, const char *path);
void       editor_redraw_screen(void);
void       editor_set_cursor(location_t loc);
location_t editor_get_cursor(void);

void cmd_file_new(void);
void cmd_file_open(void);
void cmd_file_save(void);
void cmd_file_save_as(void);
void cmd_file_exit(void);

bool cmd_edit_cut_copy_active(void);

void cmd_edit_cut(void);
void cmd_edit_copy(void);
void cmd_edit_paste(void);
void cmd_edit_select_all(void);
void cmd_edit_find(void);
void cmd_edit_find_next(void);
