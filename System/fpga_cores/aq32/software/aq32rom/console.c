#include "console.h"
#include "lib.h"

#define DEF_FGCOL (7)
#define DEF_BGCOL (4)

struct terminal_data {
    uint16_t *buffer;
    int       rows, columns;
    int       cursor_row, cursor_col;
    uint8_t   saved_color;
    uint8_t   escape_idx;
    uint8_t   escape_cmd[16];
    uint8_t  *cmd_params;
    uint8_t   last_char;
    uint8_t   attributes;
    uint8_t   fg_col;
    uint8_t   bg_col;
    uint8_t   text_color;
};

static void memmove16(uint16_t *dst, const uint16_t *src, unsigned count) {
    uint16_t       *d = dst;
    const uint16_t *s = src;
    if (d < s) {
        while (count--) {
            *(d++) = *(s++);
        }
    } else {
        s += (count - 1);
        d += (count - 1);
        while (count--) {
            *(d--) = *(s--);
        }
    }
}

static void memset16(uint16_t *dst, uint16_t val, unsigned count) {
    while (count--)
        *(dst++) = val;
}

static void clear_display(struct terminal_data *td, int n) {
    if (n == 0) {
        // erase from begin of display up to and including current cursor position
        memset16(
            td->buffer,
            (td->text_color << 8) | ' ',
            td->columns * td->cursor_row + td->cursor_col + 1);

    } else if (n == 1) {
        // erase from current cursor position (inclusive) to end of display
        memset16(
            td->buffer + td->columns * td->cursor_row + td->cursor_col,
            (td->text_color << 8) | ' ',
            (td->columns * td->rows) - (td->columns * td->cursor_row + td->cursor_col));

    } else { // 2
        // erase entire display
        memset16(
            td->buffer,
            (td->text_color << 8) | ' ',
            td->columns * td->rows);
    }
}

static void clear_line(struct terminal_data *td, int n) {
    if (n == 0) {
        // erase from begin of line up to and including current cursor position
        memset16(
            td->buffer + td->columns * td->cursor_row,
            (td->text_color << 8) | ' ',
            td->cursor_col + 1);

    } else if (n == 1) {
        // erase from current cursor position (inclusive) to end of line
        memset16(
            td->buffer + td->columns * td->cursor_row + td->cursor_col,
            (td->text_color << 8) | ' ',
            td->columns - td->cursor_col);

    } else { // 2
        // erase entire line
        memset16(
            td->buffer + td->columns * td->cursor_row,
            (td->text_color << 8) | ' ',
            td->columns);
    }
}

static void insert_line(struct terminal_data *td, int n) {
    if (n < 1)
        return;
    if (n > td->rows)
        n = td->rows;

    // Inserts <n> lines into the buffer at the cursor position.
    // The line the cursor is on, and lines below it, will be shifted downwards.

    int row1 = td->cursor_row;
    int row2 = td->cursor_row + n;
    if (row2 > td->rows)
        row2 = td->rows;

    uint16_t *p1     = td->buffer + td->columns * row1;
    uint16_t *p2     = td->buffer + td->columns * row2;
    int       count1 = td->rows - row2;
    int       count2 = row2 - row1;

    if (count1 > 0)
        memmove16(p2, p1, count1 * td->columns);
    if (count2 > 0)
        memset16(p1, (td->text_color << 8) | ' ', count2 * td->columns);
}

static void insert_char(struct terminal_data *td, int n) {
    if (n < 1)
        return;

    uint16_t *p    = td->buffer + td->cursor_col * td->columns;
    int       col1 = td->cursor_col;
    int       col2 = td->cursor_col + n;
    if (col2 > td->columns)
        col2 = td->columns;

    uint16_t *p1     = p + col1;
    uint16_t *p2     = p + col2;
    int       count1 = td->columns - col2;
    int       count2 = col2 - col1;

    if (count1 > 0)
        memmove16(p2, p1, count1);
    if (count2 > 0)
        memset16(p1, (td->text_color << 8) | ' ', count2);
}

static void delete_line(struct terminal_data *td, int n) {
    if (n < 1)
        return;
    if (n > td->rows)
        n = td->rows;

    int y1 = td->cursor_row;
    int y2 = td->cursor_row + n;
    int y3 = td->rows - n;
    if (y3 < td->cursor_row)
        y3 = td->cursor_row;

    uint16_t *p1     = td->buffer + td->columns * y1;
    uint16_t *p2     = td->buffer + td->columns * y2;
    uint16_t *p3     = td->buffer + td->columns * y3;
    int       count1 = td->rows - y2;
    int       count2 = td->rows - y3;

    if (count1 > 0)
        memmove16(p1, p2, count1 * td->columns);
    if (count2 > 0)
        memset16(p3, (td->text_color << 8) | ' ', count2 * td->columns);
}

static void delete_char(struct terminal_data *td, int n) {
    if (n < 1)
        return;

    uint16_t *p  = td->buffer + td->cursor_row * td->columns;
    int       x1 = td->cursor_col;
    int       x2 = td->cursor_col + n;
    int       x3 = td->columns - n;
    if (x3 < td->cursor_col)
        x3 = td->cursor_col;

    uint16_t *p1     = p + x1;
    uint16_t *p2     = p + x2;
    uint16_t *p3     = p + x3;
    int       count1 = td->columns - x2;
    int       count2 = td->columns - x3;

    if (count1 > 0)
        memmove16(p1, p2, count1);
    if (count2 > 0)
        memset16(p3, (td->text_color << 8) | ' ', count2);
}

static void scroll_up(struct terminal_data *td, int n) {
    if (n < 1)
        return;
    if (n > td->rows)
        n = td->rows;

    memmove16(td->buffer, td->buffer + td->columns * n, td->columns * (td->rows - n));
    memset16(td->buffer + td->columns * (td->rows - n), (td->text_color << 8) | ' ', td->columns * n);
}

static void scroll_down(struct terminal_data *td, int n) {
    if (n < 1)
        return;
    if (n > td->rows)
        n = td->rows;

    memmove16(td->buffer + td->columns * n, td->buffer, td->columns * (td->rows - n));
    memset16(td->buffer, (td->text_color << 8) | ' ', td->columns * n);
}

static void check_cursor(struct terminal_data *td) {
    if (td->cursor_row < 0) {
        td->cursor_row = 0;
    }
    if (td->cursor_col < 0) {
        td->cursor_col = 0;
    }

    if (td->cursor_col >= td->columns) {
        td->cursor_col = 0;
        td->cursor_row++;
    }

    if (td->cursor_row >= td->rows) {
        scroll_up(td, td->cursor_row - td->rows + 1);
        td->cursor_row = td->rows - 1;
    }
}

static void hide_cursor(struct terminal_data *td) {
    if (td->buffer == NULL ||
        td->cursor_row < 0 || td->cursor_row >= td->rows ||
        td->cursor_col < 0 || td->cursor_col >= td->columns)
        return;

    uint16_t *p = &td->buffer[td->cursor_row * td->columns + td->cursor_col];
    *p          = (*p & 0xFF) | (td->saved_color << 8);
}

static void show_cursor(struct terminal_data *td) {
    if (td->buffer == NULL ||
        td->cursor_row < 0 || td->cursor_row >= td->rows ||
        td->cursor_col < 0 || td->cursor_col >= td->columns)
        return;

    uint16_t *p     = &td->buffer[td->cursor_row * td->columns + td->cursor_col];
    td->saved_color = *p >> 8;
    *p              = (*p & 0xFF) | (0x7000);
}

static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }

static inline void cursor_set(struct terminal_data *td, int col, int row) {
    td->cursor_col = imax(0, imin(col, td->columns - 1));
    td->cursor_row = imax(0, imin(row, td->rows - 1));
}

static void drawchar(struct terminal_data *td, char ch) {
    if (ch < ' ')
        return;

    td->last_char = ch;

    td->buffer[td->cursor_row * td->columns + td->cursor_col] = (td->text_color << 8) | ch;
    td->cursor_col++;
    check_cursor(td);
}

static int get_param(struct terminal_data *td, int defval) {
    if (td->cmd_params[0] < '0' || td->cmd_params[0] > '9')
        return defval;

    int n = 0;
    while (td->cmd_params[0] >= '0' && td->cmd_params[0] <= '9') {
        n = n * 10 + (td->cmd_params[0] - '0');
        td->cmd_params++;
    }
    return n;
}

static void handle_sgr(struct terminal_data *td) {
    while (1) {
        int n = get_param(td, 0);
        // printf("- %d\n", n);

        if (n == 0) {
            // Reset
            td->attributes = 0;
            td->fg_col     = 7;
            td->bg_col     = 0;
        } else if (n == 1) {
            // Bold
            td->attributes |= 1;
        } else if (n == 4) {
            // Underline
        } else if (n == 7) {
            // Reverse video
            td->attributes |= 2;
        } else if (n == 10) {
            // Primary (default) font
            td->attributes = 0;
        } else if (n == 22) {
            // Normal intensity
            td->attributes &= ~1;
        } else if (n == 24) {
            // Not underlined
        } else if (n == 27) {
            // Not reversed
            td->attributes &= ~2;
        } else if (n >= 30 && n <= 37) {
            // Foreground color
            td->fg_col = n - 30;
        } else if (n == 39) {
            // Default foreground color
            td->fg_col = 7;
        } else if (n >= 40 && n <= 47) {
            // Background color
            td->bg_col = n - 40;
        } else if (n == 49) {
            // Default background color
            td->fg_col = 0;
        } else if (n >= 90 && n <= 97) {
            // Bright foreground color
            td->fg_col = n - 90 + 8;
        } else if (n >= 100 && n <= 107) {
            // Bright background color
            td->bg_col = n - 100 + 8;
        } else {
            // Unknown
        }

        if (td->cmd_params[0] != ';')
            break;

        td->cmd_params++;
    }

    if (td->attributes & 2) {
        td->text_color = (td->bg_col << 4) | td->fg_col;
        if (td->attributes & 1) {
            td->text_color |= 0x80;
        }
    } else {
        td->text_color = (td->fg_col << 4) | td->bg_col;
        if (td->attributes & 1) {
            td->text_color |= 0x80;
        }
    }
}

static void handle_csi(struct terminal_data *td) {
    td->cmd_params          = td->escape_cmd + 2;
    int  len                = strlen((const char *)td->escape_cmd);
    char cmd                = td->escape_cmd[len - 1];
    td->escape_cmd[len - 1] = 0;

    switch (cmd) {
        case 'A': cursor_set(td, td->cursor_col, td->cursor_row - get_param(td, 1)); break;
        case 'B': cursor_set(td, td->cursor_col, td->cursor_row + get_param(td, 1)); break;
        case 'C': cursor_set(td, td->cursor_col + get_param(td, 1), td->cursor_row); break;
        case 'D': cursor_set(td, td->cursor_col - get_param(td, 1), td->cursor_row); break;
        case 'E': cursor_set(td, 0, td->cursor_row + get_param(td, 1)); break;
        case 'F': cursor_set(td, 0, td->cursor_row - get_param(td, 1)); break;
        case 'G': cursor_set(td, get_param(td, 1) - 1, td->cursor_row); break;
        case 'd': cursor_set(td, td->cursor_col, get_param(td, 1) - 1); break;
        case 'f': // Same as 'H', but not quite?
        case 'H': {
            int n = get_param(td, 1);
            if (td->cmd_params[0] == ';')
                td->cmd_params++;
            int m = get_param(td, 1);
            cursor_set(td, m - 1, n - 1);
            break;
        }
        case 'J': clear_display(td, get_param(td, 0)); break;
        case 'K': clear_line(td, get_param(td, 0)); break;
        case 'I': insert_line(td, get_param(td, 1)); break;
        case '@': insert_char(td, get_param(td, 1)); break;
        case 'P': delete_char(td, get_param(td, 1)); break;
        case 'M': delete_line(td, get_param(td, 1)); break;
        case 'S': scroll_up(td, get_param(td, 1)); break;
        case 'T': scroll_down(td, get_param(td, 1)); break;
        case 'm': handle_sgr(td); return;
        case 'b': {
            // Repeat last character n times
            int n = get_param(td, 1);
            while (n > 0) {
                drawchar(td, td->last_char);
                n--;
            }
            break;
        }
        case 'X': {
            // write N spaces w/o moving cursor
            int n = get_param(td, 1);
            if (n > td->columns - td->cursor_col)
                n = td->columns - td->cursor_col;

            memset16(td->buffer + td->columns * td->cursor_row + td->cursor_col, (td->text_color << 8) | ' ', n);
            break;
        }

        case 'h': {
            // if (cmd_params[0] == '?') {
            //     cmd_params++;
            //     int mode = get_param(0);
            //     if (mode == 1) {
            //         // Cursor Keys Mode - set: application sequences
            //         term_flags |= 1;
            //     } else if (mode == 25) {
            //         // Cursor - set: show cursor
            //         term_flags |= 2;
            //     }
            // }
            break;
        }

        case 'l':
            // if (cmd_params[0] == '?') {
            //     cmd_params++;
            //     int mode = get_param(0);
            //     if (mode == 1) {
            //         // Cursor Keys Mode - reset: cursor sequences
            //         term_flags &= ~1;
            //     } else if (mode == 25) {
            //         // Cursor - set: show cursor
            //         term_flags &= ~2;
            //     }
            // }
            break;

        case 'r': // DECSTBM: Set Top and Bottom Margins
            break;
        case 't': // DECSLPP: set page size - ie window height
            break;
        case 'n': // DSR: cursor position query
            break;
        case 'c': // RIS: restore power-on settings
            // text_color = 0x70;
            // attributes = 0;
            // term_flags = 0;
            break;
        case 's': // save cursor
            break;
        case 'u': // restore cursor
            break;

        default: {
            // text_color = 0xF5;
            // printf("\nCSI:'%c' - '%s'\n", cmd, cmd_params);
            // while (1)
            //     ;
            break;
        }
    }
}

static void terminal_process(struct terminal_data *td, char ch) {
    if (td->buffer == NULL || td->columns == 0 || td->rows == 0) {
        return;
    }

    if (td->escape_idx) {
        if (td->escape_idx < sizeof(td->escape_cmd) - 1)
            td->escape_cmd[td->escape_idx++] = ch;

        if (td->escape_cmd[1] == '[') {
            // CSI: Control Sequence Introducer
            if (td->escape_idx > 2 && ch >= '@' && ch <= '~') {
                td->escape_cmd[td->escape_idx] = 0;
                td->escape_idx                 = 0;
                handle_csi(td);
            }

        } else if (td->escape_cmd[1] == '[') {
            // Ignore OSC
            if (ch == 7 || ch == '\\') {
                td->escape_idx = 0;
            }

        } else if (td->escape_cmd[1] == 'M') {
            // insert_line(1);
            td->escape_idx = 0;

        } else {
            td->escape_idx = 0;
        }
        return;
    }

    if (ch == '\a') {
        // Terminal bell

    } else if (ch == '\b') {
        // Backspace
        if (td->cursor_col > 0) {
            td->cursor_col--;
        } else {
            if (td->cursor_row > 0) {
                td->cursor_col = td->columns - 1;
                td->cursor_row--;
            }
        }
        check_cursor(td);

    } else if (ch == '\t') {
        // Horizontal TAB
        td->cursor_col = (td->cursor_col + 8) & ~7;
        check_cursor(td);

    } else if (ch == '\n') {
        // Newline
        td->cursor_row++;
        check_cursor(td);

    } else if (ch == '\r') {
        // Carriage return
        td->cursor_col = 0;

    } else if (ch == 0x1B) {
        // Escape
        td->escape_cmd[td->escape_idx++] = ch;

    } else {
        drawchar(td, ch);
    }
}

static struct terminal_data terminal = {
    .buffer     = (uint16_t *)TRAM,
    .rows       = 25,
    .columns    = 80,
    .text_color = (DEF_BGCOL << 4) | DEF_FGCOL,
    .fg_col     = DEF_FGCOL,
    .bg_col     = DEF_BGCOL,
};

void console_init(void) {
    hide_cursor(&terminal);
    clear_display(&terminal, 2);
    show_cursor(&terminal);
}

void console_putc(char ch) {
    hide_cursor(&terminal);
    terminal_process(&terminal, ch);
    show_cursor(&terminal);
}
