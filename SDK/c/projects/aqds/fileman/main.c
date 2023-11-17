#include <aqplus.h>
#include <esp.h>
#include <stdio.h>

#define TEXT_RAM ((uint8_t *)0x3000)

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

static uint8_t *text_p;
static uint8_t  text_x;
static uint8_t  text_y;
static uint8_t  text_color;
static uint8_t  text_ch;

static uint8_t selected_row;
static uint8_t current_pane;

static char tmpbuf[81];

void print_2digits(uint8_t val);
void print_4digits(uint16_t val);
void print_5digits(uint16_t val);

static void _reset_text(void) {
    text_x = 0;
    text_y = 0;
    text_p = TEXT_RAM;
}

void _putchar(uint8_t ch) {
    IO_VCTRL    = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    *text_p     = (ch);
    IO_VCTRL    = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    *(text_p++) = text_color;

    if (text_x == 79) {
        text_x = 0;
        text_y++;
    } else {
        text_x++;
    }
}

static void _puts(const char *s) {
    while (*s) {
        _putchar(*(s++));
    }
}

static bool _puts2(const char *s, uint8_t max_len) {
    while (*s && max_len) {
        _putchar(*(s++));
        max_len--;
    }
    return *s != NULL;
}

static void _puts3(const char *s, uint8_t len) {
    while (*s && len) {
        _putchar(*(s++));
        len--;
    }
    while (len) {
        _putchar(' ');
        len--;
    }
}

static void _pad(void) {
    while (text_x) {
        _putchar(' ');
    }
}

static void _skip(void) {
    text_p += 40;
    text_x += 40;
}

static void _set_color(uint8_t x, uint8_t y, uint8_t width, uint8_t color) {
    text_x   = x;
    text_y   = y;
    text_p   = TEXT_RAM + y * 80 + x;
    IO_VCTRL = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    while (width--) {
        *(text_p++) = color;
    }
}

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

    _putchar(222);
    for (int i = 0; i < 38; i++) {
        _putchar(172);
    }
    _putchar(206);
    _skip();

    text_color = 0x74;
    _putchar(214);
    text_color = 0xC4;
    _puts3("        Name", 20);
    text_color = 0x74;
    _putchar(214);
    text_color = 0xC4;
    _puts(" Size");
    text_color = 0x74;
    _putchar(214);
    text_color = 0xC4;
    _puts(" Date/Time ");
    text_color = 0x74;
    _putchar(214);

    _skip();

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

        _putchar(214);
        if (eof) {
            for (int i = 0; i < 20; i++) {
                _putchar(' ');
            }
        } else {
            _puts3(filename, 20);
        }
        _putchar(214);
        if (eof) {
            for (int i = 0; i < 5; i++) {
                _putchar(' ');
            }
        } else {
            if (st.attr & 1) {
                _puts3("  dir", 5);

            } else {
                if (st.size < 1024LU) {
                    uint16_t sz = st.size;
                    print_4digits(sz);
                    _putchar('B');
                } else if (st.size < 1024 * 1024LU) {
                    uint16_t sz = st.size >> 10;
                    print_4digits(sz);
                    _putchar('K');
                } else {
                    uint16_t sz = st.size >> 20;
                    print_4digits(sz);
                    _putchar('M');
                }
            }
        }

        _putchar(214);
        if (eof) {
            for (int i = 0; i < 11; i++) {
                _putchar(' ');
            }
        } else {
            uint8_t year = (*((uint8_t *)&st.date + 1) >> 1) + 80;

            if (cur_year == year) {
                print_2digits((st.date >> 5) & 15);
                _putchar('-');
                print_2digits(*((uint8_t *)&st.date) & 31);
                _putchar(' ');
                print_2digits(*((uint8_t *)&st.time + 1) >> 3);
                _putchar(':');
                print_2digits((st.time >> 5) & 63);
            } else {
                _putchar(' ');
                print_4digits((*((uint8_t *)&st.date + 1) >> 1) + 1980);
                _putchar('-');
                print_2digits((st.date >> 5) & 15);
                _putchar('-');
                print_2digits(*((uint8_t *)&st.date) & 31);
            }
        }
        _putchar(214);
        _skip();
    }

    closedir(dd);

    _putchar(207);
    for (int i = 0; i < 38; i++) {
        _putchar(172);
    }
    _putchar(223);
    _skip();
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
    text_x = 0;
    text_y = 24;
    text_p = TEXT_RAM + (24 * 80);

    static const char *f_values = " 1 2 3 4 5 6 7 8 910";

    for (uint8_t j = 0; j < 10; j++) {
        text_color = 0x70;
        _putchar(f_values[j * 2 + 0]);
        _putchar(f_values[j * 2 + 1]);

        text_color = 0x09;
        _puts3(fn_labels[j], 6);
    }
}

void main(void) {
    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    _reset_text();
    text_color = 0x74;

    // 2023-01-01
    // 01-01 12:12
    draw_listing();

    text_x = 40;
    text_y = 0;
    text_p = TEXT_RAM + 40;
    draw_listing();
    draw_fn_bar();

    selected_row = 0;
    // _set_color(1, 4, 38, 0x09);

    _set_color(1, 2 + selected_row, 38, 0x09);

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
            _set_color(current_pane ? 41 : 1, 2 + selected_row, 38, 0x74);
            selected_row = new_selected_row;
            current_pane = new_pane;
            _set_color(current_pane ? 41 : 1, 2 + selected_row, 38, 0x09);
        }
    }
}
