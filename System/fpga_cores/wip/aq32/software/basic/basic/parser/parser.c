#include "parser.h"
#include "tokenizer.h"
#include "basic.h"
#include "reloc.h"
#include "bytecode/bytecode.h"
#include "common/buffers.h"
#include "ctype2.h"

static uint8_t  default_type[26];
static bool     do_emit_line_tag = false;
static bool     emit_enabled;
static uint8_t *ptr_bytecode_buf_end;

static void bc_emit_u16(uint16_t val);
static void bc_emit_expr(void);
static void parse_statements(void);

static void bc_emit_disable(void) { emit_enabled = false; }
static void bc_emit_enable(void) { emit_enabled = true; }

static void expect(uint8_t tok) {
    if (get_token() != tok)
        _basic_error(ERR_SYNTAX_ERROR);
    ack_token();
}

static void bc_emit(uint8_t val) {
    if (!emit_enabled)
        return;

    if (do_emit_line_tag) {
        do_emit_line_tag = false;
        bc_emit(BC_LINE_TAG);
        bc_emit_u16(tokenizer_get_cur_line());
    }

    if (buf_bytecode_end >= ptr_bytecode_buf_end)
        _basic_error(ERR_OUT_OF_MEM);
    *(buf_bytecode_end++) = val;
}

static void bc_emit_u16(uint16_t val) {
    bc_emit(val & 0xFF);
    bc_emit((val >> 8) & 0xFF);
}

static void bc_emit_u32(uint32_t val) {
    bc_emit(val & 0xFF);
    bc_emit((val >> 8) & 0xFF);
    bc_emit((val >> 16) & 0xFF);
    bc_emit((val >> 24) & 0xFF);
}

static void bc_emit_u64(uint64_t val) {
    bc_emit(val & 0xFF);
    bc_emit((val >> 8) & 0xFF);
    bc_emit((val >> 16) & 0xFF);
    bc_emit((val >> 24) & 0xFF);
    bc_emit((val >> 32) & 0xFF);
    bc_emit((val >> 40) & 0xFF);
    bc_emit((val >> 48) & 0xFF);
    bc_emit((val >> 56) & 0xFF);
}

static void bc_emit_store_var(uint8_t type, uint16_t var_offset) {
    switch (type) {
        case '%': bc_emit(BC_STORE_VAR_INT); break;
        case '&': bc_emit(BC_STORE_VAR_LONG); break;
        case '!': bc_emit(BC_STORE_VAR_SINGLE); break;
        case '#': bc_emit(BC_STORE_VAR_DOUBLE); break;
        case '$': bc_emit(BC_STORE_VAR_STRING); break;
        default: _basic_error(ERR_INTERNAL_ERROR); break;
    }
    bc_emit_u16(var_offset);
}

static void bc_emit_push_var(uint8_t type, uint16_t var_offset) {
    switch (type) {
        case '%': bc_emit(BC_PUSH_VAR_INT); break;
        case '&': bc_emit(BC_PUSH_VAR_LONG); break;
        case '!': bc_emit(BC_PUSH_VAR_SINGLE); break;
        case '#': bc_emit(BC_PUSH_VAR_DOUBLE); break;
        case '$': bc_emit(BC_PUSH_VAR_STRING); break;
        default: _basic_error(ERR_INTERNAL_ERROR); break;
    }
    bc_emit_u16(var_offset);
}

static void bc_emit_store_array(uint8_t type, uint8_t num_dimensions, uint16_t var_offset) {
    switch (type) {
        case '%': bc_emit(BC_STORE_ARRAY_INT); break;
        case '&': bc_emit(BC_STORE_ARRAY_LONG); break;
        case '!': bc_emit(BC_STORE_ARRAY_SINGLE); break;
        case '#': bc_emit(BC_STORE_ARRAY_DOUBLE); break;
        case '$': bc_emit(BC_STORE_ARRAY_STRING); break;
        default: _basic_error(ERR_INTERNAL_ERROR); break;
    }
    bc_emit(num_dimensions);
    bc_emit_u16(var_offset);
}

static void bc_emit_push_array(uint8_t type, uint8_t num_dimensions, uint16_t var_offset) {
    switch (type) {
        case '%': bc_emit(BC_PUSH_ARRAY_INT); break;
        case '&': bc_emit(BC_PUSH_ARRAY_LONG); break;
        case '!': bc_emit(BC_PUSH_ARRAY_SINGLE); break;
        case '#': bc_emit(BC_PUSH_ARRAY_DOUBLE); break;
        case '$': bc_emit(BC_PUSH_ARRAY_STRING); break;
        default: _basic_error(ERR_INTERNAL_ERROR); break;
    }
    bc_emit(num_dimensions);
    bc_emit_u16(var_offset);
}

static void bc_emit_dim_array(uint8_t type, uint8_t num_dimensions, uint16_t var_offset) {
    switch (type) {
        case '%': bc_emit(BC_DIM_ARRAY_INT); break;
        case '&': bc_emit(BC_DIM_ARRAY_LONG); break;
        case '!': bc_emit(BC_DIM_ARRAY_SINGLE); break;
        case '#': bc_emit(BC_DIM_ARRAY_DOUBLE); break;
        case '$': bc_emit(BC_DIM_ARRAY_STRING); break;
        default: _basic_error(ERR_INTERNAL_ERROR); break;
    }
    bc_emit(num_dimensions);
    bc_emit_u16(var_offset);
}

static void bc_emit_push_const_int(int16_t val) {
    bc_emit(BC_PUSH_CONST_INT);
    bc_emit_u16(val);
}

static void bc_emit_push_const_long(int32_t val) {
    bc_emit(BC_PUSH_CONST_LONG);
    bc_emit_u32(val);
}

static void bc_emit_push_const_single(float val) {
    bc_emit(BC_PUSH_CONST_SINGLE);
    bc_emit_u32(*(uint32_t *)&val);
}

static void bc_emit_push_const_double(double val) {
    bc_emit(BC_PUSH_CONST_DOUBLE);
    bc_emit_u64(*(uint64_t *)&val);
}

static void deftype(uint8_t type_ch) {
    ack_token();

    while (1) {
        expect(TOK_IDENTIFIER);
        if (tokval_strlen != 1)
            _basic_error(ERR_SYNTAX_ERROR);

        char from_ch = to_upper(tokval_str[0]);
        if (!is_upper(from_ch))
            _basic_error(ERR_SYNTAX_ERROR);

        char to_ch = from_ch;
        if (get_token() == TOK_MINUS) {
            ack_token();

            expect(TOK_IDENTIFIER);
            if (tokval_strlen != 1)
                _basic_error(ERR_SYNTAX_ERROR);

            to_ch = to_upper(tokval_str[0]);
            if (!is_upper(to_ch))
                _basic_error(ERR_SYNTAX_ERROR);
        }

        for (int i = from_ch - 'A'; i <= to_ch - 'A'; i++)
            default_type[i] = type_ch;

        if (get_token() != TOK_COMMA)
            break;
        ack_token();
    }
}

static void infer_identifier_type(void) {
    if (is_typechar(tokval_str[tokval_strlen - 1]))
        return;

    tokval_str[tokval_strlen++] = default_type[tokval_str[0] - 'A'];
    tokval_str[tokval_strlen]   = 0;
}

struct func {
    uint8_t bc;
    int     num_params;
    void (*emit_func)(void);
};

static void bc_emit_func_instr(void) {
    expect(TOK_LPAREN);

    bool has_start_offset = false;

    // First determine number of parameters
    struct tokenizer_state tok_state;
    tokenizer_save_state(&tok_state);
    bc_emit_disable();
    bc_emit_expr();
    expect(TOK_COMMA);
    bc_emit_expr();
    if (get_token() == TOK_COMMA) {
        has_start_offset = true;
    }
    bc_emit_enable();
    tokenizer_restore_state(&tok_state);

    // Now actually parse
    if (!has_start_offset) {
        bc_emit_push_const_int(1);
    } else {
        bc_emit_expr();
        expect(TOK_COMMA);
    }

    bc_emit_expr();
    expect(TOK_COMMA);
    bc_emit_expr();
    expect(TOK_RPAREN);
    bc_emit(BC_FUNC_INSTR);
}

static void bc_emit_func_mid_s(void) {
    expect(TOK_LPAREN);
    bc_emit_expr();
    expect(TOK_COMMA);
    bc_emit_expr();
    if (get_token() == TOK_COMMA) {
        ack_token();
        bc_emit_expr();
    } else {
        bc_emit_push_const_int(INT16_MAX);
    }
    expect(TOK_RPAREN);
    bc_emit(BC_FUNC_MIDs);
}

static void bc_emit_func_rnd(void) {
    if (get_token() == TOK_LPAREN) {
        ack_token();
        bc_emit_expr();
        expect(TOK_RPAREN);
    } else {
        bc_emit_push_const_int(1);
    }
    bc_emit(BC_FUNC_RND);
}

static void bc_emit_func_pos(void) {
    if (get_token() == TOK_LPAREN) {
        ack_token();
        bc_emit_disable();
        bc_emit_expr();
        bc_emit_enable();
        expect(TOK_RPAREN);
    }
    bc_emit(BC_FUNC_POS);
}

static void bc_emit_func_seek(void) {
    expect(TOK_LPAREN);

    // Parse optional '#'
    if (get_token() == TOK_HASH)
        ack_token();

    // File number
    bc_emit_expr();

    expect(TOK_RPAREN);
    bc_emit(BC_FILE_TELL);
}

static void bc_emit_func_lof(void) {
    expect(TOK_LPAREN);

    // Parse optional '#'
    if (get_token() == TOK_HASH)
        ack_token();

    // File number
    bc_emit_expr();

    expect(TOK_RPAREN);
    bc_emit(BC_FILE_SIZE);
}

static void bc_emit_func_eof(void) {
    expect(TOK_LPAREN);

    // Parse optional '#'
    if (get_token() == TOK_HASH)
        ack_token();

    // File number
    bc_emit_expr();

    expect(TOK_RPAREN);
    bc_emit(BC_FILE_EOF);
}

static void bc_emit_func_input_s(void) {
    expect(TOK_LPAREN);

    // Number of characters to read
    bc_emit_expr();

    expect(TOK_COMMA);

    // Parse optional '#'
    if (get_token() == TOK_HASH)
        ack_token();

    // File number
    bc_emit_expr();

    expect(TOK_RPAREN);
    bc_emit(BC_FILE_INPUTs);
}

// clang-format off
static const struct func funcs[TOK_FUNC_LAST - TOK_FUNC_FIRST + 1] = {
    [TOK_ABS        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_ABS,     .num_params = 1, .emit_func = NULL},
    [TOK_ASC        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_ASC,     .num_params = 1, .emit_func = NULL},
    [TOK_ATN        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_ATN,     .num_params = 1, .emit_func = NULL},
    [TOK_CDBL       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CDBL,    .num_params = 1, .emit_func = NULL},
    [TOK_CHRs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CHRs,    .num_params = 1, .emit_func = NULL},
    [TOK_CINT       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CINT,    .num_params = 1, .emit_func = NULL},
    [TOK_CLNG       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CLNG,    .num_params = 1, .emit_func = NULL},
    [TOK_COS        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_COS,     .num_params = 1, .emit_func = NULL},
    [TOK_CSNG       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CSNG,    .num_params = 1, .emit_func = NULL},
    [TOK_CSRLIN     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CSRLIN,  .num_params = 0, .emit_func = NULL},
    [TOK_CVD        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CVD,     .num_params = 1, .emit_func = NULL},
    [TOK_CVI        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CVI,     .num_params = 1, .emit_func = NULL},
    [TOK_CVL        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CVL,     .num_params = 1, .emit_func = NULL},
    [TOK_CVS        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_CVS,     .num_params = 1, .emit_func = NULL},
    [TOK_EOF        - TOK_FUNC_FIRST] = {.bc = BC_FILE_EOF,     .num_params = 0, .emit_func = bc_emit_func_eof},
    [TOK_ERL        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_ERL,     .num_params = 0, .emit_func = NULL},
    [TOK_ERR        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_ERR,     .num_params = 0, .emit_func = NULL},
    [TOK_EXP        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_EXP,     .num_params = 1, .emit_func = NULL},
    [TOK_FIX        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_FIX,     .num_params = 1, .emit_func = NULL},
    [TOK_HEXs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_HEXs,    .num_params = 1, .emit_func = NULL},
    [TOK_INKEYs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_INKEYs,  .num_params = 0, .emit_func = NULL},
    [TOK_INPUTs     - TOK_FUNC_FIRST] = {.bc = 0,               .num_params = 0, .emit_func = bc_emit_func_input_s},
    [TOK_INSTR      - TOK_FUNC_FIRST] = {.bc = 0,               .num_params = 0, .emit_func = bc_emit_func_instr},
    [TOK_INT        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_INT,     .num_params = 1, .emit_func = NULL},
    [TOK_LCASEs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_LCASEs,  .num_params = 1, .emit_func = NULL},
    [TOK_LEFTs      - TOK_FUNC_FIRST] = {.bc = BC_FUNC_LEFTs,   .num_params = 2, .emit_func = NULL},
    [TOK_LEN        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_LEN,     .num_params = 1, .emit_func = NULL},
    [TOK_LOF        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_LOG,     .num_params = 0, .emit_func = bc_emit_func_lof},
    [TOK_LOG        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_LOG,     .num_params = 1, .emit_func = NULL},
    [TOK_LTRIMs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_LTRIMs,  .num_params = 1, .emit_func = NULL},
    [TOK_MIDs       - TOK_FUNC_FIRST] = {.bc = 0,               .num_params = 0, .emit_func = bc_emit_func_mid_s},
    [TOK_MKDs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_MKDs,    .num_params = 1, .emit_func = NULL},
    [TOK_MKIs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_MKIs,    .num_params = 1, .emit_func = NULL},
    [TOK_MKLs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_MKLs,    .num_params = 1, .emit_func = NULL},
    [TOK_MKSs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_MKSs,    .num_params = 1, .emit_func = NULL},
    [TOK_OCTs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_OCTs,    .num_params = 1, .emit_func = NULL},
    [TOK_POS        - TOK_FUNC_FIRST] = {.bc = 0,               .num_params = 0, .emit_func = bc_emit_func_pos},
    [TOK_RIGHTs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_RIGHTs,  .num_params = 2, .emit_func = NULL},
    [TOK_RND        - TOK_FUNC_FIRST] = {.bc = 0,               .num_params = 0, .emit_func = bc_emit_func_rnd},
    [TOK_RTRIMs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_RTRIMs,  .num_params = 1, .emit_func = NULL},
    [TOK_SEEK       - TOK_FUNC_FIRST] = {.bc = 0,               .num_params = 0, .emit_func = bc_emit_func_seek},
    [TOK_SGN        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_SGN,     .num_params = 1, .emit_func = NULL},
    [TOK_SIN        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_SIN,     .num_params = 1, .emit_func = NULL},
    [TOK_SPACEs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_SPACEs,  .num_params = 1, .emit_func = NULL},
    [TOK_SQR        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_SQR,     .num_params = 1, .emit_func = NULL},
    [TOK_STRINGs    - TOK_FUNC_FIRST] = {.bc = BC_FUNC_STRINGs, .num_params = 2, .emit_func = NULL},
    [TOK_STRs       - TOK_FUNC_FIRST] = {.bc = BC_FUNC_STRs,    .num_params = 1, .emit_func = NULL},
    [TOK_TAN        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_TAN,     .num_params = 1, .emit_func = NULL},
    [TOK_TIMER      - TOK_FUNC_FIRST] = {.bc = BC_FUNC_TIMER,   .num_params = 0, .emit_func = NULL},
    [TOK_UCASEs     - TOK_FUNC_FIRST] = {.bc = BC_FUNC_UCASEs,  .num_params = 1, .emit_func = NULL},
    [TOK_VAL        - TOK_FUNC_FIRST] = {.bc = BC_FUNC_VAL,     .num_params = 1, .emit_func = NULL},
};
// clang-format on

static bool bc_emit_base_type(void) {
    uint8_t tok = get_token();
    switch (tok) {
        case TOK_CONST_INT: {
            ack_token();
            bc_emit_push_const_int(tokval_num.val_long);
            return true;
        }
        case TOK_CONST_LONG: {
            ack_token();
            bc_emit_push_const_long(tokval_num.val_long);
            return true;
        }
        case TOK_CONST_SINGLE: {
            ack_token();
            bc_emit_push_const_single(tokval_num.val_single);
            return true;
        }
        case TOK_CONST_DOUBLE: {
            ack_token();
            bc_emit_push_const_double(tokval_num.val_double);
            return true;
        }
        case TOK_CONST_STR: {
            ack_token();
            bc_emit(BC_PUSH_CONST_STRING);
            bc_emit(tokval_strlen);
            for (int i = 0; i < tokval_strlen; i++)
                bc_emit(tokval_str[i]);
            return true;
        }
    }
    return false;
}

static void bc_emit_expr0(void) {
    if (bc_emit_base_type())
        return;

    uint8_t tok = get_token();
    switch (tok) {
        case TOK_IDENTIFIER: {
            ack_token();
            infer_identifier_type();
            uint8_t var_type = tokval_str[tokval_strlen - 1];

            if (get_token() == TOK_LPAREN) {
                // Array element
                ack_token();
                tokval_str[tokval_strlen++] = '(';
                tokval_str[tokval_strlen]   = 0;
                uint16_t var_offset         = reloc_var_get(tokval_str, tokval_strlen);
                uint8_t  num_dimensions     = 0;

                while (1) {
                    bc_emit_expr();
                    num_dimensions++;
                    if (get_token() != TOK_COMMA || num_dimensions >= ARRAY_MAX_DIMENSIONS)
                        break;
                    expect(TOK_COMMA);
                }
                expect(TOK_RPAREN);

                bc_emit_push_array(var_type, num_dimensions, var_offset);

            } else {
                // Normal variable
                uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);
                bc_emit_push_var(var_type, var_offset);
            }
            break;
        }
        case TOK_LPAREN: {
            ack_token();
            bc_emit_expr();
            expect(TOK_RPAREN);
            break;
        }
        default: {
            if (tok >= TOK_FUNC_FIRST && tok <= TOK_FUNC_LAST) {
                ack_token();

                const struct func *func = &funcs[tok - TOK_FUNC_FIRST];

                if (func->emit_func) {
                    func->emit_func();
                } else {
                    if (func->num_params > 0) {
                        expect(TOK_LPAREN);
                        for (int i = 0; i < func->num_params; i++) {
                            bc_emit_expr();
                            if (i != func->num_params - 1)
                                expect(TOK_COMMA);
                        }
                        expect(TOK_RPAREN);
                    }
                    bc_emit(func->bc);
                }
                break;
            }
            _basic_error(ERR_SYNTAX_ERROR);
            break;
        }
    }
}

struct op {
    uint8_t bc;
    uint8_t prec;
};

// clang-format off
static const struct op ops[TOK_OP_LAST - TOK_OP_FIRST + 1] = {
    [TOK_POW    - TOK_OP_FIRST] = {.bc = BC_OP_POW,    .prec = 13},
                             // = {.bc = BC_OP_NEGATE, .prec = 12},
    [TOK_MULT   - TOK_OP_FIRST] = {.bc = BC_OP_MULT,   .prec = 11},
    [TOK_DIV    - TOK_OP_FIRST] = {.bc = BC_OP_DIV,    .prec = 11},
    [TOK_INTDIV - TOK_OP_FIRST] = {.bc = BC_OP_INTDIV, .prec = 10},
    [TOK_MOD    - TOK_OP_FIRST] = {.bc = BC_OP_MOD,    .prec =  9},
    [TOK_PLUS   - TOK_OP_FIRST] = {.bc = BC_OP_ADD,    .prec =  8},
    [TOK_MINUS  - TOK_OP_FIRST] = {.bc = BC_OP_SUB,    .prec =  8},
    [TOK_EQ     - TOK_OP_FIRST] = {.bc = BC_OP_EQ,     .prec =  7},
    [TOK_NE     - TOK_OP_FIRST] = {.bc = BC_OP_NE,     .prec =  7},
    [TOK_LT     - TOK_OP_FIRST] = {.bc = BC_OP_LT,     .prec =  7},
    [TOK_LE     - TOK_OP_FIRST] = {.bc = BC_OP_LE,     .prec =  7},
    [TOK_GT     - TOK_OP_FIRST] = {.bc = BC_OP_GT,     .prec =  7},
    [TOK_GE     - TOK_OP_FIRST] = {.bc = BC_OP_GE,     .prec =  7},
    [TOK_NOT    - TOK_OP_FIRST] = {.bc = BC_OP_NOT,    .prec =  6},
    [TOK_AND    - TOK_OP_FIRST] = {.bc = BC_OP_AND,    .prec =  5},
    [TOK_OR     - TOK_OP_FIRST] = {.bc = BC_OP_OR,     .prec =  4},
    [TOK_XOR    - TOK_OP_FIRST] = {.bc = BC_OP_XOR,    .prec =  3},
    [TOK_EQV    - TOK_OP_FIRST] = {.bc = BC_OP_EQV,    .prec =  2},
    [TOK_IMP    - TOK_OP_FIRST] = {.bc = BC_OP_IMP,    .prec =  1},
};
// clang-format on

static void bc_emit_expr_prec(uint8_t min_prec) {
    uint8_t tok_op = get_token();

    // Special handling of unary identity/negation
    if (min_prec <= 12 && (tok_op == TOK_PLUS || tok_op == TOK_MINUS)) {
        ack_token();
        bc_emit_expr_prec(12);

        if (tok_op == TOK_MINUS)
            bc_emit(BC_OP_NEGATE);
    }

    // Special handling of not operator
    else if (tok_op == TOK_NOT && min_prec <= ops[tok_op - TOK_OP_FIRST].prec) {
        const struct op *op = &ops[tok_op - TOK_OP_FIRST];
        ack_token();
        bc_emit_expr_prec(op->prec);
        bc_emit(op->bc);
    }

    // Binary operation
    else {
        bc_emit_expr0();
    }

    while (1) {
        tok_op = get_token();
        if (tok_op < TOK_OP_FIRST || tok_op > TOK_OP_LAST || tok_op == TOK_NOT)
            break;

        const struct op *op = &ops[tok_op - TOK_OP_FIRST];
        if (op->prec < min_prec)
            break;

        ack_token();
        bc_emit_expr_prec(op->prec + 1);
        bc_emit(op->bc);
    }
}

static void bc_emit_expr(void) { bc_emit_expr_prec(0); }

static void bc_emit_stmt_print(void) {
    // Print to file?
    if (get_token() == TOK_HASH) {
        ack_token();
        bc_emit_expr();
        expect(TOK_COMMA);
        bc_emit(BC_SET_FILE);
    } else {
        bc_emit(BC_UNSET_FILE);
    }

    bool newline = true;
    while (1) {
        uint8_t tok = get_token();
        if (tok == TOK_EOL || tok == TOK_COLON || tok == TOK_ELSE)
            break;

        if (tok == TOK_SEMICOLON) {
            ack_token();
            newline = false;

        } else if (tok == TOK_COMMA) {
            ack_token();
            bc_emit(BC_PRINT_NEXT_FIELD);

        } else if (tok == TOK_SPC) {
            ack_token();
            expect(TOK_LPAREN);
            bc_emit_expr();
            expect(TOK_RPAREN);
            bc_emit(BC_PRINT_SPC);

        } else if (tok == TOK_TAB) {
            ack_token();
            expect(TOK_LPAREN);
            bc_emit_expr();
            expect(TOK_RPAREN);
            bc_emit(BC_PRINT_TAB);

        } else {
            newline = true;
            bc_emit_expr();
            bc_emit(BC_PRINT_VAL);
        }
    }
    if (newline) {
        bc_emit(BC_PRINT_NEWLINE);
    }
}

static void _bc_emit_target(void) {
    uint8_t tok = get_token();
    ack_token();

    uint16_t offset;
    if (tok == TOK_CONST_INT || tok == TOK_CONST_LONG) {
        offset = reloc_linenr_get(tokval_num.val_long, buf_bytecode_get_cur_offset());
    } else if (tok == TOK_IDENTIFIER) {
        offset = reloc_label_get(tokval_str, tokval_strlen, buf_bytecode_get_cur_offset());
    } else {
        _basic_error(ERR_SYNTAX_ERROR);
    }
    bc_emit_u16(offset);
}

static void bc_emit_stmt_gosub(void) {
    bc_emit(BC_JSR);
    _bc_emit_target();
}

static void bc_emit_stmt_goto(void) {
    bc_emit(BC_JMP);
    _bc_emit_target();
}

static void bc_emit_stmt_if(void) {
    bc_emit_expr();

    if (get_token() == TOK_GOTO) {
        ack_token();
        bc_emit(BC_JMP_NZ);
        _bc_emit_target();

        if (get_token() == TOK_ELSE) {
            ack_token();
            parse_statements();
        }
        return;
    }

    expect(TOK_THEN);

    if (get_token() != TOK_EOL) {
        // Single line if/else

        bc_emit(BC_JMP_Z);
        uint16_t if_false_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

        parse_statements();

        // ELSE?
        if (get_token() != TOK_ELSE) {
            buf_bytecode_patch_u16(if_false_offset, buf_bytecode_get_cur_offset());
            return;
        }
        ack_token();

        bc_emit(BC_JMP);
        uint16_t else_skip_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);
        buf_bytecode_patch_u16(if_false_offset, buf_bytecode_get_cur_offset());
        parse_statements();
        buf_bytecode_patch_u16(else_skip_offset, buf_bytecode_get_cur_offset());
        return;
    }

    // Block if/elseif/else/endif
#define IF_FALSE_OFFSETS_MAX 64
    uint16_t if_done_offsets[IF_FALSE_OFFSETS_MAX];
    unsigned if_done_offsets_count = 0;

    uint16_t if_false_offset;

    while (1) {
        bc_emit(BC_JMP_Z);
        if_false_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

        // Keep parsing until we find a ELSE/ELSEIF/END IF
        while (1) {
            parse_statements();

            uint8_t tok = get_token();
            if (tok == TOK_ENDIF || tok == TOK_ELSE || tok == TOK_ELSEIF)
                break;

            if (tok == TOK_END_OF_FILE)
                _basic_error(ERR_BLOCK_IF_WITHOUT_ENDIF);

            expect(TOK_EOL);
            do_emit_line_tag = true;
        }

        if (get_token() != TOK_ELSEIF)
            break;

        bc_emit(BC_JMP);
        if (if_done_offsets_count >= IF_FALSE_OFFSETS_MAX)
            _basic_error(ERR_OUT_OF_MEM);
        if_done_offsets[if_done_offsets_count++] = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

        ack_token();
        buf_bytecode_patch_u16(if_false_offset, buf_bytecode_get_cur_offset());
        bc_emit_expr();
        expect(TOK_THEN);
    }

    if (get_token() == TOK_ELSE) {
        ack_token();

        bc_emit(BC_JMP);
        uint16_t else_skip_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);
        buf_bytecode_patch_u16(if_false_offset, buf_bytecode_get_cur_offset());

        // Keep parsing until we find a END IF
        while (1) {
            parse_statements();

            uint8_t tok = get_token();
            if (tok == TOK_ENDIF)
                break;

            if (tok == TOK_END_OF_FILE)
                _basic_error(ERR_BLOCK_IF_WITHOUT_ENDIF);

            expect(TOK_EOL);
            do_emit_line_tag = true;
        }
        ack_token();
        buf_bytecode_patch_u16(else_skip_offset, buf_bytecode_get_cur_offset());

    } else {
        expect(TOK_ENDIF);
    }

    for (unsigned i = 0; i < if_done_offsets_count; i++) {
        buf_bytecode_patch_u16(if_done_offsets[i], buf_bytecode_get_cur_offset());
    }
}

static void bc_emit_stmt_while(void) {
    uint16_t while_offset = buf_bytecode_get_cur_offset();
    bc_emit_expr();
    bc_emit(BC_JMP_Z);
    uint16_t if_false_offset = buf_bytecode_get_cur_offset();
    bc_emit_u16(0xFFFF);

    // Keep parsing until we find a matching wend
    while (1) {
        parse_statements();

        uint8_t tok = get_token();
        if (tok == TOK_WEND)
            break;

        if (tok == TOK_END_OF_FILE)
            _basic_error(ERR_WHILE_WITHOUT_WEND);

        expect(TOK_EOL);
        do_emit_line_tag = true;
    }

    ack_token();
    bc_emit(BC_JMP);
    bc_emit_u16(while_offset);
    buf_bytecode_patch_u16(if_false_offset, buf_bytecode_get_cur_offset());
}

static void bc_emit_stmt_for(void) {
    expect(TOK_IDENTIFIER);
    infer_identifier_type();
    uint8_t var_type = tokval_str[tokval_strlen - 1];
    if (var_type == '$')
        _basic_error(ERR_TYPE_MISMATCH);

    // Initial assignment
    uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);
    expect(TOK_EQ);
    bc_emit_expr();

    expect(TOK_TO);

    // First skip TO condition to check if there is a STEP keyword
    struct tokenizer_state tok_to_state;
    tokenizer_save_state(&tok_to_state);
    bc_emit_disable();
    bc_emit_expr();
    bc_emit_enable();
    bool has_step = (get_token() == TOK_STEP);

    uint16_t loop_offset     = 0;
    uint16_t jmp_done_offset = 0;

    if (!has_step) {
        tokenizer_restore_state(&tok_to_state);

        bc_emit(BC_JMP);
        uint16_t jmp_cond_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

        // Loop increment
        loop_offset = buf_bytecode_get_cur_offset();
        bc_emit_push_var(var_type, var_offset);
        bc_emit(BC_OP_INC);
        buf_bytecode_patch_u16(jmp_cond_offset, buf_bytecode_get_cur_offset());
        bc_emit(BC_DUP);
        bc_emit_store_var(var_type, var_offset);

        // Condition checking
        bc_emit_expr(); // To value
        bc_emit(BC_OP_LE);
        bc_emit(BC_JMP_Z);
        jmp_done_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

    } else {
        bc_emit_store_var(var_type, var_offset);

        expect(TOK_STEP);
        struct tokenizer_state tok_step_state;
        tokenizer_save_state(&tok_step_state);

        bc_emit_expr(); // Step value
        tokenizer_restore_state(&tok_step_state);
        bc_emit_push_var(var_type, var_offset);

        bc_emit(BC_JMP);
        uint16_t jmp_cond_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

        // Loop increment
        loop_offset = buf_bytecode_get_cur_offset();
        bc_emit_expr(); // Step value
        bc_emit(BC_DUP);
        bc_emit_push_var(var_type, var_offset);
        bc_emit(BC_OP_ADD);
        bc_emit(BC_DUP);
        bc_emit_store_var(var_type, var_offset);

        // Condition checking
        tokenizer_restore_state(&tok_to_state);
        buf_bytecode_patch_u16(jmp_cond_offset, buf_bytecode_get_cur_offset());
        bc_emit_expr();
        bc_emit(BC_OP_LE_GE);
        bc_emit(BC_JMP_Z);
        jmp_done_offset = buf_bytecode_get_cur_offset();
        bc_emit_u16(0xFFFF);

        // Skip STEP
        expect(TOK_STEP);
        bc_emit_disable();
        bc_emit_expr();
        bc_emit_enable();
    }

    // Keep parsing until we find a matching next
    while (1) {
        parse_statements();

        uint8_t tok = get_token();
        if (tok == TOK_NEXT)
            break;

        if (tok == TOK_END_OF_FILE)
            _basic_error(ERR_FOR_WITHOUT_NEXT);

        expect(TOK_EOL);
        do_emit_line_tag = true;
    }
    ack_token();
    bc_emit(BC_JMP);
    bc_emit_u16(loop_offset);
    buf_bytecode_patch_u16(jmp_done_offset, buf_bytecode_get_cur_offset());
}

static void bc_emit_stmt_data(void) {
    bc_emit(BC_DATA);
    uint16_t data_size_offset = buf_bytecode_get_cur_offset();
    bc_emit_u16(0xFFFF);

    while (1) {
        uint8_t tok = get_token();
        if (tok == TOK_COLON || tok == TOK_EOL)
            break;

        if (!bc_emit_base_type())
            _basic_error(ERR_SYNTAX_ERROR);

        tok = get_token();
        if (tok != TOK_COMMA)
            break;
        ack_token();
    }

    buf_bytecode_patch_u16(data_size_offset, buf_bytecode_get_cur_offset());
}

static void bc_emit_stmt_read(void) {
    // Read fromfile?
    bool is_file = false;
    if (get_token() == TOK_HASH) {
        ack_token();
        bc_emit_expr();
        expect(TOK_COMMA);
        bc_emit(BC_SET_FILE);
        is_file = true;
    }

    while (1) {
        expect(TOK_IDENTIFIER);
        infer_identifier_type();
        uint8_t  var_type   = tokval_str[tokval_strlen - 1];
        uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);

        if (is_file) {
            bc_emit(BC_FILE_READ);
            bc_emit(var_type);
            bc_emit_u16(var_offset);

        } else {
            bc_emit(BC_DATA_READ);
            bc_emit_store_var(var_type, var_offset);
        }

        if (get_token() != TOK_COMMA)
            break;
        ack_token();
    }
}

static void bc_emit_stmt_restore(void) {
    bc_emit(BC_DATA_RESTORE);
    _bc_emit_target();
}

static void bc_emit_stmt_swap(void) {
    expect(TOK_IDENTIFIER);
    infer_identifier_type();
    uint8_t  var1_type   = tokval_str[tokval_strlen - 1];
    uint16_t var1_offset = reloc_var_get(tokval_str, tokval_strlen);

    expect(TOK_COMMA);
    expect(TOK_IDENTIFIER);
    infer_identifier_type();
    uint8_t  var2_type   = tokval_str[tokval_strlen - 1];
    uint16_t var2_offset = reloc_var_get(tokval_str, tokval_strlen);

    if (var1_type != var2_type)
        _basic_error(ERR_TYPE_MISMATCH);

    bc_emit_push_var(var1_type, var1_offset);
    bc_emit_push_var(var2_type, var2_offset);
    bc_emit_store_var(var1_type, var1_offset);
    bc_emit_store_var(var2_type, var2_offset);
}

static void bc_emit_stmt_color(void) {
    if (get_token() != TOK_COMMA) {
        bc_emit_expr();
    } else {
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
    }

    if (get_token() == TOK_COMMA) {
        ack_token();
        bc_emit_expr();
    } else {
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
    }

    bc_emit(BC_STMT_COLOR);
}

static void bc_emit_stmt_locate(void) {
    if (get_token() != TOK_COMMA) {
        bc_emit_expr();
    } else {
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
    }

    if (get_token() == TOK_COMMA) {
        ack_token();
    } else {
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
        bc_emit(BC_STMT_LOCATE);
        return;
    }

    if (get_token() != TOK_COMMA) {
        bc_emit_expr();
    } else {
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
    }

    if (get_token() == TOK_COMMA) {
        ack_token();
    } else {
        bc_emit(BC_PUSH_CONST_UNSPECIFIED_PARAM);
        bc_emit(BC_STMT_LOCATE);
        return;
    }

    bc_emit_expr();
    bc_emit(BC_STMT_LOCATE);
}

void bc_emit_stmt_return(void) {
    uint8_t tok = get_token();
    if (tok == TOK_COLON || tok == TOK_EOL) {
        bc_emit(BC_RETURN);
    } else {
        bc_emit(BC_DROP);
        bc_emit(BC_JMP);
        _bc_emit_target();
    }
}

void bc_emit_stmt_dim(void) {
    expect(TOK_IDENTIFIER);
    infer_identifier_type();
    uint8_t var_type = tokval_str[tokval_strlen - 1];
    expect(TOK_LPAREN);
    tokval_str[tokval_strlen++] = '(';
    tokval_str[tokval_strlen]   = 0;
    uint16_t var_offset         = reloc_var_get(tokval_str, tokval_strlen);
    uint8_t  num_dimensions     = 0;

    while (1) {
        bc_emit_expr();
        num_dimensions++;
        if (get_token() != TOK_COMMA || num_dimensions >= ARRAY_MAX_DIMENSIONS)
            break;
        expect(TOK_COMMA);
    }
    expect(TOK_RPAREN);
    bc_emit_dim_array(var_type, num_dimensions, var_offset);
}

void bc_emit_erase(void) {
    while (1) {
        expect(TOK_IDENTIFIER);
        infer_identifier_type();
        tokval_str[tokval_strlen++] = '(';
        tokval_str[tokval_strlen]   = 0;
        uint16_t var_offset         = reloc_var_get(tokval_str, tokval_strlen);
        bc_emit(BC_FREE_ARRAY);
        bc_emit_u16(var_offset);

        if (get_token() != TOK_COMMA)
            break;
        ack_token();
    }
}

static void bc_emit_stmt_input(void) {
    if (get_token() == TOK_HASH) {
        ack_token();

        // File number
        bc_emit_expr();
        bc_emit(BC_SET_FILE);
        expect(TOK_COMMA);

        bc_emit(BC_STMT_INPUT);
        bc_emit(0);
        bc_emit(0);

    } else {
        uint8_t flags = 1; // bit1:stay on line after enter, bit0:show questionmark
        bc_emit(BC_UNSET_FILE);
        bc_emit(BC_STMT_INPUT);

        if (get_token() == TOK_SEMICOLON) {
            ack_token();
            flags |= 2;
        }

        if (get_token() == TOK_CONST_STR) {
            ack_token();
            bc_emit(tokval_strlen);
            for (int i = 0; i < tokval_strlen; i++)
                bc_emit(tokval_str[i]);

            if (get_token() == TOK_COMMA) {
                ack_token();
                flags &= ~1;
            } else {
                expect(TOK_SEMICOLON);
            }
        } else {
            // Empty string literal
            bc_emit(0);
        }
        bc_emit(flags);
    }

    while (1) {
        expect(TOK_IDENTIFIER);
        infer_identifier_type();
        uint8_t  var_type   = tokval_str[tokval_strlen - 1];
        uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);

        bc_emit(var_type);
        bc_emit_u16(var_offset);

        if (get_token() != TOK_COMMA)
            break;
        ack_token();
    }
    bc_emit(0);
}

static void bc_emit_stmt_line_input(void) {
    if (get_token() == TOK_HASH) {
        ack_token();
        bc_emit_expr();
        expect(TOK_COMMA);

        expect(TOK_IDENTIFIER);
        infer_identifier_type();
        uint8_t  var_type   = tokval_str[tokval_strlen - 1];
        uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);
        if (var_type != '$')
            _basic_error(ERR_TYPE_MISMATCH);

        bc_emit(BC_FILE_READLINE);
        bc_emit_u16(var_offset);

    } else {
        bc_emit(BC_STMT_LINE_INPUT);

        uint8_t flags = 0;
        if (get_token() == TOK_SEMICOLON) {
            ack_token();
            flags |= 2;
        }

        if (get_token() == TOK_CONST_STR) {
            ack_token();
            bc_emit(tokval_strlen);
            for (int i = 0; i < tokval_strlen; i++)
                bc_emit(tokval_str[i]);
            expect(TOK_SEMICOLON);
        } else {
            // Empty string literal
            bc_emit(0);
        }

        bc_emit(flags);

        expect(TOK_IDENTIFIER);
        infer_identifier_type();
        uint8_t  var_type   = tokval_str[tokval_strlen - 1];
        uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);
        if (var_type != '$')
            _basic_error(ERR_TYPE_MISMATCH);

        bc_emit_u16(var_offset);
    }
}

static void bc_emit_stmt_open(void) {
    bc_emit_expr();
    expect(TOK_FOR);

    if (get_token() == TOK_INPUT) {
        ack_token();
        bc_emit_push_const_int(OPEN_MODE_INPUT);
    } else {
        expect(TOK_IDENTIFIER);
        if (strcmp(tokval_str, "OUTPUT") == 0) {
            bc_emit_push_const_int(OPEN_MODE_OUTPUT);
        } else if (strcmp(tokval_str, "RANDOM") == 0) {
            bc_emit_push_const_int(OPEN_MODE_RANDOM);
        } else if (strcmp(tokval_str, "APPEND") == 0) {
            bc_emit_push_const_int(OPEN_MODE_APPEND);
        } else {
            _basic_error(ERR_SYNTAX_ERROR);
        }
    }

    expect(TOK_IDENTIFIER);
    if (strcmp(tokval_str, "AS") != 0)
        _basic_error(ERR_SYNTAX_ERROR);

    bc_emit(BC_FILE_OPEN);

    expect(TOK_IDENTIFIER);
    infer_identifier_type();
    uint8_t  var_type   = tokval_str[tokval_strlen - 1];
    uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);
    bc_emit_store_var(var_type, var_offset);
}

static void bc_emit_stmt_close(void) {
    uint8_t tok = get_token();
    if (tok == TOK_COLON || tok == TOK_EOL) {
        bc_emit(BC_FILE_CLOSE_ALL);
        return;
    }

    while (1) {
        // Parse optional '#'
        if (get_token() == TOK_HASH)
            ack_token();
        bc_emit_expr();
        bc_emit(BC_FILE_CLOSE);

        if (get_token() != TOK_COMMA)
            break;
        ack_token();
    }
}

static void bc_emit_stmt_write(void) {
    // Parse optional '#'
    if (get_token() == TOK_HASH)
        ack_token();

    // File number
    bc_emit_expr();
    bc_emit(BC_SET_FILE);
    expect(TOK_COMMA);

    while (1) {
        expect(TOK_IDENTIFIER);
        infer_identifier_type();
        uint8_t  var_type   = tokval_str[tokval_strlen - 1];
        uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);

        bc_emit(BC_FILE_WRITE);
        bc_emit(var_type);
        bc_emit_u16(var_offset);

        if (get_token() != TOK_COMMA)
            break;
        ack_token();
    }
}

static void bc_emit_stmt_seek(void) {
    // Parse optional '#'
    if (get_token() == TOK_HASH)
        ack_token();

    // File number
    bc_emit_expr();
    bc_emit(BC_SET_FILE);
    expect(TOK_COMMA);
    bc_emit_expr();
    bc_emit(BC_FILE_SEEK);
}

static void bc_emit_stmt_timer(void) {
    _basic_error(ERR_UNHANDLED);
}

struct stmt {
    uint8_t bc;
    int     num_params;
    void (*emit_stmt)(void);
};

// clang-format off
static const struct stmt stmts[TOK_STMT_LAST - TOK_STMT_FIRST + 1] = {
    [TOK_CHDIR      - TOK_STMT_FIRST] = {.bc = BC_STMT_CHDIR,     .num_params = 1, .emit_stmt = NULL},
    [TOK_CLEAR      - TOK_STMT_FIRST] = {.bc = BC_STMT_CLEAR,     .num_params = 0, .emit_stmt = NULL},
    [TOK_CLOSE      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_close},
    [TOK_CLS        - TOK_STMT_FIRST] = {.bc = BC_STMT_CLS,       .num_params = 0, .emit_stmt = NULL},
    [TOK_COLOR      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_color},
    [TOK_DATA       - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_data},
    [TOK_DIM        - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_dim},
    [TOK_END        - TOK_STMT_FIRST] = {.bc = BC_END,            .num_params = 0, .emit_stmt = NULL},
    [TOK_ERASE      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_erase},
    [TOK_ERROR      - TOK_STMT_FIRST] = {.bc = BC_STMT_ERROR,     .num_params = 1, .emit_stmt = NULL},
    [TOK_FOR        - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_for},
    [TOK_GOSUB      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_gosub},
    [TOK_GOTO       - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_goto},
    [TOK_IF         - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_if},
    [TOK_INPUT      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_input},
    [TOK_KILL       - TOK_STMT_FIRST] = {.bc = BC_STMT_KILL,      .num_params = 1, .emit_stmt = NULL},
    [TOK_LINE_INPUT - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_line_input},
    [TOK_LOCATE     - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_locate},
    [TOK_MKDIR      - TOK_STMT_FIRST] = {.bc = BC_STMT_MKDIR,     .num_params = 1, .emit_stmt = NULL},
    [TOK_OPEN       - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_open},
    [TOK_PRINT      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_print},
    [TOK_RANDOMIZE  - TOK_STMT_FIRST] = {.bc = BC_STMT_RANDOMIZE, .num_params = 1, .emit_stmt = NULL},
    [TOK_READ       - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_read},
    [TOK_RESTORE    - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_restore},
 // [TOK_RESUME     - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = NULL},
    [TOK_RETURN     - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_return},
    [TOK_RMDIR      - TOK_STMT_FIRST] = {.bc = BC_STMT_RMDIR,     .num_params = 1, .emit_stmt = NULL},
    [TOK_SWAP       - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_swap},
 // [TOK_TIMER      - TOK_STMT_FIRST] = {.bc = BC_STMT_TIMER,     .num_params = 0, .emit_stmt = NULL},
    [TOK_WHILE      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_while},
    [TOK_WIDTH      - TOK_STMT_FIRST] = {.bc = BC_STMT_WIDTH,     .num_params = 1, .emit_stmt = NULL},
    [TOK_WRITE      - TOK_STMT_FIRST] = {.bc = 0,                 .num_params = 0, .emit_stmt = bc_emit_stmt_write},
};
// clang-format on

static void parse_statement(void) {
    uint8_t tok = get_token();
    if (tok == TOK_EOL || tok == TOK_END_OF_FILE)
        return;

    // Default type keywords
    switch (tok) {
        case TOK_DEFINT: deftype('%'); return;
        case TOK_DEFLNG: deftype('&'); return;
        case TOK_DEFSNG: deftype('!'); return;
        case TOK_DEFDBL: deftype('#'); return;
        case TOK_DEFSTR: deftype('$'); return;
        case TOK_LINENR:
            reloc_linenr_add(tokval_num.val_long, buf_bytecode_get_cur_offset());
            ack_token();
            break;
        case TOK_LABEL:
            reloc_label_add(tokval_str, tokval_strlen, buf_bytecode_get_cur_offset());
            ack_token();
            break;
    }

    tok = get_token();
    if (tok == TOK_EOL || tok == TOK_END_OF_FILE)
        return;

    if (tok == TOK_LET || tok == TOK_IDENTIFIER) {
        if (tok == TOK_LET) {
            ack_token();
            expect(TOK_IDENTIFIER);
        }
        ack_token();
        infer_identifier_type();
        uint8_t var_type = tokval_str[tokval_strlen - 1];

        if (get_token() == TOK_LPAREN) {
            // Array element
            ack_token();

            struct tokenizer_state saved_state;
            tokenizer_save_state(&saved_state);

            tokval_str[tokval_strlen++] = '(';
            tokval_str[tokval_strlen]   = 0;
            uint16_t var_offset         = reloc_var_get(tokval_str, tokval_strlen);
            uint8_t  num_dimensions     = 0;

            bc_emit_disable();

            while (1) {
                bc_emit_expr();
                num_dimensions++;
                if (get_token() != TOK_COMMA || num_dimensions >= ARRAY_MAX_DIMENSIONS)
                    break;
                expect(TOK_COMMA);
            }
            expect(TOK_RPAREN);

            bc_emit_enable();

            expect(TOK_EQ);
            bc_emit_expr();

            tokenizer_restore_state(&saved_state);
            for (int i = 0; i < num_dimensions; i++) {
                bc_emit_expr();
                if (i < num_dimensions - 1)
                    expect(TOK_COMMA);
            }
            expect(TOK_RPAREN);
            bc_emit_store_array(var_type, num_dimensions, var_offset);

            bc_emit_disable();
            expect(TOK_EQ);
            bc_emit_expr();
            bc_emit_enable();

        } else {
            // Normal variable
            uint16_t var_offset = reloc_var_get(tokval_str, tokval_strlen);
            expect(TOK_EQ);

            bc_emit_expr();
            bc_emit_store_var(var_type, var_offset);
        }
        return;
    }

    if (tok >= TOK_STMT_FIRST && tok <= TOK_STMT_LAST) {
        ack_token();

        if (stmts[tok - TOK_STMT_FIRST].emit_stmt) {
            stmts[tok - TOK_STMT_FIRST].emit_stmt();

        } else if (stmts[tok - TOK_STMT_FIRST].bc == 0) {
            _basic_error(ERR_SYNTAX_ERROR);

        } else {
            int num = stmts[tok - TOK_STMT_FIRST].num_params;
            for (int i = 0; i < num; i++) {
                bc_emit_expr();
                if (i != num - 1)
                    expect(TOK_COMMA);
            }
            bc_emit(stmts[tok - TOK_STMT_FIRST].bc);
        }
        return;

    } else {
        switch (tok) {
            case TOK_SEEK: {
                ack_token();
                bc_emit_stmt_seek();
                break;
            }

            case TOK_TIMER: {
                ack_token();
                bc_emit_stmt_timer();
                break;
            }
        }
    }
}

static void parse_statements(void) {
    while (1) {
        parse_statement();
        if (get_token() != TOK_COLON)
            return;
        ack_token();
    }
}

void basic_parse(struct editbuf *eb) {
    buf_reinit();

    buf_bytecode_end     = buf_bytecode;
    ptr_bytecode_buf_end = buf_bytecode + sizeof(buf_bytecode);
    emit_enabled         = true;

    reloc_init();
    tokenizer_init(eb);
    for (int i = 0; i < 26; i++)
        default_type[i] = '!';

    while (1) {
        parse_statements();

        if (get_token() == TOK_END_OF_FILE)
            break;
        expect(TOK_EOL);
        do_emit_line_tag = true;
    }
    bc_emit(BC_END);

    reloc_process_relocations();
}
