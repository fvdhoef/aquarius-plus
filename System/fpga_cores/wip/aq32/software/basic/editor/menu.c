#include "menu.h"
#include "screen.h"

static int get_menuitem_count(const struct menu *menu, bool include_separator) {
    int                     count = 0;
    const struct menu_item *mi    = menu->items;
    while (mi && mi->title) {
        if (include_separator || mi->title[0] != '-')
            count++;
        mi++;
    }
    return count;
}

static const char *get_key_str(uint8_t ch) {
    static char tmp[4];
    switch (ch) {
        case CH_F1: return "F1";
        case CH_F2: return "F2";
        case CH_F3: return "F3";
        case CH_F4: return "F4";
        case CH_F5: return "F5";
        case CH_F6: return "F6";
        case CH_F7: return "F7";
        case CH_F8: return "F8";
        case CH_F9: return "F9";
        case CH_F10: return "F10";
        case CH_F11: return "F11";
        case CH_F12: return "F12";
        case CH_PRINTSCREEN: return "PrtScr";
        case CH_PAUSE: return "Pause";
        case CH_INSERT: return "Ins";
        case CH_HOME: return "Home";
        case CH_PAGEUP: return "PgUp";
        case CH_DELETE: return "Del";
        case CH_END: return "End";
        case CH_PAGEDOWN: return "PgDn";
        case CH_RIGHT: return "Right";
        case CH_LEFT: return "Left";
        case CH_DOWN: return "Down";
        case CH_UP: return "Up";
        default:
            snprintf(tmp, sizeof(tmp), "%c", ch);
            return tmp;
    }
}

static const char *get_shortcut_str(const struct menu_item *mi) {
    static char tmp[64];
    if (mi->shortcut == 0) {
        tmp[0] = 0;
    } else {
        snprintf(
            tmp, sizeof(tmp), "  %s%s%s%s",
            (mi->shortcut & KEY_MOD_CTRL) ? "Ctrl+" : "",
            (mi->shortcut & KEY_MOD_SHIFT) ? "Shift+" : "",
            (mi->shortcut & KEY_MOD_ALT) ? "Alt+" : "",
            get_key_str(mi->shortcut & 0xFF));
    }
    return tmp;
}

menu_handler_t menubar_find_shortcut(const struct menu *menus, uint16_t shortcut) {
    const struct menu *m = menus;
    while (m->title) {
        const struct menu_item *mi = m->items;
        while (mi->title) {
            if (mi->shortcut == shortcut) {
                if (mi->is_active_handler && !mi->is_active_handler())
                    return NULL;

                return mi->handler;
            }

            mi++;
        }
        m++;
    }
    return NULL;
}

static int menu_item_get_width(const struct menu_item *mi) {
    if (mi->title[0] == '-')
        return 0;
    return strlen_accel(mi->title) + strlen(get_shortcut_str(mi));
}

static int menu_get_width(const struct menu *menu) {
    const struct menu_item *mi = menu->items;
    if (!mi)
        return 0;

    int width = 0;
    while (mi->title) {
        width = max(width, menu_item_get_width(mi));
        mi++;
    }
    return width;
}

void menubar_render(const struct menu *menus, bool show_accel, const struct menu *active_menu) {
    scr_locate(0, 0);
    scr_setcolor(COLOR_MENU);

    scr_putchar(' ');
    scr_putchar(' ');

    const struct menu *m = menus;
    while (m->title) {
        uint8_t col = (m == active_menu) ? COLOR_MENU_SEL : COLOR_MENU;
        scr_setcolor(col);
        scr_putchar(' ');
        scr_puttext_accel(m->title, show_accel);
        scr_putchar(' ');
        m++;
    }

    scr_setcolor(COLOR_MENU);
    scr_fillchar(' ', TRAM->text + 80 - p_scr);
}

static int get_menu_offset(const struct menu *menus, const struct menu *active_menu) {
    int                x = 1;
    const struct menu *m = menus;
    while (m != active_menu && m->title) {
        x += 2 + strlen_accel(m->title);
        m++;
    }
    return x;
}

static const struct menu_item *get_menu_item_by_idx(const struct menu *menu, int idx) {
    int                     count = 0;
    const struct menu_item *mi    = menu->items;
    while (mi && mi->title) {
        if (mi->title[0] != '-') {
            if (count == idx)
                return mi;

            count++;
        }
        mi++;
    }
    return NULL;
}

static const struct menu_item *get_menu_item_by_accel(const struct menu *menu, char ch) {
    const struct menu_item *mi = menu->items;
    while (mi && mi->title) {
        const char *p = mi->title;
        while (*p) {
            if (p[0] == '&') {
                if (toupper(p[1]) == toupper(ch))
                    return mi;
                break;
            }
            p++;
        }
        mi++;
    }
    return NULL;
}

static const struct menu *get_menu_by_accel(const struct menu *menus, char ch) {
    const struct menu *m = menus;
    while (m && m->title) {
        const char *p = m->title;
        while (*p) {
            if (p[0] == '&') {
                if (toupper(p[1]) == toupper(ch))
                    return m;
                break;
            }
            p++;
        }
        m++;
    }
    return NULL;
}

static void render_menu(const struct menu *menus, const struct menu *active_menu, int active_idx, bool render_border) {
    int x    = get_menu_offset(menus, active_menu);
    int y    = 1;
    int w    = 4 + menu_get_width(active_menu);
    int idx  = 0;
    int rows = get_menuitem_count(active_menu, true);

    if (render_border)
        scr_draw_border(y, x, w, 2 + rows, COLOR_MENU, 0, NULL);
    y++;

    // Draw items
    const struct menu_item *mi = active_menu->items;
    while (mi && mi->title) {
        if (mi->title[0] == '-') {
            if (render_border)
                scr_draw_separator(y, x, w, COLOR_MENU);

        } else {
            bool    selected   = (idx == active_idx);
            uint8_t col        = COLOR_MENU;
            bool    show_accel = true;
            if (mi->is_active_handler && !(mi->is_active_handler())) {
                col        = COLOR_MENU_INACTIVE;
                show_accel = false;
            }

            if (selected)
                col = COLOR_MENU_SEL;

            scr_setcolor(col);
            scr_locate(y, x + 1);

            const char *shortcut_str     = get_shortcut_str(mi);
            int         shortcut_str_len = strlen(shortcut_str);
            int         title_len        = strlen_accel(mi->title);

            scr_putchar(' ');
            scr_puttext_accel(mi->title, show_accel);
            scr_fillchar(' ', w - 4 - title_len - shortcut_str_len);

            scr_setcolor(selected ? COLOR_MENU_SHORTCUT_SEL : COLOR_MENU_SHORTCUT);
            scr_puttext(shortcut_str);
            scr_setcolor(col);
            scr_putchar(' ');

            if (selected)
                scr_status_msg(mi->status);

            idx++;
        }

        mi++;
        y++;
    }
}

void menubar_handle(const struct menu *menus, void (*redraw_screen)(void)) {
    int                key;
    const struct menu *active_menu = menus;
    bool               menu_open   = false;

    // Entering when Alt is being pressed
    menubar_render(menus, true, NULL);

    // Wait for Alt to be released or an accelerator key pressed
    while (1) {
        while ((key = KEYBUF) < 0);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            if ((key & KEY_KEYDOWN) == 0 && (scancode == SCANCODE_LALT || scancode == SCANCODE_RALT)) {
                break;
            }
        } else {
            uint8_t ch = key & 0xFF;

            // Find menu with matching accelerator key
            while (active_menu->title) {
                const char *p = active_menu->title;
                while (*p) {
                    if (p[0] == '&' && toupper(p[1]) == toupper(ch)) {
                        break;
                    }
                    p++;
                }
                if (*p)
                    break;

                active_menu++;
            }

            // No match found
            if (!active_menu->title) {
                // Check for shortcut
                uint16_t       shortcut = (key & (KEY_MOD_CTRL | KEY_MOD_SHIFT | KEY_MOD_ALT)) | toupper(ch);
                menu_handler_t handler  = menubar_find_shortcut(menus, shortcut);
                if (handler)
                    handler();
                return;
            }

            // Match found
            menu_open = true;
            break;
        }
    }

    int active_idx = 0;

    const struct menu_item *selected_mi = NULL;
    const struct menu      *prev_menu   = NULL;

    // Menu interaction
    while (selected_mi == NULL) {
        if (active_menu != prev_menu) {
            active_idx = 0;
            redraw_screen();
        }
        menubar_render(menus, !menu_open, active_menu);

        if (menu_open) {
            render_menu(menus, active_menu, active_idx, active_menu != prev_menu);
            prev_menu = active_menu;
        }

        while ((key = KEYBUF) < 0);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Dismiss menu on pressing and releasing alt again or pressing ESC
            if (((key & KEY_KEYDOWN) && (scancode == SCANCODE_LALT || scancode == SCANCODE_RALT)) ||
                ((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return;

        } else {
            uint8_t ch = key & 0xFF;
            if (ch == CH_RIGHT) {
                active_menu++;
                if (!active_menu->title)
                    active_menu = menus;
            } else if (ch == CH_LEFT) {
                if (active_menu == menus) {
                    while (active_menu[1].title)
                        active_menu++;
                } else {
                    active_menu--;
                }
            } else if (ch == CH_DOWN) {
                if (!menu_open) {
                    menu_open = true;
                } else {
                    active_idx++;
                    if (active_idx >= get_menuitem_count(active_menu, false)) {
                        active_idx = 0;
                    }
                }
            } else if (ch == CH_UP) {
                if (menu_open) {
                    active_idx--;
                    if (active_idx < 0) {
                        active_idx = get_menuitem_count(active_menu, false) - 1;
                    }
                }
            } else if (ch == '\r') {
                if (!menu_open) {
                    menu_open = true;
                } else {
                    selected_mi = get_menu_item_by_idx(active_menu, active_idx);
                }
            } else {
                if (!menu_open) {
                    const struct menu *m = get_menu_by_accel(menus, ch);
                    if (m) {
                        active_menu = m;
                        menu_open   = true;
                    }
                } else {
                    selected_mi = get_menu_item_by_accel(active_menu, ch);
                }
            }
        }
        if (selected_mi != NULL) {
            if (!selected_mi->is_active_handler || selected_mi->is_active_handler())
                break;

            selected_mi = NULL;
        }
    }

    redraw_screen();

    if (selected_mi && selected_mi->handler) {
        selected_mi->handler();
    }
}
