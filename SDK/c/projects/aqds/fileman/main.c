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
};

static uint8_t selected_row;
static uint8_t current_pane;

static char tmpbuf[81];

struct pane {
    char    dir_path[128];
    uint8_t page;
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

static void draw_listing(void) {
    uint8_t cur_year = get_cur_year();

    scr_putchar(222);
    for (int i = 0; i < 38; i++) {
        scr_putchar(172);
    }
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

    int8_t      dd = opendir("");
    struct stat st;
    static char filename[128];

    bool eof = false;

    for (int j = 0; j < 21; j++) {
        if (!eof) {
            int8_t res = readdir(dd, &st, filename, sizeof(filename));
            if (res == ERR_EOF) {
                eof = true;
            }
        }

        scr_putchar(214);
        if (eof) {
            for (int i = 0; i < 20; i++) {
                scr_putchar(' ');
            }
        } else {
            scr_puts_pad(filename, 20);
        }
        scr_putchar(214);
        if (eof) {
            for (int i = 0; i < 5; i++) {
                scr_putchar(' ');
            }
        } else {
            if (st.attr & 1) {
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
            for (int i = 0; i < 11; i++) {
                scr_putchar(' ');
            }
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
        scr_putchar(214);
        scr_skip();
    }

    closedir(dd);

    scr_putchar(207);
    for (int i = 0; i < 38; i++) {
        scr_putchar(172);
    }
    scr_putchar(223);
    scr_skip();
}

static const char *fn_labels[10] = {
    "Run",    // F1
    "MkFile", // F2
    "View",   // F3
    "Edit",   // F4
    "Copy",   // F5
    "RenMov", // F6
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

void main(void) {
    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    scr_to_xy(0, 0);
    text_color = 0x74;

    // 2023-01-01
    // 01-01 12:12
    draw_listing();

    scr_to_xy(40, 0);
    draw_listing();
    draw_fn_bar();

    selected_row = 0;
    // scr_set_color(1, 4, 38, 0x09);

    scr_set_color(1, 2 + selected_row, 38, 0x09);

    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;

        uint8_t new_selected_row = selected_row;
        uint8_t new_pane         = current_pane;

        switch (scancode) {
            case CH_LEFT: new_pane = 0; break;
            case CH_RIGHT: new_pane = 1; break;
            case CH_DOWN: {
                new_selected_row++;
                break;
            }
            case CH_UP: {
                if (new_selected_row > 0)
                    new_selected_row--;
                break;
            }
            default: break;
        }

        if (new_selected_row != selected_row || new_pane != current_pane) {
            scr_set_color(current_pane ? 41 : 1, 2 + selected_row, 38, 0x74);
            selected_row = new_selected_row;
            current_pane = new_pane;
            scr_set_color(current_pane ? 41 : 1, 2 + selected_row, 38, 0x09);
        }
    }
}
