#include <aqplus.h>
#include <esp.h>
#include <stdio.h>
#include "screen.h"

enum {
    CH_BACKSPACE = 8,
    CH_TAB       = 9,
    CH_RETURN    = 13,
    CH_DELETE    = 127,
    CH_PGUP      = 0x8A,
    CH_PGDN      = 0x8B,
    CH_RIGHT     = 0x8E,
    CH_UP        = 0x8F,
    CH_END       = 0x9A,
    CH_HOME      = 0x9B,
    CH_LEFT      = 0x9E,
    CH_DOWN      = 0x9F,
    CH_F1        = 0x80,
    CH_F2        = 0x81,
    CH_F3        = 0x82,
    CH_F4        = 0x83,
    CH_F5        = 0x84,
    CH_F6        = 0x85,
    CH_F7        = 0x86,
    CH_F8        = 0x87,
    CH_F9        = 0x90,
    CH_F10       = 0x91,
    CH_F11       = 0x92,
    CH_F12       = 0x93,
};

#define ENTRIES_PER_PAGE (21)

static char *const pgm_binary   = (char *)0xFE80;
static char *const pgm_argument = (char *)0xFF00;

static uint8_t     selected_row;
static uint8_t     current_pane;
static uint8_t     cur_year;
static struct stat st;
static char        filename[128];
static bool        eof;
static bool        redraw_listing;
static bool        redraw_screen;
static const char  allowed_filename_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_$%-_@~`!(){}^#&+,;=[]";

static char tmpbuf[81];

struct pane {
    char    dir_path[128];
    uint8_t page;
    uint8_t num_items;
    bool    is_end_of_dir;
};

static struct pane left_pane = {
    .dir_path = "/",
};
static struct pane right_pane = {
    .dir_path = "/",
};

static uint8_t get_cur_year(void) {
    esp_cmd(ESPCMD_GETDATETIME);
    esp_send_byte(0);
    if (esp_get_byte() == 0) {
        uint8_t *p = tmpbuf;
        while (1) {
            uint8_t val = esp_get_byte();
            *(p++)      = val;
            if (val == 0)
                break;
        }
    }
    return 100 + (tmpbuf[2] - '0') * 10 + (tmpbuf[3] - '0');
}

static void scr_put_path(const char *s) {
    uint8_t len = 35;
    scr_putchar(172);
    if (s && *s) {
        scr_putchar(' ');
        while (*s && len) {
            scr_putchar(*(s++));
            len--;
        }
        scr_putchar(' ');
    }
    while (len) {
        scr_putchar(172);
        len--;
    }
}

static void scr_put_spaces(uint8_t count) {
    while (count--) {
        scr_putchar(' ');
    }
}

static void draw_listing_line(struct pane *pane) {
    scr_putchar(214);
    if (eof) {
        scr_put_spaces(20);
    } else {
        pane->num_items++;
        scr_puts_pad(filename, 20);
    }
    scr_putchar(214);
    if (eof) {
        scr_put_spaces(5);
    } else {
        if (st.attr & DE_ATTR_DIR) {
            scr_puts_pad("  dir", 5);

        } else {
            if (st.size < 1024LU) {
                uint16_t sz = st.size;
                print_4digits(sz);
                scr_putchar('B');
            } else if (st.size < 1024 * 1024LU) {
                uint16_t sz = st.size >> 10;
                print_4digits(sz);
                scr_putchar('K');
            } else {
                uint16_t sz = st.size >> 20;
                print_4digits(sz);
                scr_putchar('M');
            }
        }
    }

    scr_putchar(214);
    if (eof) {
        scr_put_spaces(11);
    } else {
        if (st.date == 0 && st.time == 0) {
            scr_put_spaces(11);
        } else {
            uint8_t year = (*((uint8_t *)&st.date + 1) >> 1) + 80;

            if (cur_year == year) {
                print_2digits((st.date >> 5) & 15);
                scr_putchar('-');
                print_2digits(*((uint8_t *)&st.date) & 31);
                scr_putchar(' ');
                print_2digits(*((uint8_t *)&st.time + 1) >> 3);
                scr_putchar(':');
                print_2digits((st.time >> 5) & 63);
            } else {
                scr_putchar(' ');
                print_4digits((*((uint8_t *)&st.date + 1) >> 1) + 1980);
                scr_putchar('-');
                print_2digits((st.date >> 5) & 15);
                scr_putchar('-');
                print_2digits(*((uint8_t *)&st.date) & 31);
            }
        }
    }
    scr_putchar(214);
    scr_skip();
}

static void draw_listing(struct pane *pane) {
    text_color = 0x74;
    scr_to_xy(pane == &left_pane ? 0 : 40, 0);

    pane->num_items = 0;
    chdir(pane->dir_path);
    cur_year = get_cur_year();

    scr_putchar(222);
    scr_put_path(pane->dir_path);
    scr_putchar(206);
    scr_skip();

    text_color = 0x74;
    scr_putchar(214);
    text_color = 0xC4;
    scr_puts_pad("        Name", 20);
    text_color = 0x74;
    scr_putchar(214);
    text_color = 0xC4;
    scr_puts(" Size");
    text_color = 0x74;
    scr_putchar(214);
    text_color = 0xC4;
    scr_puts(" Date/Time ");
    text_color = 0x74;
    scr_putchar(214);

    scr_skip();

    text_color = 0x74;

    eof       = false;
    int8_t dd = opendirext("", DE_FLAG_DOTDOT, (uint16_t)pane->page * ENTRIES_PER_PAGE);
    if (dd < 0)
        eof = true;

    for (int j = 0; j < ENTRIES_PER_PAGE; j++) {
        if (!eof) {
            int8_t res = readdir(dd, &st, filename, sizeof(filename));
            if (res == ERR_EOF) {
                eof = true;
            }
        }
        draw_listing_line(pane);
    }
    int8_t res          = readdir(dd, &st, filename, sizeof(filename));
    pane->is_end_of_dir = (res == ERR_EOF);

    closedir(dd);

    scr_putchar(207);
    for (int i = 0; i < 38; i++) {
        scr_putchar(172);
    }
    scr_putchar(223);
    scr_skip();
}

static void read_selected(void) {
    struct pane *pane = current_pane ? &right_pane : &left_pane;
    chdir(pane->dir_path);
    int8_t dd  = opendirext("", DE_FLAG_DOTDOT, (uint16_t)pane->page * ENTRIES_PER_PAGE + selected_row);
    int8_t res = readdir(dd, &st, filename, sizeof(filename));
    closedir(dd);
}

static void draw_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *title) {
    text_color = 0x7B;
    scr_to_xy(x, y);
    scr_putchar(222);
    scr_putchar(172);
    {
        uint8_t     len = w - 3;
        const char *s   = title;
        if (s && *s) {
            len -= 2;
            scr_putchar(' ');
            while (*s && len) {
                scr_putchar(*(s++));
                len--;
            }
            scr_putchar(' ');
        }
        while (len) {
            scr_putchar(172);
            len--;
        }
    }
    scr_putchar(206);
    y++;

    for (int j = 0; j < h - 2; j++) {
        scr_to_xy(x, y);
        scr_putchar(214);
        scr_put_spaces(w - 2);
        scr_putchar(214);
        y++;
    }

    scr_to_xy(x, y);
    scr_putchar(207);
    for (int i = 0; i < w - 2; i++) {
        scr_putchar(172);
    }
    scr_putchar(223);
}

static bool filename_editor(const char *title) {
    draw_window(10, 6, 60, 5, title);

    uint8_t len    = 0;
    bool    redraw = true;

    while (1) {
        if (redraw) {
            uint8_t i  = 0;
            text_color = 0x7F;
            scr_to_xy(12, 8);

            for (i = 0; i < len; i++) {
                scr_putchar(filename[i]);
            }
            text_color = 0x77;
            scr_putchar(' ');
            text_color = 0x7F;
            scr_put_spaces(56 - len - 1);
        }

        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;
        if (scancode == 3) // CTRL-C/Escape
            return false;
        if (scancode == CH_RETURN) {
            filename[len] = 0;
            return len > 0;
        }
        if (scancode == CH_BACKSPACE) {
            if (len > 0) {
                redraw = true;
                len--;
            }
            continue;
        }
        if (len >= 55)
            continue;

        for (int i = 0; i < sizeof(allowed_filename_chars); i++) {
            if (allowed_filename_chars[i] == scancode) {
                filename[len++] = scancode;
                redraw          = true;
                break;
            }
        }
    }
}

static bool confirm(const char *title) {
    draw_window(10, 6, 60, 5, title);
    scr_to_xy(12, 8);
    scr_puts("Are you sure? Type Y to confirm.");

    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;
        if (scancode == 'y' || scancode == 'Y')
            return true;
        return false;
    }
}

static const char *fn_labels[10] = {
    "Run",    // F1
    "MkFile", // F2
    "",       //"View",   // F3
    "Edit",   // F4
    "",       //"Copy",   // F5
    "",       //"RenMov", // F6
    "MkDir",  // F7
    "Delete", // F8
    "",       // F9
    "Quit",   // F10
};

void draw_fn_bar(void) {
    scr_to_xy(0, 24);

    static const char *f_values = " 1 2 3 4 5 6 7 8 910";

    for (uint8_t j = 0; j < 10; j++) {
        text_color = 0x70;
        scr_putchar(f_values[j * 2 + 0]);
        scr_putchar(f_values[j * 2 + 1]);

        text_color = 0x09;
        scr_puts_pad(fn_labels[j], 6);
    }
}

static void hide_selection(void) {
    scr_set_color(current_pane ? 41 : 1, 2 + selected_row, 38, 0x74);
}
static void show_selection(void) {
    scr_set_color(current_pane ? 41 : 1, 2 + selected_row, 38, 0x09);
}

void main(void) {
    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    getcwd(left_pane.dir_path, sizeof(left_pane.dir_path));

    draw_listing(&left_pane);
    draw_listing(&right_pane);
    draw_fn_bar();

    selected_row = 0;
    // scr_set_color(1, 4, 38, 0x09);

    scr_set_color(1, 2 + selected_row, 38, 0x09);

    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;

        uint8_t      new_selected_row = selected_row;
        uint8_t      new_pane         = current_pane;
        struct pane *pane             = new_pane ? &right_pane : &left_pane;

        switch (scancode) {
            case CH_LEFT: {
                new_pane = 0;
                pane     = &left_pane;
                break;
            }
            case CH_RIGHT: {
                new_pane = 1;
                pane     = &right_pane;
                break;
            }
            case CH_DOWN: {
                if (new_selected_row == pane->num_items - 1) {
                    if (!pane->is_end_of_dir) {
                        pane->page++;
                        new_selected_row = 0;
                        redraw_listing   = true;
                    }
                } else {
                    new_selected_row++;
                }
                break;
            }
            case CH_UP: {
                if (new_selected_row == 0) {
                    if (pane->page > 0) {
                        pane->page--;
                        new_selected_row = ENTRIES_PER_PAGE - 1;
                        redraw_listing   = true;
                    }
                } else {
                    new_selected_row--;
                }
                break;
            }
            case CH_PGDN: {
                if (!pane->is_end_of_dir) {
                    pane->page++;
                    redraw_listing = true;
                } else {
                    new_selected_row = pane->num_items - 1;
                }
                break;
            }
            case CH_PGUP: {
                if (pane->page > 0) {
                    pane->page--;
                    redraw_listing = true;
                } else {
                    new_selected_row = 0;
                }
                break;
            }
            case CH_F1: {
                // RUN selected item
                read_selected();
                if (st.attr & DE_ATTR_DIR)
                    break;

                char       *p       = pgm_binary;
                const char *runcmd  = " RUN \"";
                const char *runcmd2 = "\"\r";

                uint8_t sl = strlen(runcmd);
                memcpy(p, runcmd, sl);
                p += sl;

                sl = strlen(filename);
                memcpy(p, filename, sl);
                p += sl;

                sl = strlen(runcmd2) + 1;
                memcpy(p, runcmd2, sl);

                // Go back to BASIC
                __asm__("jp 0xF800");
                break;
            }
            case CH_F2: {
                // Make new file
                hide_selection();
                *filename = 0;
                if (filename_editor("Enter name for new file") && *filename != '.') {
                    int8_t fd = open(filename, FO_WRONLY | FO_CREATE | FO_EXCL);
                    if (fd >= 0) {
                        close(fd);
                    }
                }
                redraw_screen = true;
                break;
            }
            case CH_F4: {
                // Open item in editor
                read_selected();
                if (st.attr & DE_ATTR_DIR)
                    break;

                const char *pgm = "/aqds/editor.bin";
                memcpy(pgm_binary, pgm, strlen(pgm) + 1);
                memcpy(pgm_argument, filename, 128);

                // Run editor
                __asm__("jp 0xF803");
                break;
            }
            case CH_F7: {
                // Make directory
                hide_selection();
                *filename = 0;
                if (filename_editor("Enter name for new directory") && *filename != '.') {
                    mkdir(filename);
                }
                redraw_screen = true;
                break;
            }
            case CH_F8: {
                // Delete file/directory
                read_selected();
                if (confirm((st.attr & DE_ATTR_DIR) ? "Delete directory?" : "Delete file?")) {
                    delete (filename);
                }
                redraw_screen = true;
                break;
            }

            case CH_F10: {
                chdir(pane->dir_path);
                *pgm_binary = 0;

                // Go back to BASIC
                __asm__("jp 0xF800");
                break;
            }
            case CH_RETURN: {
                read_selected();
                if (st.attr & DE_ATTR_DIR) {
                    chdir(filename);
                    getcwd(pane->dir_path, sizeof(pane->dir_path));
                    pane->page       = 0;
                    new_selected_row = 0;
                    redraw_listing   = true;
                }
                break;
            }

            default: break;
        }

        bool redraw_selection = redraw_listing | redraw_screen;

        if (redraw_screen) {
            redraw_screen  = false;
            redraw_listing = false;
            draw_listing(&left_pane);
            draw_listing(&right_pane);
            draw_fn_bar();
        }

        if (redraw_listing) {
            redraw_listing = false;
            draw_listing(pane);
        }

        if (new_selected_row >= pane->num_items)
            new_selected_row = pane->num_items - 1;

        redraw_selection |= (new_selected_row != selected_row || new_pane != current_pane);
        if (redraw_selection) {
            hide_selection();
            selected_row = new_selected_row;
            current_pane = new_pane;
            show_selection();
        }
    }
}
