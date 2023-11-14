#include <aqplus.h>

#define TEXT_RAM ((uint8_t *)0x3000)

#define MAX_FILE_SIZE (10000)
#define SCRSAVE_PATH "/.editor-scrsave"
#define CLIPBOARD_PATH "/.editor-clipboard"

#define CHARS_PER_LINE (75)
#define DISPLAY_LINES (24)

#define COLOR_STATUSBAR 0x84
#define COLOR_SIDEBAR 0x8B
#define COLOR_SEPARATOR 0xBF
#define COLOR_WHITESPACE_SELECTED 0x69
#define COLOR_WHITESPACE_CURSOR 0x7E
#define COLOR_TEXT 0x8F
#define COLOR_TEXT_SELECTED 0x09
#define COLOR_CURSOR 0x7E

enum {
    CH_CTRL_A = 1,
    CH_CTRL_B = 2,
    CH_CTRL_C = 3,
    CH_CTRL_D = 4,
    CH_CTRL_E = 5,
    CH_CTRL_F = 6,
    CH_CTRL_G = 7,
    CH_CTRL_H = 8, // Same as backspace
    CH_CTRL_I = 9, // Same as tab
    CH_CTRL_J = 10,
    CH_CTRL_K = 11,
    CH_CTRL_L = 12,
    CH_CTRL_M = 13, // Same as return
    CH_CTRL_N = 14,
    CH_CTRL_O = 15,
    CH_CTRL_P = 16,
    CH_CTRL_Q = 17,
    CH_CTRL_R = 18,
    CH_CTRL_S = 19,
    CH_CTRL_T = 20,
    CH_CTRL_U = 21,
    CH_CTRL_V = 22,
    CH_CTRL_W = 23,
    CH_CTRL_X = 24,
    CH_CTRL_Y = 25,
    CH_CTRL_Z = 26,

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

struct editor_data {
    const char *path;
    uint8_t     dirty;              // Document unsaved?
    uint8_t     pad;                // !! Keep this before buf !!
    uint8_t     buf[MAX_FILE_SIZE]; // The document buffer
    uint8_t     pad2;               // !! Keep this after buf !!
    uint8_t    *split_begin;        // Begin of split (points to free character)
    uint8_t    *split_end;          // End of split (points to used character)
    uint8_t    *top_p;              // Pointer to first character in view (always <= split_begin)
    int16_t     top_line;           // Line number of first line in view
    int8_t      top_virtscreen_line;
    uint8_t     top_is_newline;
    int16_t     wanted_col;
    int16_t     cursor_line, cursor_col;
    int8_t      virtscreen_line;
    int8_t      virtscreen_col;
    uint8_t    *selection_split_begin, *selection_split_end;
};

static uint8_t  text_color;
static uint8_t *text_p;
static uint8_t  text_x;
static uint8_t  text_y;
static char     tmpstr[81];

static uint8_t        render_new_line;
static uint8_t        render_eof;
static uint8_t        render_draw_cursor;
static const uint8_t *render_tp;
static int16_t        render_current_line;
static uint8_t        render_has_selection;
static uint8_t        render_in_selection;

static struct editor_data data;

static uint8_t _stack[128];

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

static void _pad(void) {
    while (text_x) {
        _putchar(' ');
    }
}

static void render_statusbar(const char *str) {
    text_p      = TEXT_RAM + 24 * 80;
    text_x      = 0;
    text_y      = 24;
    uint8_t *pe = text_p;

    text_color = COLOR_STATUSBAR;
    while (*str) {
        _putchar(*(str++));
    }
    _pad();
}

static void render_screen(int render_status) {
    _reset_text();

    render_new_line      = data.top_is_newline;
    render_eof           = 0;
    render_draw_cursor   = 0;
    render_tp            = data.top_p;
    render_current_line  = data.top_line;
    render_has_selection = data.selection_split_begin != NULL;
    render_in_selection  = render_has_selection && render_tp > data.selection_split_begin;

    while (text_y != 24) {
        // Draw line number
        text_color = COLOR_SIDEBAR;
        if (render_eof) {
            _putchar(' ');
            _putchar(' ');
            _putchar(' ');
            _putchar(' ');
        } else if (render_new_line) {
            render_new_line = 0;

            sprintf(tmpstr, "%4d", render_current_line + 1);
            for (uint8_t i = 0; i < 4; i++) {
                _putchar(tmpstr[i]);
            }
        } else {
            _putchar(' ');
            _putchar(0xC6);
            _putchar(0xC6);
            _putchar(' ');
        }

        // Draw separator
        text_color = COLOR_SEPARATOR;
        _putchar(0xB5);

        if (!render_eof) {
            while (text_x) {
                uint8_t ch;

                if (render_has_selection && render_tp == data.selection_split_begin || render_tp == data.selection_split_end) {
                    render_in_selection = !render_in_selection;
                }

                // Get character
                {
                    if (render_tp == data.split_begin) {
                        render_tp          = data.split_end;
                        render_draw_cursor = 1;

                        if (render_has_selection) {
                            render_in_selection = !render_in_selection;
                        }
                    }
                    if (render_tp >= data.buf + MAX_FILE_SIZE) {
                        render_eof = 1;
                        break;
                    } else {
                        ch = *(render_tp++);
                    }
                }

                if (ch == '\n') {
                    render_current_line++;
                    render_new_line = 1;
                    break;
                }
                text_color = render_draw_cursor ? COLOR_CURSOR : (render_in_selection ? COLOR_TEXT_SELECTED : COLOR_TEXT);

                if (ch == ' ' && render_in_selection) {
                    // Draw whitespace with centered-dot character
                    ch         = 0xC6;
                    text_color = render_draw_cursor ? COLOR_WHITESPACE_CURSOR : COLOR_WHITESPACE_SELECTED;
                }

                _putchar(ch);
                render_draw_cursor = 0;
            }

            // Draw text
            if (text_x) {
                text_color = render_draw_cursor ? COLOR_CURSOR : (render_in_selection ? COLOR_TEXT_SELECTED : COLOR_TEXT);
                _putchar(' ');
                render_draw_cursor = 0;
            }
        }
        text_color = COLOR_TEXT;
        _pad();
    }

    // Render status bar
    if (render_status) {
        unsigned bytes_free = data.split_end - data.split_begin;
        sprintf(
            tmpstr, "Ln %d, Col %d   %u bytes free.   %s%c",
            data.cursor_line + 1, data.cursor_col + 1, bytes_free, data.path, data.dirty ? '*' : ' ');

        render_statusbar(tmpstr);
    }
}

static int load_file(const char *path) {
    struct stat st;

    int16_t result = stat(path, &st);
    if (result < 0)
        return result;
    if (st.attr & 1) {
        // Directory
        return -1;
    }
    if (st.size > MAX_FILE_SIZE) {
        // printf("File too big! (max %u bytes)\n", MAX_FILE_SIZE);
        return -1;
    }

    // Clear load area
    memset((void *)data.buf, 0, MAX_FILE_SIZE);

    int8_t fd = open(path, FO_RDONLY);
    if (fd < 0)
        return fd;

    data.split_begin = data.buf;
    data.split_end   = data.buf + (MAX_FILE_SIZE - st.size);

    result = read(fd, data.split_end, st.size);
    if (result < 0) {
        return result;
    }
    close(fd);

    data.path           = path;
    data.top_p          = data.buf;
    data.top_line       = 0;
    data.top_is_newline = 1;

    return 0;
}

static int save_file(const char *path) {
    int8_t fd = open(path, FO_WRONLY | FO_CREATE | FO_TRUNC);
    if (fd < 0)
        return fd;
    write(fd, data.buf, data.split_begin - data.buf);
    write(fd, data.split_end, data.buf + MAX_FILE_SIZE - data.split_end);

    close(fd);
    return 0;
}

static void new_empty_file(const char *path) {
    data.split_begin    = data.buf;
    data.split_end      = data.buf + MAX_FILE_SIZE;
    data.top_p          = data.buf;
    data.top_line       = 0;
    data.top_is_newline = 1;
    data.path           = path;
    data.dirty          = 1;
}

static int push_before_split(uint8_t ch) {
    if (data.split_begin >= data.split_end) {
        return -1;
    }
    *(data.split_begin++) = ch;
    data.cursor_col++;

    if (ch == '\n' || ++data.virtscreen_col == CHARS_PER_LINE) {
        if (ch == '\n') {
            data.cursor_col = 0;
            data.cursor_line++;
        }

        data.virtscreen_col = 0;
        data.virtscreen_line++;
    }

    return 0;
}

static int peek_before_split(void) {
    return data.split_begin[-1];
}

static int peek_after_split(void) {
    return *data.split_end;
}

static int pop_before_split(void) {
    uint8_t ch;

    if (data.split_begin <= data.buf) {
        return -1;
    }

    data.cursor_col--;

    ch = *(--data.split_begin);
    if (ch == '\n' || --data.virtscreen_col < 0) {
        const uint8_t *p = data.split_begin;
        while (*(--p) != '\n') {
        }

        if (ch == '\n') {
            data.cursor_line--;
            data.cursor_col = (data.split_begin - p - 1);
        }

        data.virtscreen_col = (data.split_begin - p - 1) % CHARS_PER_LINE;
        data.virtscreen_line--;
    }

    return ch;
}

static int push_after_split(uint8_t ch) {
    if (data.split_begin >= data.split_end) {
        return -1;
    }
    *(--data.split_end) = ch;
    return 0;
}

static int pop_after_split(void) {
    if (data.split_end >= data.buf + MAX_FILE_SIZE) {
        return -1;
    }
    return *(data.split_end++);
}

static int cursor_left(void) {
    int ch = pop_before_split();
    if (ch >= 0) {
        push_after_split(ch);
    }
    return ch;
}

static int cursor_right(void) {
    int ch = pop_after_split();
    if (ch >= 0) {
        push_before_split(ch);
    }
    return ch;
}

static void cursor_home(void) {
    while (peek_before_split() != '\n') {
        cursor_left();
    }
}

static void cursor_end(void) {
    while (peek_after_split() != '\n') {
        cursor_right();
    }
}

static void cursor_next_line(void) {
    int     i;
    uint8_t newline_found = 0;

    // First try to advance 1 display line within current line
    for (i = 0; i < CHARS_PER_LINE; i++) {
        if (cursor_right() == '\n') {
            newline_found = 1;
            break;
        }
    }

    // If newline found advance to wanted column
    if (newline_found) {
        for (i = 0; i < data.wanted_col; i++) {
            if (peek_after_split() == '\n') {
                break;
            }
            cursor_right();
        }
    }
}

static void cursor_prev_line(void) {
    int     i;
    uint8_t newline_found = 0;

    // First try to go back 1 display line within current line
    for (i = 0; i < CHARS_PER_LINE; i++) {
        if (cursor_left() == '\n') {
            newline_found = 1;
            break;
        }
    }

    if (newline_found) {
        // Go to start of virtual line
        while (data.virtscreen_col > 0) {
            cursor_left();
        }

        // Advance to wanted column
        for (i = 0; i < data.wanted_col; i++) {
            if (peek_after_split() == '\n') {
                break;
            }
            cursor_right();
        }
    }
}

static void cursor_page_up(void) {
    int i;
    for (i = 0; i < DISPLAY_LINES - 1; i++) {
        cursor_prev_line();
    }
}

static void cursor_page_down(void) {
    int i;
    for (i = 0; i < DISPLAY_LINES - 1; i++) {
        cursor_next_line();
    }
}

static void scroll_down(void) {
    int i = 0;
    data.top_virtscreen_line++;

    for (i = 0; i < CHARS_PER_LINE; i++) {
        if (*(data.top_p++) == '\n') {
            data.top_is_newline = 1;
            data.top_line++;
            return;
        }
    }
    data.top_is_newline = 0;
}

static void scroll_up(void) {
    int i = 0;
    data.top_virtscreen_line--;

    if (data.top_p[-1] == '\n') {
        data.top_line--;
    }

    for (i = 0; i < CHARS_PER_LINE; i++) {
        data.top_p--;
        if (data.top_p[-1] == '\n') {
            data.top_is_newline = 1;
            // data.top_line--;
            return;
        }
    }
    data.top_is_newline = 0;
}

static void do_backspace(void) {
    // Check how many contiguous spaces until start of line
    const uint8_t *p      = data.split_begin;
    int            spaces = 0;
    while (1) {
        uint8_t val = *(--p);
        if (val == '\n') {
            break;
        }
        if (val == ' ') {
            spaces++;
        } else {
            spaces = -1;
            break;
        }
    }

    if (spaces < 1) {
        spaces = 1;
    }

    // Backspace until spaces at multiple of 4
    do {
        pop_before_split();
        spaces--;
    } while (spaces % 4 != 0);

    data.dirty = 1;
}

static void do_delete(void) {
    pop_after_split();
    data.dirty = 1;
}

static void do_tab(void) {
    int spaces;

    push_before_split(' ');
    spaces = 4 - data.cursor_col & 3;
    while (spaces--) {
        push_before_split(' ');
    }
    data.dirty = 1;
}

static void do_enter(void) {
    int indent_spaces = 0;

    // Get number of spaces at start of line
    const uint8_t *p = data.split_begin;
    while (*(--p) != '\n') {
    }
    p++;
    while (p < data.split_begin && *(p++) == ' ') {
        indent_spaces++;
    }

    push_before_split('\n');
    while (indent_spaces--) {
        push_before_split(' ');
    }

    data.dirty = 1;
}

static void delete_selection(void) {
    if (data.selection_split_begin == NULL) {
        return;
    }

    if (data.selection_split_begin < data.split_begin) {
        int num_bytes = data.split_begin - data.selection_split_begin;
        while (num_bytes--) {
            // Use backspace
            pop_before_split();
        }

    } else if (data.selection_split_end > data.split_end) {
        int num_bytes = data.selection_split_end - data.split_end;
        while (num_bytes--) {
            // Use delete
            pop_after_split();
        }
    }

    data.selection_split_begin = NULL;
    data.selection_split_end   = NULL;
}

static int save_selection(void) {
    int      selected_bytes = 0;
    uint8_t *p              = NULL;

    if (data.selection_split_begin == NULL) {
        return 0;
    }

    if (data.selection_split_begin < data.split_begin) {
        p              = data.selection_split_begin;
        selected_bytes = data.split_begin - data.selection_split_begin;

    } else if (data.selection_split_end > data.split_end) {
        p              = data.split_end;
        selected_bytes = data.selection_split_end - data.split_end;
    }

    if (selected_bytes == 0)
        return 0;

    int8_t fd = open(CLIPBOARD_PATH, FO_WRONLY | FO_CREATE | FO_TRUNC);
    if (fd < 0)
        return 0;
    write(fd, p, selected_bytes);
    close(fd);

    {
        char str[64];
        sprintf(str, "%u bytes copied.", selected_bytes);
        render_statusbar(str);
    }

    return selected_bytes;
}

static void paste_clipboard(void) {
    int     i;
    uint8_t buf[64];

    int8_t fd = open(CLIPBOARD_PATH, FO_RDONLY);
    if (fd < 0)
        return;

    while (1) {
        int16_t result = read(fd, buf, sizeof(buf));
        if (result <= 0)
            break;

        for (i = 0; i < (int)result; i++)
            push_before_split(buf[i]);

        data.dirty = 1;
    }
    close(fd);
}

static inline void esp_send_byte(uint8_t val) {
    while (IO_ESPCTRL & 2) {
    }
    IO_ESPDATA = val;
}

static inline uint8_t esp_get_byte(void) {
    while ((IO_ESPCTRL & 1) == 0) {
    }
    return IO_ESPDATA;
}

static inline void esp_cmd(uint8_t cmd) {
    IO_ESPCTRL = 0x83;
    esp_send_byte(cmd);
}

int main(void) {
    for (int i = 0; i < 128; i++) {
        _stack[i] = 0xAA;
    }

    __asm__("ld sp,#(__stack + 128)");

    // IO_VCTRL   = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    IO_SYSCTRL = (1 << 2); // Turbo mode

    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    // _reset_text();
    // text_color = COLOR_TEXT;
    // _putchar('A');
    // _pad();

    // while (1) {
    // }

    uint8_t quit         = 0;
    uint8_t prev_shifted = 0;

    data.pad  = '\n';
    data.pad2 = '\n';

    const char *path = "Blaat2.txt";

    if (load_file(path) < 0) {
        new_empty_file(path);
    }

    render_screen(1);

    while (!quit) {
        uint8_t do_render     = 0;
        uint8_t render_status = 1;
        int     scancode;

        while ((scancode = IO_KEYBUF) > 0) {
            uint8_t keep_selection       = 0;
            uint8_t update_wanted_col    = 1;
            uint8_t shifted              = (IO_KEYBOARD_COL7 & 0x10) == 0; //(scancode >= 128) && (scancode & KEY_SHIFTED) != 0;
            uint8_t render_has_selection = (data.selection_split_begin != NULL);
            render_status                = 1;
            do_render                    = 1;

            if (scancode == CH_UP || scancode == CH_DOWN || scancode == CH_CTRL_S || scancode == CH_CTRL_Q) {
                update_wanted_col = 0;
            }

            if (scancode >= ' ' && scancode < 127) {
                push_before_split(scancode);
                data.dirty = 1;

            } else {
                uint8_t *prev_split_begin = data.split_begin;
                uint8_t *prev_split_end   = data.split_end;

                switch (scancode) {
                    case CH_RETURN:
                        do_enter();
                        break;

                    case CH_BACKSPACE:
                        if (render_has_selection)
                            delete_selection();
                        else
                            do_backspace();
                        break;

                    case CH_DELETE:
                        if (render_has_selection)
                            delete_selection();
                        else
                            do_delete();
                        break;

                    case CH_TAB:
                        // if (render_has_selection) {
                        //     // TODO: indent selection
                        //     keep_selection = 1;
                        // } else {
                        do_tab();
                        // }
                        break;

                    case CH_RIGHT: cursor_right(); break;
                    case CH_LEFT: cursor_left(); break;
                    case CH_DOWN: cursor_next_line(); break;
                    case CH_UP: cursor_prev_line(); break;
                    case CH_HOME: cursor_home(); break;
                    case CH_END: cursor_end(); break;
                    case CH_PGDN: cursor_page_down(); break;
                    case CH_PGUP: cursor_page_up(); break;

                    case CH_CTRL_S:
                        if (data.dirty) {
                            int result = save_file(data.path);
                            if (result != 0) {
                                // render_statusbar(fatfs_error(result));
                                render_status = 0;
                                break;
                            }
                            data.dirty = 0;
                        }
                        break;

                    case CH_CTRL_Q:
                        if (data.dirty) {
                            render_statusbar("Are you sure you want to quit? (Unchanged changes will be lost.)");
                            while ((scancode = IO_KEYBUF) == 0) {
                            }
                            if (scancode == 'y' || scancode == 'Y') {
                                quit = 1;
                            }
                        } else {
                            quit = 1;
                        }
                        break;

                    case CH_CTRL_C: {
                        if (save_selection()) {
                            keep_selection = 1;
                            render_status  = 0;
                        }
                        break;
                    }

                    case CH_CTRL_X: {
                        if (save_selection()) {
                            render_status = 0;
                            delete_selection();
                        }
                        break;
                    }

                    case CH_CTRL_V: {
                        paste_clipboard();
                        break;
                    }
                }

                if (shifted && !prev_shifted) {
                    data.selection_split_begin = prev_split_begin;
                    data.selection_split_end   = prev_split_end;
                }
                if (!keep_selection) {
                    prev_shifted = shifted;
                }
            }

            if (!keep_selection && !shifted) {
                data.selection_split_begin = NULL;
                data.selection_split_end   = NULL;
            }

            if (update_wanted_col) {
                data.wanted_col = data.virtscreen_col;
            }
        }

        while (data.virtscreen_line - data.top_virtscreen_line < 0) {
            scroll_up();
        }

        while (data.virtscreen_line - data.top_virtscreen_line >= DISPLAY_LINES) {
            scroll_down();
        }

        if (do_render) {
            render_screen(render_status);
        }
    }

    IO_VCTRL   = VCTRL_TEXT_EN;
    IO_SYSCTRL = 0;
    __asm__(
        "ld sp,#0x38A0\n"
        "jp 0x2003");
    return 0;
}
