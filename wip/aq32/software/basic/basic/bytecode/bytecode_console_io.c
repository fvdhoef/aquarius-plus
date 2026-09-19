#include "bytecode_internal.h"
#include "console.h"
#include "readline.h"
#include "common/parsenum.h"

void bc_stmt_cls(void) {
    console_clear_screen();
}

void bc_stmt_color(void) {
    int color_background = bc_stack_pop_long();
    int color_foreground = bc_stack_pop_long();

    if (color_background != INT32_MIN) {
        if (color_background < 0 || color_background > 15)
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        console_set_background_color(color_background);
    }

    if (color_foreground != INT32_MIN) {
        if (color_foreground < 0 || color_foreground > 15)
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        console_set_foreground_color(color_foreground);
    }
}

void bc_stmt_locate(void) {
    int cursor_on = bc_stack_pop_long();
    int column    = bc_stack_pop_long();
    int row       = bc_stack_pop_long();

    if (cursor_on != INT32_MIN) {
        if (cursor_on < 0 || cursor_on > 1)
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        console_show_cursor(cursor_on != 0);
    }

    if (column != INT32_MIN) {
        if (column < 1 || column > console_get_num_columns())
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        console_set_cursor_column(column - 1);
    }

    if (row != INT32_MIN) {
        if (row < 1 || row > console_get_num_rows())
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        console_set_cursor_row(row - 1);
    }
}

void bc_func_csrlin(void) {
    bc_stack_push_long(console_get_cursor_row() + 1);
}

void bc_func_pos(void) {
    bc_stack_push_long(console_get_cursor_column() + 1);
}

void bc_print_val(void) {
    stkval_t *val = bc_stack_pop();
    switch (val->type) {
        case VT_LONG: {
            char tmp[64];
            int  len = snprintf(tmp, sizeof(tmp), "%s%d ", val->val_long >= 0 ? " " : "", (int)val->val_long);
            if (file_io_cur_file < 0) {
                console_puts(tmp);
            } else {
                file_io_write(file_io_cur_file, tmp, len);
            }
            break;
        }
        case VT_SINGLE: {
            char tmp[64];
            int  len = snprintf(tmp, sizeof(tmp), "%s%.7g ", val->val_single >= 0 ? " " : "", (double)val->val_single);
            if (file_io_cur_file < 0) {
                console_puts(tmp);
            } else {
                file_io_write(file_io_cur_file, tmp, len);
            }
            break;
        }
        case VT_DOUBLE: {
            char tmp[64];
            int  len = snprintf(tmp, sizeof(tmp), "%s%.16lg ", val->val_double >= 0 ? " " : "", val->val_double);
            if (file_io_cur_file < 0) {
                console_puts(tmp);
            } else {
                file_io_write(file_io_cur_file, tmp, len);
            }
            break;
        }
        case VT_STR: {
            const uint8_t *p = val->val_str.p;
            if (file_io_cur_file < 0) {
                const uint8_t *p_end = p + val->val_str.length;
                while (p < p_end)
                    console_putc(*(p++));
            } else {
                file_io_write(file_io_cur_file, p, val->val_str.length);
            }
            bc_free_temp_val(val);
            break;
        }
    }
}

void bc_print_spc(void) {
    int val = bc_stack_pop_long();
    if (file_io_cur_file < 0) {
        for (int i = 0; i < val; i++)
            console_putc(' ');
    } else {
        for (int i = 0; i < val; i++)
            file_io_write(file_io_cur_file, " ", 1);
    }
}

void bc_print_tab(void) {
    int val = bc_stack_pop_long();
    if (val < 1)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    val -= 1;

    if (file_io_cur_file < 0) {
        val %= console_get_num_columns();
        while (console_get_cursor_column() != val)
            console_putc(' ');
    } else {
        while (file_io_get_column(file_io_cur_file) != (unsigned)val)
            file_io_write(file_io_cur_file, " ", 1);
    }
}

void bc_print_next_field(void) {
    if (file_io_cur_file < 0) {
        int w = console_get_num_columns();
        while (1) {
            console_putc(' ');
            int column = console_get_cursor_column();
            if (column % 14 == 0 && column + 14 < w)
                break;
        }
    } else {
        while (1) {
            file_io_write(file_io_cur_file, " ", 1);
            int column = file_io_get_column(file_io_cur_file);
            if (column % 14 == 0)
                break;
        }
    }
}

void bc_print_newline(void) {
    if (file_io_cur_file < 0) {
        console_puts("\r\n");
    } else {
        file_io_write(file_io_cur_file, "\n", 1);
    }
}

void bc_set_file(void) {
    file_io_cur_file = bc_stack_pop_long();
}

void bc_unset_file(void) {
    file_io_cur_file = -1;
}

void bc_stmt_width(void) {
    int val = bc_stack_pop_long();
    if (!console_set_width(val))
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    console_set_width(val);
}

void bc_stmt_input(void) {
    char           buf[256];
    const uint8_t *start_p_cur     = bc_state.p_cur - 1;
    int            start_stack_idx = bc_state.stack_idx;

    uint8_t len = bc_get_u8();
    for (unsigned i = 0; i < len; i++)
        console_putc(bc_get_u8());

    uint8_t flags = bc_get_u8();
    if (flags & 1)
        console_puts("? ");

    int result;
    if (file_io_cur_file < 0) {
        if (flags & 2) {
            result = readline_no_newline(buf, sizeof(buf));
        } else {
            result = readline(buf, sizeof(buf));
        }
        if (result < 0) {
            // CTRL-C
            bc_state.stop = true;
            return;
        }
    } else {
        file_io_readline(file_io_cur_file, buf, sizeof(buf));
        result = strlen(buf);
    }

    const uint8_t *ps     = (const uint8_t *)buf;
    const uint8_t *ps_end = ps + result;

    bool first = true;

    while (1) {
        while (ps[0] == ' ')
            ps++;

        uint8_t var_type = bc_get_u8();
        if (var_type == 0) {
            if (ps[0] != 0)
                goto redo;
            break;
        }
        uint8_t *p_var = &bc_state.p_vars[bc_get_u16()];

        if (!first) {
            if (ps[0] != ',')
                goto redo;
            ps++;
            while (ps[0] == ' ')
                ps++;
        }

        if (var_type == '$') {
            // Find end of string
            const uint8_t *str_end = ps;
            while (str_end < ps_end) {
                if (str_end[0] == ',')
                    break;
                str_end++;
            }
            while (str_end > ps && str_end[-1] == ' ')
                str_end--;

            store_var_string(p_var, (const char *)ps, str_end - ps);
            ps = str_end;

        } else {
            bc_str_parse_val(&ps, ps_end);
            switch (var_type) {
                case '%': _store_var_int(p_var); break;
                case '&': _store_var_long(p_var); break;
                case '!': _store_var_single(p_var); break;
                case '#': _store_var_double(p_var); break;
                default: _basic_error(ERR_INTERNAL_ERROR);
            }
        }
        first = false;
    }
    return;

redo:
    if (file_io_cur_file >= 0) {
        _basic_error(ERR_INPUT_PAST_END_OF_FILE);
    }

    printf("\nRedo from start\n");
    bc_state.p_cur     = start_p_cur;
    bc_state.stack_idx = start_stack_idx;
    return;
}

void bc_stmt_line_input(void) {
    char buf[256];

    uint8_t len = bc_get_u8();
    for (unsigned i = 0; i < len; i++)
        console_putc(bc_get_u8());

    uint8_t flags = bc_get_u8();
    int     result;
    if (flags & 2) {
        result = readline_no_newline(buf, sizeof(buf));
    } else {
        result = readline(buf, sizeof(buf));
    }
    if (result < 0) {
        // CTRL-C
        bc_state.stop = true;
        return;
    }

    uint8_t *p_var = &bc_state.p_vars[bc_get_u16()];
    store_var_string(p_var, buf, result);
}

void bc_func_inkey_s(void) {
    uint8_t ch = console_getc();
    if (ch == 0) {
        bc_stack_push_temp_str(0);
    } else {
        stkval_t *stk     = bc_stack_push_temp_str(1);
        stk->val_str.p[0] = ch;
    }
}
