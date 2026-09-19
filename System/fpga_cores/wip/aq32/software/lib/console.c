#include "console.h"

#define DEF_FGCOL    (0)
#define DEF_BGCOL    (3)
#define CURSOR_COLOR (0x80)

static void memset16(uint16_t *dst, uint16_t val, unsigned count) {
    while (count--)
        *(dst++) = val;
}

static inline int _num_columns(void) {
    return (GFX->CTRL & GFX_CTRL_TEXT_MODE80) ? 80 : 40;
}

static void hide_cursor(void) {
    if (TRAM->cursor_visible) {
        uint16_t *p_cursor   = &TRAM->text[TRAM->cursor_row * _num_columns() + TRAM->cursor_column];
        *p_cursor            = TRAM->saved_color | (*p_cursor & 0xFF);
        TRAM->cursor_visible = false;
    }
}

static void show_cursor(void) {
    if (TRAM->cursor_enabled && !TRAM->cursor_visible) {
        uint16_t *p_cursor   = &TRAM->text[TRAM->cursor_row * _num_columns() + TRAM->cursor_column];
        uint16_t  val        = *p_cursor;
        TRAM->saved_color    = val & 0xFF00;
        *p_cursor            = (CURSOR_COLOR << 8) | (val & 0xFF);
        TRAM->cursor_visible = true;
    }
}

void _clear_screen(void) {
    TRAM->cursor_row    = 0;
    TRAM->cursor_column = 0;
    memset16(TRAM->text, TRAM->text_color | ' ', TEXT_COLUMNS * TEXT_ROWS);
}

#define INITVAL1 0xAA55
#define INITVAL2 0x55AA

void console_init(void) {
    if (GFX->CTRL != (GFX_CTRL_TEXT_MODE80 | GFX_CTRL_TEXT_EN) ||
        TRAM->init_val1 != INITVAL1 ||
        TRAM->init_val2 != INITVAL2) {

        reinit_video();

        TRAM->text_color     = (DEF_FGCOL << 12) | (DEF_BGCOL << 8);
        TRAM->cursor_visible = false;
        TRAM->cursor_enabled = true;
        TRAM->cursor_color   = (CURSOR_COLOR << 8);

        _clear_screen();

        TRAM->init_val1 = INITVAL1;
        TRAM->init_val2 = INITVAL2;
    }

    hide_cursor();

    GFX->CTRL            = GFX_CTRL_TEXT_MODE80 | GFX_CTRL_TEXT_EN;
    TRAM->cursor_enabled = true;
    TRAM->text_color     = (DEF_FGCOL << 12) | (DEF_BGCOL << 8);
    TRAM->cursor_color   = (CURSOR_COLOR << 8);

    show_cursor();

    console_flush_input();
}

bool console_set_width(int width) {
    if (width != 40 && width != 80)
        return false;

    if (width != _num_columns()) {
        if (width == 80) {
            GFX->CTRL |= GFX_CTRL_TEXT_MODE80;
        } else {
            GFX->CTRL &= ~GFX_CTRL_TEXT_MODE80;
        }
        _clear_screen();
    }
    return true;
}

void console_clear_screen(void) {
    hide_cursor();
    _clear_screen();
    show_cursor();
}

void console_show_cursor(bool en) {
    hide_cursor();
    TRAM->cursor_enabled = en;
    show_cursor();
}

void console_set_cursor_row(int val) {
    hide_cursor();
    TRAM->cursor_row = clamp(val, 0, TEXT_ROWS - 1);
    show_cursor();
}
int console_get_cursor_row(void) {
    return TRAM->cursor_row;
}
int console_get_num_rows(void) {
    return TEXT_ROWS;
}

void console_set_cursor_column(int val) {
    hide_cursor();
    TRAM->cursor_column = clamp(val, 0, _num_columns() - 1);
    show_cursor();
}
int console_get_num_columns(void) { return _num_columns(); }
int console_get_cursor_column(void) { return TRAM->cursor_column; }

void console_set_foreground_color(int val) {
    TRAM->text_color = ((val & 0xF) << 12) | (TRAM->text_color & 0x0F00);
}
void console_set_background_color(int val) {
    TRAM->text_color = (TRAM->text_color & 0xF000) | ((val & 0xF) << 8);
}

static void _putc(char ch) {
    if (ch == '\b') {
        if (TRAM->cursor_column > 0) {
            TRAM->cursor_column--;
        } else {
            if (TRAM->cursor_row > 0) {
                TRAM->cursor_column = _num_columns() - 1;
                TRAM->cursor_row--;
            }
        }

    } else if (ch == '\t') {
        // Horizontal TAB
        TRAM->cursor_column = (TRAM->cursor_column + 8) & ~7;

    } else if (ch == '\n') {
        // Newline
        TRAM->cursor_row++;

    } else if (ch == '\r') {
        // Carriage return
        TRAM->cursor_column = 0;

    } else {
        // Regular character
        TRAM->text[TRAM->cursor_row * _num_columns() + TRAM->cursor_column] = TRAM->text_color | ch;
        TRAM->cursor_column++;
    }

    if (TRAM->cursor_column >= _num_columns()) {
        TRAM->cursor_column = 0;
        TRAM->cursor_row++;
    }
    if (TRAM->cursor_row >= TEXT_ROWS) {
        memmove(TRAM->text, TRAM->text + _num_columns(), _num_columns() * (TEXT_ROWS - 1) * sizeof(uint16_t));
        memset16(TRAM->text + _num_columns() * (TEXT_ROWS - 1), TRAM->text_color | ' ', _num_columns());
        TRAM->cursor_row = TEXT_ROWS - 1;
    }
}

void console_putc(char ch) {
    hide_cursor();
    _putc(ch);
    show_cursor();
}

void console_puts(const char *s) {
    hide_cursor();
    while (*s)
        _putc(*(s++));
    show_cursor();
}

static uint8_t kb_buf[64];
static uint8_t kb_wridx;
static uint8_t kb_rdidx;
static uint8_t kb_cnt;

static void _process_keybuf(void) {
    while (1) {
        int key = KEYBUF;
        if (key < 0)
            return;
        _console_handle_key(key);
        if ((key & KEY_IS_SCANCODE))
            continue;
    }
}

uint8_t console_getc(void) {
    _process_keybuf();

    if (kb_cnt == 0)
        return 0;

    uint8_t ch = kb_buf[kb_rdidx++];
    if (kb_rdidx == sizeof(kb_buf))
        kb_rdidx = 0;
    kb_cnt--;

    return ch;
}

void console_flush_input(void) {
    while (KEYBUF >= 0) {
    }
    kb_wridx = 0;
    kb_rdidx = 0;
    kb_cnt   = 0;
}

static void _dummy() {}
void        console_ctrl_c_pressed(void) __attribute__((weak, alias("_dummy")));

void _console_handle_key(int key) {
    if (key < 0)
        return;
    if (key & KEY_IS_SCANCODE)
        return;

    uint8_t ch = key & 0xFF;
    if (key & KEY_MOD_CTRL) {
        uint8_t ch_upper = toupper(ch);
        if (ch_upper >= '@' && ch_upper <= '_')
            ch = ch_upper - '@';
        else if (ch_upper == 0x7F)
            ch = '\b';
    }

    if (ch == 3)
        console_ctrl_c_pressed();

    if (kb_cnt < sizeof(kb_buf) && ch > 0) {
        kb_buf[kb_wridx++] = ch;
        if (kb_wridx == sizeof(kb_buf))
            kb_wridx = 0;
        kb_cnt++;
    }
}
