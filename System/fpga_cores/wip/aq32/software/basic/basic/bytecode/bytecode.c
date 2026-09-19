#include "bytecode.h"
#include "bytecode_internal.h"
#include "console.h"
#include "csr.h"

typedef void (*bc_handler_t)(void);
static bc_handler_t bc_handlers[];
struct bc_state     bc_state;

stkval_t *bc_stack_push_temp_str(unsigned length) {
    stkval_t *stk = bc_stack_push();

    if (length > INT16_MAX)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    uint8_t *p = NULL;
    if (length > 0 && (p = buf_malloc(length)) == NULL)
        _basic_error(ERR_OUT_OF_MEM);

    stk->type           = VT_STR;
    stk->val_str.flags  = STR_FLAGS_TYPE_TEMP;
    stk->val_str.p      = p;
    stk->val_str.length = length;
    return stk;
}

void bc_free_temp_val(stkval_t *val) {
    if (val->type != VT_STR)
        return;
    if ((val->val_str.flags & STR_FLAGS_TYPE_MSK) != STR_FLAGS_TYPE_TEMP)
        return;

    buf_free(val->val_str.p);
}

void bc_to_long_round(stkval_t *val) {
    if (val->type == VT_STR)
        _basic_error(ERR_TYPE_MISMATCH);

    if (val->type == VT_SINGLE) {
        val->type     = VT_LONG;
        val->val_long = (int32_t)roundf(val->val_single);
    } else if (val->type == VT_DOUBLE) {
        val->type     = VT_LONG;
        val->val_long = (int32_t)round(val->val_double);
    }
}
void bc_to_single(stkval_t *val) {
    if (val->type == VT_STR)
        _basic_error(ERR_TYPE_MISMATCH);

    if (val->type == VT_LONG) {
        val->type       = VT_SINGLE;
        val->val_single = val->val_long;
    } else if (val->type == VT_DOUBLE) {
        val->type       = VT_SINGLE;
        val->val_single = (float)val->val_double;
    }
}
void bc_to_double(stkval_t *val) {
    if (val->type == VT_STR)
        _basic_error(ERR_TYPE_MISMATCH);

    if (val->type == VT_LONG) {
        val->type       = VT_SINGLE;
        val->val_double = val->val_long;
    } else if (val->type == VT_SINGLE) {
        val->type       = VT_SINGLE;
        val->val_double = val->val_single;
    }
}
void bc_promote_type(stkval_t *val, unsigned type) {
    if (val->type >= type)
        return;

    if (val->type == VT_LONG) {
        if (type == VT_SINGLE) {
            val->val_single = val->val_long;
        } else if (type == VT_DOUBLE) {
            val->val_double = val->val_long;
        }
    } else if (val->type == VT_SINGLE) {
        val->val_double = val->val_single;
    }
    val->type = type;
}
void bc_promote_types(stkval_t *val_l, stkval_t *val_r) {
    if (val_l->type == VT_STR || val_r->type == VT_STR)
        _basic_error(ERR_TYPE_MISMATCH);

    if (val_r->type < val_l->type) {
        bc_promote_type(val_r, val_l->type);
    } else if (val_l->type < val_r->type) {
        bc_promote_type(val_l, val_r->type);
    }
}
void bc_promote_types_flt(stkval_t *val_l, stkval_t *val_r) {
    if (val_l->type == VT_STR || val_r->type == VT_STR)
        _basic_error(ERR_TYPE_MISMATCH);

    int type = val_l->type > val_r->type ? val_l->type : val_r->type;
    if (type < VT_SINGLE)
        type = VT_SINGLE;

    if (val_l->type < type)
        bc_promote_type(val_l, type);
    if (val_r->type < type)
        bc_promote_type(val_r, type);
}

void bc_end(void) {
    _basic_error(0);
}
void bc_line_tag(void) {
    err_line = bc_get_u16();
}
void bc_dup(void) {
    if (bc_state.stack_idx >= STACK_DEPTH)
        _basic_error(ERR_INTERNAL_ERROR);
    stkval_t *stk_from = &bc_state.stack[bc_state.stack_idx];
    if (stk_from->type == VT_STR)
        _basic_error(ERR_TYPE_MISMATCH);
    stkval_t *stk_to = bc_stack_push();
    *stk_to          = *stk_from;
}
void bc_swap(void) {
    if (bc_state.stack_idx > STACK_DEPTH - 2)
        _basic_error(ERR_INTERNAL_ERROR);

    stkval_t tmp                           = bc_state.stack[bc_state.stack_idx];
    bc_state.stack[bc_state.stack_idx]     = bc_state.stack[bc_state.stack_idx + 1];
    bc_state.stack[bc_state.stack_idx + 1] = tmp;
}
void bc_drop(void) {
    if (bc_state.stack_idx >= STACK_DEPTH)
        _basic_error(ERR_INTERNAL_ERROR);
    bc_state.stack_idx++;
}

void bc_push_const_unspecified_param(void) {
    bc_stack_push_long(INT32_MIN);
}
void bc_push_const_int(void) {
    bc_stack_push_long((int16_t)bc_get_u16());
}
void bc_push_const_long(void) {
    bc_stack_push_long(bc_get_u32());
}
void bc_push_const_single(void) {
    uint32_t u32 = bc_get_u32();
    bc_stack_push_single(*(float *)&u32);
}
void bc_push_const_double(void) {
    uint64_t u64 = bc_get_u64();
    bc_stack_push_double(*(double *)&u64);
}
void bc_push_const_string(void) {
    stkval_t *stk       = bc_stack_push();
    stk->type           = VT_STR;
    stk->val_str.flags  = STR_FLAGS_TYPE_CONST;
    stk->val_str.p      = (uint8_t *)(bc_state.p_cur + 1);
    stk->val_str.length = bc_state.p_cur[0];
    bc_state.p_cur += 1 + bc_state.p_cur[0];
}

void bc_jmp(void) {
    bc_state.p_cur = bc_state.p_buf + bc_get_u16();
}
void bc_jmp_nz(void) {
    stkval_t *val = bc_stack_pop();
    bc_to_long_round(val);

    uint16_t offset = bc_get_u16();
    if (val->val_long != 0)
        bc_state.p_cur = bc_state.p_buf + offset;
}
void bc_jmp_z(void) {
    stkval_t *val = bc_stack_pop();
    bc_to_long_round(val);

    uint16_t offset = bc_get_u16();
    if (val->val_long == 0)
        bc_state.p_cur = bc_state.p_buf + offset;
}
void bc_jsr(void) {
    uint16_t offset = bc_get_u16();
    bc_stack_push_long(bc_state.p_cur - bc_state.p_buf);
    bc_state.p_cur = bc_state.p_buf + offset;
}
void bc_return(void) {
    if (bc_state.stack_idx >= STACK_DEPTH)
        _basic_error(ERR_RETURN_WITHOUT_GOSUB);

    bc_state.p_cur = bc_state.p_buf + (uint16_t)bc_stack_pop_long();
}

void bc_func_cint(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_long_round(val);
    if (val->val_long < INT16_MIN || val->val_long > INT16_MAX)
        _basic_error(ERR_OVERFLOW);
    bc_stack_push_long(val->val_long);
}

void bc_func_clng(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_long_round(val);
    bc_stack_push_long(val->val_long);
}

void bc_func_csng(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    bc_stack_push_single(val->val_single);
}

void bc_func_cdbl(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_double(val);
    bc_stack_push_double(val->val_double);
}

void bc_stmt_error(void) {
    int err = bc_stack_pop_long();
    if (err < 1 || err > 255)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    _basic_error(err);
}

static void _find_next_data(void) {
    if (bc_state.p_cur == NULL)
        bc_state.p_cur = bc_state.p_buf;

    while (bc_state.p_cur < bc_state.p_buf_end) {
        uint8_t bc = bc_get_u8();
        if (bc == BC_DATA) {
            bc_state.p_data_end = bc_state.p_buf + bc_get_u16();
            return;
        }
        bc_state.p_cur += bc_arg_size(bc, bc_state.p_cur);
    }
    _basic_error(ERR_OUT_OF_DATA);
}

void bc_data(void) {
    // Skip data
    bc_state.p_cur = bc_state.p_buf + bc_get_u16();
}
void bc_data_read(void) {
    const uint8_t *p_save = bc_state.p_cur;
    bc_state.p_cur        = bc_state.p_data_cur;

    if (bc_state.p_data_cur == bc_state.p_data_end)
        _find_next_data();

    uint8_t bc = bc_get_u8();
    bc_handlers[bc]();

    bc_state.p_data_cur = bc_state.p_cur;
    bc_state.p_cur      = p_save;
}
void bc_data_restore(void) {
    bc_state.p_data_cur = bc_state.p_buf + bc_get_u16();
    bc_state.p_data_end = bc_state.p_data_cur;
}

void bc_stmt_clear(void) { _basic_error(ERR_UNHANDLED); }
void bc_stmt_resume(void) { _basic_error(ERR_UNHANDLED); }
void bc_stmt_timer(void) { _basic_error(ERR_UNHANDLED); }
void bc_func_erl(void) { _basic_error(ERR_UNHANDLED); }
void bc_func_err(void) { _basic_error(ERR_UNHANDLED); }

void bc_func_timer(void) {
#ifndef PCDEV
    bc_stack_push_long(csr_read(time) / 1000);
#else
    bc_stack_push_long(1234);
#endif
}

static bc_handler_t bc_handlers[] = {
    [BC_END]      = bc_end,
    [BC_LINE_TAG] = bc_line_tag,

    [BC_DUP]  = bc_dup,
    [BC_SWAP] = bc_swap,
    [BC_DROP] = bc_drop,

    [BC_OP_INC]       = bc_op_inc,
    [BC_OP_LE_GE]     = bc_op_le_ge, // Used in for loop with step, takes 3 params: step/var/end
    [BC_DATA]         = bc_data,
    [BC_DATA_READ]    = bc_data_read,
    [BC_DATA_RESTORE] = bc_data_restore,

    [BC_PUSH_CONST_UNSPECIFIED_PARAM] = bc_push_const_unspecified_param,
    [BC_PUSH_CONST_INT]               = bc_push_const_int,
    [BC_PUSH_CONST_LONG]              = bc_push_const_long,
    [BC_PUSH_CONST_SINGLE]            = bc_push_const_single,
    [BC_PUSH_CONST_DOUBLE]            = bc_push_const_double,
    [BC_PUSH_CONST_STRING]            = bc_push_const_string,

    [BC_PUSH_VAR_INT]    = bc_push_var_int,
    [BC_PUSH_VAR_LONG]   = bc_push_var_long,
    [BC_PUSH_VAR_SINGLE] = bc_push_var_single,
    [BC_PUSH_VAR_DOUBLE] = bc_push_var_double,
    [BC_PUSH_VAR_STRING] = bc_push_var_string,

    [BC_STORE_VAR_INT]    = bc_store_var_int,
    [BC_STORE_VAR_LONG]   = bc_store_var_long,
    [BC_STORE_VAR_SINGLE] = bc_store_var_single,
    [BC_STORE_VAR_DOUBLE] = bc_store_var_double,
    [BC_STORE_VAR_STRING] = bc_store_var_string,

    [BC_PUSH_ARRAY_INT]    = bc_push_array_int,
    [BC_PUSH_ARRAY_LONG]   = bc_push_array_long,
    [BC_PUSH_ARRAY_SINGLE] = bc_push_array_single,
    [BC_PUSH_ARRAY_DOUBLE] = bc_push_array_double,
    [BC_PUSH_ARRAY_STRING] = bc_push_array_string,

    [BC_STORE_ARRAY_INT]    = bc_store_array_int,
    [BC_STORE_ARRAY_LONG]   = bc_store_array_long,
    [BC_STORE_ARRAY_SINGLE] = bc_store_array_single,
    [BC_STORE_ARRAY_DOUBLE] = bc_store_array_double,
    [BC_STORE_ARRAY_STRING] = bc_store_array_string,

    [BC_DIM_ARRAY_INT]    = bc_dim_array_int,
    [BC_DIM_ARRAY_LONG]   = bc_dim_array_long,
    [BC_DIM_ARRAY_SINGLE] = bc_dim_array_single,
    [BC_DIM_ARRAY_DOUBLE] = bc_dim_array_double,
    [BC_DIM_ARRAY_STRING] = bc_dim_array_string,

    [BC_FREE_ARRAY] = bc_free_array,

    [BC_JMP]    = bc_jmp,
    [BC_JMP_NZ] = bc_jmp_nz,
    [BC_JMP_Z]  = bc_jmp_z,
    [BC_JSR]    = bc_jsr,
    [BC_RETURN] = bc_return,

    [BC_PRINT_VAL]        = bc_print_val,
    [BC_PRINT_SPC]        = bc_print_spc,
    [BC_PRINT_TAB]        = bc_print_tab,
    [BC_PRINT_NEXT_FIELD] = bc_print_next_field,
    [BC_PRINT_NEWLINE]    = bc_print_newline,
    [BC_SET_FILE]         = bc_set_file,
    [BC_UNSET_FILE]       = bc_unset_file,

    // Operator tokens
    [BC_OP_POW]    = bc_op_pow,    // TOK_POW
    [BC_OP_MULT]   = bc_op_mult,   // TOK_MULT
    [BC_OP_DIV]    = bc_op_div,    // TOK_DIV
    [BC_OP_INTDIV] = bc_op_intdiv, // TOK_INTDIV
    [BC_OP_MOD]    = bc_op_mod,    // TOK_MOD
    [BC_OP_ADD]    = bc_op_add,    // TOK_PLUS
    [BC_OP_SUB]    = bc_op_sub,    // TOK_MINUS
    [BC_OP_EQ]     = bc_op_eq,     // TOK_EQ
    [BC_OP_NE]     = bc_op_ne,     // TOK_NE
    [BC_OP_LT]     = bc_op_lt,     // TOK_LT
    [BC_OP_LE]     = bc_op_le,     // TOK_LE
    [BC_OP_GT]     = bc_op_gt,     // TOK_GT
    [BC_OP_GE]     = bc_op_ge,     // TOK_GE
    [BC_OP_NOT]    = bc_op_not,    // TOK_NOT
    [BC_OP_AND]    = bc_op_and,    // TOK_AND
    [BC_OP_OR]     = bc_op_or,     // TOK_OR
    [BC_OP_XOR]    = bc_op_xor,    // TOK_XOR
    [BC_OP_EQV]    = bc_op_eqv,    // TOK_EQV
    [BC_OP_IMP]    = bc_op_imp,    // TOK_IMP
    [BC_OP_NEGATE] = bc_op_negate,

    // Statement tokens
    [BC_FILE_CLOSE_ALL] = bc_file_close_all,
    [BC_FILE_CLOSE]     = bc_file_close,
    [BC_FILE_READ]      = bc_file_read,
    [BC_FILE_READLINE]  = bc_file_readline,
    [BC_FILE_SEEK]      = bc_file_seek,
    [BC_FILE_SIZE]      = bc_file_size,
    [BC_FILE_TELL]      = bc_file_tell,
    [BC_FILE_WRITE]     = bc_file_write,
    [BC_FILE_EOF]       = bc_file_eof,
    [BC_FILE_INPUTs]    = bc_file_inputs_s,

    [BC_STMT_CHDIR]      = bc_stmt_chdir,
    [BC_STMT_CLEAR]      = bc_stmt_clear,
    [BC_STMT_CLS]        = bc_stmt_cls,
    [BC_STMT_COLOR]      = bc_stmt_color,
    [BC_STMT_ERROR]      = bc_stmt_error,
    [BC_STMT_INPUT]      = bc_stmt_input,
    [BC_STMT_KILL]       = bc_stmt_kill,
    [BC_STMT_LINE_INPUT] = bc_stmt_line_input,
    [BC_STMT_LOCATE]     = bc_stmt_locate,
    [BC_STMT_MKDIR]      = bc_stmt_mkdir,
    [BC_STMT_RANDOMIZE]  = bc_stmt_randomize,
    [BC_STMT_RESUME]     = bc_stmt_resume,
    [BC_STMT_RMDIR]      = bc_stmt_rmdir,
    [BC_STMT_TIMER]      = bc_stmt_timer,
    [BC_STMT_WIDTH]      = bc_stmt_width,

    // Function tokens
    [BC_FILE_OPEN]    = bc_func_open,
    [BC_FUNC_ABS]     = bc_func_abs,
    [BC_FUNC_ASC]     = bc_func_asc,
    [BC_FUNC_ATN]     = bc_func_atn,
    [BC_FUNC_CDBL]    = bc_func_cdbl,
    [BC_FUNC_CHRs]    = bc_func_chr_s, // CHR$
    [BC_FUNC_CINT]    = bc_func_cint,
    [BC_FUNC_CLNG]    = bc_func_clng,
    [BC_FUNC_COS]     = bc_func_cos,
    [BC_FUNC_CSNG]    = bc_func_csng,
    [BC_FUNC_CSRLIN]  = bc_func_csrlin,
    [BC_FUNC_CVD]     = bc_func_cvd,
    [BC_FUNC_CVI]     = bc_func_cvi,
    [BC_FUNC_CVL]     = bc_func_cvl,
    [BC_FUNC_CVS]     = bc_func_cvs,
    [BC_FUNC_ERL]     = bc_func_erl,
    [BC_FUNC_ERR]     = bc_func_err,
    [BC_FUNC_EXP]     = bc_func_exp,
    [BC_FUNC_FIX]     = bc_func_fix,
    [BC_FUNC_HEXs]    = bc_func_hex_s,   // HEX$
    [BC_FUNC_INKEYs]  = bc_func_inkey_s, // INKEY$
    [BC_FUNC_INSTR]   = bc_func_instr,
    [BC_FUNC_INT]     = bc_func_int,
    [BC_FUNC_LCASEs]  = bc_func_lcase_s, // LCASE$
    [BC_FUNC_LEFTs]   = bc_func_left_s,  // LEFT$
    [BC_FUNC_LEN]     = bc_func_len,
    [BC_FUNC_LOG]     = bc_func_log,
    [BC_FUNC_LTRIMs]  = bc_func_ltrim_s, // LTRIM$
    [BC_FUNC_MIDs]    = bc_func_mid_s,   // MID$
    [BC_FUNC_MKDs]    = bc_func_mkd_s,   // MKD$
    [BC_FUNC_MKIs]    = bc_func_mki_s,   // MKI$
    [BC_FUNC_MKLs]    = bc_func_mkl_s,   // MKL$
    [BC_FUNC_MKSs]    = bc_func_mks_s,   // MKS$
    [BC_FUNC_OCTs]    = bc_func_oct_s,   // OCT$
    [BC_FUNC_POS]     = bc_func_pos,
    [BC_FUNC_RIGHTs]  = bc_func_right_s, // RIGHT$
    [BC_FUNC_RND]     = bc_func_rnd,
    [BC_FUNC_RTRIMs]  = bc_func_rtrim_s, // RTRIM$
    [BC_FUNC_SGN]     = bc_func_sgn,
    [BC_FUNC_SIN]     = bc_func_sin,
    [BC_FUNC_SPACEs]  = bc_func_space_s, // SPACE$
    [BC_FUNC_SQR]     = bc_func_sqr,
    [BC_FUNC_STRINGs] = bc_func_string_s, // STRING$
    [BC_FUNC_STRs]    = bc_func_str_s,    // STR$
    [BC_FUNC_TAN]     = bc_func_tan,
    [BC_FUNC_TIMER]   = bc_func_timer,
    [BC_FUNC_UCASEs]  = bc_func_ucase_s, // UCASE$
    [BC_FUNC_VAL]     = bc_func_val,
};

#ifndef PCDEV
void console_ctrl_c_pressed(void) {
    bc_state.stop = true;
}
#endif

void bytecode_run(const uint8_t *p_buf, size_t bc_size, size_t vars_sz) {
    buf_reinit();
    memset(&bc_state, 0, sizeof(bc_state));

    bc_state.p_vars = buf_calloc(vars_sz);
    if (!bc_state.p_vars)
        _basic_error(ERR_OUT_OF_MEM);

    bc_state.p_buf     = p_buf;
    bc_state.p_buf_end = p_buf + bc_size;
    bc_state.p_cur     = p_buf;
    bc_state.stack_idx = STACK_DEPTH;

    while (!bc_state.stop) {
#ifndef PCDEV
        int key = KEYBUF;
        if (key >= 0)
            _console_handle_key(key);
#endif
        uint8_t bc = bc_get_u8();
        bc_handlers[bc]();
    }
}

int bc_arg_size(uint8_t bc, const uint8_t *p) {
    switch (bc) {
        case BC_PUSH_CONST_INT: return 2; break;
        case BC_PUSH_CONST_LONG: return 4; break;
        case BC_PUSH_CONST_SINGLE: return 4; break;
        case BC_PUSH_CONST_DOUBLE: return 8; break;
        case BC_PUSH_CONST_STRING: return 1 + p[0]; break;

        case BC_JMP:
        case BC_JMP_NZ:
        case BC_JMP_Z:
        case BC_JSR:
        case BC_LINE_TAG:
        case BC_PUSH_VAR_INT:
        case BC_PUSH_VAR_LONG:
        case BC_PUSH_VAR_SINGLE:
        case BC_PUSH_VAR_DOUBLE:
        case BC_PUSH_VAR_STRING:
        case BC_STORE_VAR_INT:
        case BC_STORE_VAR_LONG:
        case BC_STORE_VAR_SINGLE:
        case BC_STORE_VAR_DOUBLE:
        case BC_STORE_VAR_STRING:
            return 2;

        default:
            return 0;
    }
}

int bytecode_get_line_for_offset(const uint8_t *buf, size_t buf_size, uint16_t offset) {
    const uint8_t *p        = buf;
    const uint8_t *p_end    = buf + buf_size;
    const uint8_t *p_offset = p + offset;
    uint16_t       linenr   = 0;

    while (p < p_end && p < p_offset) {
        uint8_t bc = *(p++);
        if (bc == BC_LINE_TAG)
            linenr = read_u16(p);

        p += bc_arg_size(bc, p);
    }
    return linenr;
}
