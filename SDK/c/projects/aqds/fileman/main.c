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

struct pane {
    char    dir_path[128];
    uint8_t page;
    uint8_t num_items;
    bool    is_end_of_dir;
};

static char *const     pgm_binary   = (char *)0xFE80;
static char *const     pgm_argument = (char *)0xFF00;
static struct pane     left_pane    = {.dir_path = "/"};
static struct pane     right_pane   = {.dir_path = "/"};
static struct pane    *current_pane = &left_pane;
static uint8_t         selected_row;
static uint8_t         cur_year;
static struct esp_stat st;
static char            filename[128];
static bool            eof;
static bool            redraw_listing;
static bool            redraw_screen;
static const char      allowed_filename_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_$%-_@~`!(){}^#&+,;=[]";
static char            tmpbuf[128];

static const char *fn_labels[10] = {
    "Run",    // F1
    "MkFile", // F2
    "Compil", //"View",   // F3
    "Edit",   // F4
    "Copy",   // F5
    "RenMov", // F6
    "MkDir",  // F7
    "Delete", // F8
    "",       // F9
    "Quit",   // F10
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
    esp_chdir(pane->dir_path);
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
    int8_t dd = esp_opendirext("", DE_FLAG_DOTDOT, (uint16_t)pane->page * ENTRIES_PER_PAGE);
    if (dd < 0)
        eof = true;

    for (int j = 0; j < ENTRIES_PER_PAGE; j++) {
        if (!eof) {
            int8_t res = esp_readdir(dd, &st, filename, sizeof(filename));
            if (res == ERR_EOF) {
                eof = true;
            }
        }
        draw_listing_line(pane);
    }
    int8_t res          = esp_readdir(dd, &st, filename, sizeof(filename));
    pane->is_end_of_dir = (res == ERR_EOF);

    esp_closedir(dd);

    scr_putchar(207);
    for (int i = 0; i < 38; i++) {
        scr_putchar(172);
    }
    scr_putchar(223);
    scr_skip();
}

static void read_selected(void) {
    esp_chdir(current_pane->dir_path);
    int8_t dd  = esp_opendirext("", DE_FLAG_DOTDOT, (uint16_t)current_pane->page * ENTRIES_PER_PAGE + selected_row);
    int8_t res = esp_readdir(dd, &st, filename, sizeof(filename));
    esp_closedir(dd);
}

static void draw_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *title) {
    redraw_screen = true;

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

static bool string_editor(const char *title, char *str) {
    draw_window(10, 6, 60, 5, title);

    uint8_t len    = strlen(str);
    bool    redraw = true;

    while (1) {
        if (redraw) {
            uint8_t i  = 0;
            text_color = 0x7F;
            scr_to_xy(12, 8);

            for (i = 0; i < len; i++) {
                scr_putchar(str[i]);
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
            str[len] = 0;
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
                str[len++] = scancode;
                redraw     = true;
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

static void hide_selection(struct pane *pane, uint8_t row) {
    scr_set_color(pane == &right_pane ? 41 : 1, 2 + row, 38, 0x74);
}
static void show_selection(void) {
    scr_set_color(current_pane == &right_pane ? 41 : 1, 2 + selected_row, 38, 0x09);
}

static void cmd_run(void) {
    // RUN selected item
    read_selected();
    if (st.attr & DE_ATTR_DIR)
        return;

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
}

static void cmd_mkfile(void) {
    // Make new file
    hide_selection(current_pane, selected_row);
    *filename = 0;
    if (string_editor("Enter name for new file", filename) && *filename != '.') {
        esp_chdir(current_pane->dir_path);
        int8_t fd = esp_open(filename, FO_WRONLY | FO_CREATE | FO_EXCL);
        if (fd >= 0) {
            esp_close(fd);
        }
    }
    redraw_screen = true;
}

static void cmd_edit(void) {
    // Open item in editor
    read_selected();
    if (st.attr & DE_ATTR_DIR)
        return;

    const char *pgm = "/aqds/editor.bin";
    memcpy(pgm_binary, pgm, strlen(pgm) + 1);
    memcpy(pgm_argument, filename, 128);

    // Run editor
    __asm__("jp 0xF803");
}

static void cmd_assemble(void) {
    // Open item in editor
    read_selected();
    if (st.attr & DE_ATTR_DIR)
        return;

    const char *pgm = "/aqds/assembler.bin";
    memcpy(pgm_binary, pgm, strlen(pgm) + 1);
    memcpy(pgm_argument, filename, 128);

    // Run editor
    __asm__("jp 0xF803");
}

static int8_t open_with_dir(const char *dir, const char *path, uint8_t flags) {
    esp_cmd(ESPCMD_OPEN);
    esp_send_byte(flags);
    esp_send_bytes(dir, strlen(dir));
    esp_send_byte('/');
    esp_send_bytes(path, strlen(path) + 1);
    return (int8_t)esp_get_byte();
}

static void cmd_copy(void) {
    read_selected();
    if (st.attr & DE_ATTR_DIR)
        return;

    draw_window(10, 6, 60, 5, "Duplicate / copy file");
    scr_to_xy(12, 8);
    scr_puts("Duplicate (D) or Copy (C)?");

    const char *source_dir = current_pane->dir_path;
    const char *dest_dir   = NULL;
    strcpy(tmpbuf, filename);

    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;
        if (scancode == 'd' || scancode == 'D') {
            if (!string_editor("Enter new file name", tmpbuf))
                return;
            if (strcmp(tmpbuf, filename) == 0)
                return;

            dest_dir = source_dir;
            break;
        }
        if (scancode == 'c' || scancode == 'C') {
            if (strcmp(left_pane.dir_path, right_pane.dir_path) == 0) {
                return;
            }
            dest_dir = (current_pane == &left_pane) ? right_pane.dir_path : left_pane.dir_path;
            break;
        }
        return;
    }

    int8_t fd1 = open_with_dir(source_dir, filename, FO_RDONLY);
    if (fd1 < 0)
        return;
    int8_t fd2 = open_with_dir(dest_dir, tmpbuf, FO_WRONLY | FO_CREATE | FO_EXCL);
    if (fd2 == ERR_EXISTS) {
        if (confirm("Overwrite existing file?")) {
            fd2 = open_with_dir(dest_dir, tmpbuf, FO_WRONLY | FO_CREATE | FO_TRUNC);
        }
    }
    if (fd2 < 0) {
        esp_close(fd1);
        return;
    }

    // Copy file
    uint16_t percentage = 0;
    draw_window(10, 6, 60, 5, "Copying file");
    scr_to_xy(12, 8);
    print_4digits(percentage);
    scr_putchar('%');

    void    *tmpbuf     = (void *)0x8000;
    uint16_t bufsize    = 16384;
    uint32_t total_read = 0;

    while (1) {
        int16_t bytes_read = esp_read(fd1, tmpbuf, bufsize);
        if (bytes_read <= 0)
            break;
        esp_write(fd2, tmpbuf, bytes_read);

        total_read += bytes_read;

        percentage = (total_read * 100ULL) / st.size;

        scr_to_xy(12, 8);
        print_4digits(percentage);
    }

    esp_close(fd1);
    esp_close(fd2);
}

static void cmd_renmov(void) {
    read_selected();
    if (strcmp(filename, "..") == 0)
        return;

    draw_window(10, 6, 60, 5, "Rename / move file");
    scr_to_xy(12, 8);
    scr_puts("Rename (R) or Move (M)?");

    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;
        if (scancode == 'r' || scancode == 'R') {
            strcpy(tmpbuf, filename);
            if (string_editor("Enter new file name", tmpbuf)) {
                esp_rename(filename, tmpbuf);
            }
            return;
        }
        if (scancode == 'm' || scancode == 'M') {
            if (strcmp(left_pane.dir_path, right_pane.dir_path) == 0) {
                return;
            }

            const char *source_dir = current_pane->dir_path;
            const char *dest_dir   = (current_pane == &left_pane) ? right_pane.dir_path : left_pane.dir_path;

            uint8_t source_dir_len = strlen(source_dir);
            uint8_t dest_dir_len   = strlen(dest_dir);
            uint8_t filename_len   = strlen(filename);

            esp_cmd(ESPCMD_RENAME);
            esp_send_bytes(source_dir, source_dir_len);
            esp_send_byte('/');
            esp_send_bytes(filename, filename_len + 1);
            esp_send_bytes(dest_dir, dest_dir_len);
            esp_send_byte('/');
            esp_send_bytes(filename, filename_len + 1);
            esp_get_byte();
            return;
        }
        return;
    }
}

static void cmd_mkdir(void) {
    // Make directory
    hide_selection(current_pane, selected_row);
    *filename = 0;
    if (string_editor("Enter name for new directory", filename) && *filename != '.') {
        esp_chdir(current_pane->dir_path);
        esp_mkdir(filename);
    }
    redraw_screen = true;
}

static void cmd_delete(void) {
    // Delete file/directory
    read_selected();
    if (confirm((st.attr & DE_ATTR_DIR) ? "Delete directory?" : "Delete file?")) {
        esp_delete(filename);
    }
    redraw_screen = true;
}

static void cmd_quit(void) {
    esp_chdir(current_pane->dir_path);
    *pgm_binary = 0;

    // Go back to BASIC
    __asm__("jp 0xF800");
}

void main(void) {
    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    scr_init();

    esp_getcwd(left_pane.dir_path, sizeof(left_pane.dir_path));

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

        uint8_t      prev_selected_row = selected_row;
        struct pane *prev_pane         = current_pane;

        switch (scancode) {
            case CH_LEFT: current_pane = &left_pane; break;
            case CH_RIGHT: current_pane = &right_pane; break;
            case CH_DOWN: {
                if (selected_row == current_pane->num_items - 1) {
                    if (!current_pane->is_end_of_dir) {
                        current_pane->page++;
                        selected_row   = 0;
                        redraw_listing = true;
                    }
                } else {
                    selected_row++;
                }
                break;
            }
            case CH_UP: {
                if (selected_row == 0) {
                    if (current_pane->page > 0) {
                        current_pane->page--;
                        selected_row   = ENTRIES_PER_PAGE - 1;
                        redraw_listing = true;
                    }
                } else {
                    selected_row--;
                }
                break;
            }
            case CH_PGDN: {
                if (!current_pane->is_end_of_dir) {
                    current_pane->page++;
                    redraw_listing = true;
                } else {
                    selected_row = current_pane->num_items - 1;
                }
                break;
            }
            case CH_PGUP: {
                if (current_pane->page > 0) {
                    current_pane->page--;
                    redraw_listing = true;
                } else {
                    selected_row = 0;
                }
                break;
            }
            case CH_F1: cmd_run(); break;
            case CH_F2: cmd_mkfile(); break;
            case CH_F3: cmd_assemble(); break;
            case CH_F4: cmd_edit(); break;
            case CH_F5: cmd_copy(); break;
            case CH_F6: cmd_renmov(); break;
            case CH_F7: cmd_mkdir(); break;
            case CH_F8: cmd_delete(); break;
            case CH_F10: cmd_quit(); break;
            case CH_RETURN: {
                read_selected();
                if (st.attr & DE_ATTR_DIR) {
                    esp_chdir(filename);
                    esp_getcwd(current_pane->dir_path, sizeof(current_pane->dir_path));
                    current_pane->page = 0;
                    selected_row       = 0;
                    redraw_listing     = true;
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
            draw_listing(current_pane);
        }

        if (selected_row >= current_pane->num_items)
            selected_row = current_pane->num_items - 1;

        redraw_selection |= (prev_selected_row != selected_row || prev_pane != current_pane);
        if (redraw_selection) {
            hide_selection(prev_pane, prev_selected_row);
            show_selection();
        }
    }
}
