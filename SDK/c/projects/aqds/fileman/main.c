#include <aqplus.h>
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

static void _reset_text(void) {
    text_x = 0;
    text_y = 0;
    text_p = TEXT_RAM;
}

static void _putchar(uint8_t ch) {
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

static void draw_listing(void) {
    _putchar(222);
    for (int i = 0; i < 38; i++) {
        _putchar(172);
    }
    _putchar(206);
    _skip();

    text_color = 0x74;
    _putchar(214);
    text_color = 0xC4;
    _puts(" Name              ");
    text_color = 0x74;
    _putchar(214);
    text_color = 0xC4;
    _puts(" Size ");
    text_color = 0x74;
    _putchar(214);
    text_color = 0xC4;
    _puts(" Date/Time ");
    text_color = 0x74;
    _putchar(214);

    _skip();

    text_color = 0x74;

    for (int j = 0; j < 21; j++) {
        _putchar(214);
        for (int i = 0; i < 19; i++) {
            _putchar(' ');
        }
        _putchar(214);
        for (int i = 0; i < 6; i++) {
            _putchar(' ');
        }
        _putchar(214);
        for (int i = 0; i < 11; i++) {
            _putchar(' ');
        }
        _putchar(214);
        _skip();
    }

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

    for (int j = 0; j < 10; j++) {
        text_color = 0x70;

        char buf[6];
        sprintf(buf, "%2d", j + 1);
        _puts(buf);

        text_color = 0x09;
        _puts3(fn_labels[j], 6);
    }
}

void main(void) {
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
            case CH_DOWN: new_selected_row++; break;
            case CH_UP: new_selected_row--; break;
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
