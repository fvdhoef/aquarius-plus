#include "bytecode_internal.h"
#include "common/parsenum.h"
#include "ctype2.h"

void bc_func_len(void) {
    stkval_t *val = bc_stack_pop_str();
    int       len = val->val_str.length;
    bc_free_temp_val(val);
    bc_stack_push_long(len);
}

void bc_func_left_s(void) {
    int n = bc_stack_pop_long();
    if (n < 0)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    stkval_t val_s = *bc_stack_pop_str();
    if (n < val_s.val_str.length) {
        stkval_t *stk = bc_stack_push_temp_str(n);
        memcpy(stk->val_str.p, val_s.val_str.p, n);
        bc_free_temp_val(&val_s);

    } else {
        // Keep current string
        *bc_stack_push() = val_s;
    }
}

void bc_func_right_s(void) {
    int n = bc_stack_pop_long();
    if (n < 0)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    stkval_t val_s = *bc_stack_pop_str();
    if (n < val_s.val_str.length) {
        stkval_t *stk = bc_stack_push_temp_str(n);
        memcpy(stk->val_str.p, val_s.val_str.p + val_s.val_str.length - n, n);
        bc_free_temp_val(&val_s);

    } else {
        // Keep current string
        *bc_stack_push() = val_s;
    }
}

void bc_func_mid_s(void) {
    int n     = bc_stack_pop_long();
    int start = bc_stack_pop_long();
    if (n < 0 || start < 1)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    start -= 1;

    stkval_t val_s = *bc_stack_pop_str();
    if (start > 0 || start + n < val_s.val_str.length) {
        if (start > val_s.val_str.length) {
            start = val_s.val_str.length;
        }
        if (start + n > val_s.val_str.length) {
            n = val_s.val_str.length - start;
        }

        stkval_t *stk = bc_stack_push_temp_str(n);
        memcpy(stk->val_str.p, val_s.val_str.p + start, n);
        bc_free_temp_val(&val_s);

    } else {
        // Keep current string
        *bc_stack_push() = val_s;
    }
}

void bc_func_asc(void) {
    stkval_t *val = bc_stack_pop_str();
    if (val->val_str.length == 0)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    uint8_t ch = val->val_str.p[0];
    bc_free_temp_val(val);
    bc_stack_push_long(ch);
}

void bc_func_instr(void) {
    stkval_t *val_substr = bc_stack_pop_str();
    stkval_t *val_str    = bc_stack_pop_str();
    int       offset     = bc_stack_pop_long();
    int       result     = 0;
    if (offset < 1)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    offset -= 1;

    for (int idx = offset; idx <= val_str->val_str.length - val_substr->val_str.length; idx++) {
        if (memcmp(val_str->val_str.p + idx, val_substr->val_str.p, val_substr->val_str.length) == 0) {
            result = idx + 1;
            break;
        }
    }

    bc_free_temp_val(val_substr);
    bc_free_temp_val(val_str);
    bc_stack_push_long(result);
}

void bc_str_parse_val(const uint8_t **ps, const uint8_t *ps_end) {
    char tmp[64];
    char type   = 0;
    int  result = -1;

    // Skip leading whitespace
    while ((*ps)[0] == ' ')
        (*ps)++;

    if ((*ps) < ps_end) {
        result = copy_num_to_buf(ps, ps_end, tmp, sizeof(tmp), &type);
    }
    if (result < 0) {
        bc_stack_push_long(0);

    } else if (result > 0) {
        int     base           = result;
        int32_t val            = strtol(tmp, NULL, base);
        bool    is_int16_range = (val >= INT16_MIN && val <= INT16_MAX);

        if (type == 0) {
            // Infer type
            type = is_int16_range ? '%' : '&';
        } else if (type == '%' && !is_int16_range) {
            _basic_error(ERR_TYPE_MISMATCH);
        }
        bc_stack_push_long(val);

    } else {
        if (type == '#') {
            bc_stack_push_double(strtod(tmp, NULL));
        } else {
            bc_stack_push_single(strtof(tmp, NULL));
        }
    }
}

void bc_func_val(void) {
    stkval_t       val_str = *bc_stack_pop_str();
    const uint8_t *ps      = val_str.val_str.p;
    const uint8_t *ps_end  = ps + val_str.val_str.length;
    bc_str_parse_val(&ps, ps_end);
    bc_free_temp_val(&val_str);
}

void bc_func_string_s(void) {
    stkval_t *val_ch = bc_stack_pop();
    uint8_t   ch;
    if (val_ch->type == VT_STR) {
        if (val_ch->val_str.length == 0)
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        ch = val_ch->val_str.p[0];
        bc_free_temp_val(val_ch);

    } else {
        bc_to_long_round(val_ch);
        if (val_ch->val_long < 0 || val_ch->val_long > 0xFF)
            _basic_error(ERR_ILLEGAL_FUNC_CALL);
        ch = val_ch->val_long;
    }

    int n = bc_stack_pop_long();
    if (n < 0 || n > INT16_MAX)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    stkval_t *stk = bc_stack_push_temp_str(n);
    memset(stk->val_str.p, ch, n);
}

void bc_func_space_s(void) {
    int n = bc_stack_pop_long();
    if (n < 0 || n > INT16_MAX)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    stkval_t *stk = bc_stack_push_temp_str(n);
    memset(stk->val_str.p, ' ', n);
}

void bc_func_str_s(void) {
    stkval_t *val = bc_stack_pop_num();
    char      tmp[32];
    int       len = 0;

    switch (val->type) {
        case VT_LONG: len = snprintf(tmp, sizeof(tmp), "%s%d", val->val_long >= 0 ? " " : "", (int)val->val_long); break;
        case VT_SINGLE: len = snprintf(tmp, sizeof(tmp), "%s%.7g", val->val_single >= 0 ? " " : "", (double)val->val_single); break;
        case VT_DOUBLE: len = snprintf(tmp, sizeof(tmp), "%s%.16lg", val->val_double >= 0 ? " " : "", val->val_double); break;
    }

    stkval_t *stk = bc_stack_push_temp_str(len);
    memcpy(stk->val_str.p, tmp, len);
}

void bc_func_chr_s(void) {
    int ch = bc_stack_pop_long();
    if (ch < 0 || ch > 255)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    stkval_t *stk     = bc_stack_push_temp_str(1);
    stk->val_str.p[0] = ch;
}

void bc_func_hex_s(void) {
    uint32_t val = bc_stack_pop_long();

    char      tmp[16];
    int       len = snprintf(tmp, sizeof(tmp), "%X", (unsigned)val);
    stkval_t *stk = bc_stack_push_temp_str(len);
    memcpy(stk->val_str.p, tmp, len);
}

void bc_func_oct_s(void) {
    uint32_t val = bc_stack_pop_long();

    char      tmp[16];
    int       len = snprintf(tmp, sizeof(tmp), "%o", (unsigned)val);
    stkval_t *stk = bc_stack_push_temp_str(len);
    memcpy(stk->val_str.p, tmp, len);
}

void bc_func_ltrim_s(void) {
    stkval_t val_s = *bc_stack_pop_str();
    int      n     = val_s.val_str.length;
    int      idx   = 0;
    while (idx < n && val_s.val_str.p[idx] == ' ')
        idx++;

    stkval_t *stk = bc_stack_push_temp_str(n - idx);
    memcpy(stk->val_str.p, val_s.val_str.p + idx, n - idx);
    bc_free_temp_val(&val_s);
}

void bc_func_rtrim_s(void) {
    stkval_t val_s = *bc_stack_pop_str();
    int      n     = val_s.val_str.length;
    while (n > 0 && val_s.val_str.p[n - 1] == ' ')
        n--;

    stkval_t *stk = bc_stack_push_temp_str(n);
    memcpy(stk->val_str.p, val_s.val_str.p, n);
    bc_free_temp_val(&val_s);
}

void bc_func_lcase_s(void) {
    stkval_t  val_s = *bc_stack_pop_str();
    int       n     = val_s.val_str.length;
    stkval_t *stk   = bc_stack_push_temp_str(n);
    for (int i = 0; i < n; i++)
        stk->val_str.p[i] = to_lower(val_s.val_str.p[i]);
    bc_free_temp_val(&val_s);
}

void bc_func_ucase_s(void) {
    stkval_t  val_s = *bc_stack_pop_str();
    int       n     = val_s.val_str.length;
    stkval_t *stk   = bc_stack_push_temp_str(n);
    for (int i = 0; i < n; i++)
        stk->val_str.p[i] = to_upper(val_s.val_str.p[i]);
    bc_free_temp_val(&val_s);
}
