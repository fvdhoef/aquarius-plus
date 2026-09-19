#pragma once

#include "common.h"

typedef void (*menu_handler_t)(void);
typedef bool (*menu_is_active_handler_t)(void);

struct menu_item {
    const char              *title;
    const char              *status;
    uint16_t                 shortcut;
    menu_handler_t           handler;
    menu_is_active_handler_t is_active_handler;
};

struct menu {
    const char             *title;
    const struct menu_item *items;
};

void           menubar_render(const struct menu *menus, bool show_accel, const struct menu *active_menu);
menu_handler_t menubar_find_shortcut(const struct menu *menus, uint16_t shortcut);
void           menubar_handle(const struct menu *menus, void (*redraw_screen)(void));
