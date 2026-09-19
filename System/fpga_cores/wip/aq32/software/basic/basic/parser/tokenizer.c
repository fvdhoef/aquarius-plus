#include "tokenizer.h"
#include "basic.h"
#include "common/parsenum.h"
#include "ctype2.h"

static struct editbuf *eb;

static struct tokenizer_state state;

static int          cur_token;
union tokval_number tokval_num;
char                tokval_str[MAX_STRLEN + 3];
int                 tokval_strlen;

struct keyword {
    const char *name;
    uint8_t     token;
};

// Keep this table sorted on name, since it is searched using a binary search
static const struct keyword keywords[] = {
    {.name = "ABS", .token = TOK_ABS},
    {.name = "AND", .token = TOK_AND},
    {.name = "ASC", .token = TOK_ASC},
    {.name = "ATN", .token = TOK_ATN},
    {.name = "CDBL", .token = TOK_CDBL},
    {.name = "CHDIR", .token = TOK_CHDIR},
    {.name = "CHR$", .token = TOK_CHRs},
    {.name = "CINT", .token = TOK_CINT},
    {.name = "CLEAR", .token = TOK_CLEAR},
    {.name = "CLNG", .token = TOK_CLNG},
    {.name = "CLOSE", .token = TOK_CLOSE},
    {.name = "CLS", .token = TOK_CLS},
    {.name = "COLOR", .token = TOK_COLOR},
    {.name = "COS", .token = TOK_COS},
    {.name = "CSNG", .token = TOK_CSNG},
    {.name = "CSRLIN", .token = TOK_CSRLIN},
    {.name = "CVD", .token = TOK_CVD},
    {.name = "CVI", .token = TOK_CVI},
    {.name = "CVL", .token = TOK_CVL},
    {.name = "CVS", .token = TOK_CVS},
    {.name = "DATA", .token = TOK_DATA},
    {.name = "DEFDBL", .token = TOK_DEFDBL},
    {.name = "DEFINT", .token = TOK_DEFINT},
    {.name = "DEFLNG", .token = TOK_DEFLNG},
    {.name = "DEFSNG", .token = TOK_DEFSNG},
    {.name = "DEFSTR", .token = TOK_DEFSTR},
    {.name = "DIM", .token = TOK_DIM},
    {.name = "ELSE", .token = TOK_ELSE},
    {.name = "ELSEIF", .token = TOK_ELSEIF},
    {.name = "END", .token = TOK_END},
    {.name = "EOF", .token = TOK_EOF},
    {.name = "EQV", .token = TOK_EQV},
    {.name = "ERASE", .token = TOK_ERASE},
    {.name = "ERL", .token = TOK_ERL},
    {.name = "ERR", .token = TOK_ERR},
    {.name = "ERROR", .token = TOK_ERROR},
    {.name = "EXP", .token = TOK_EXP},
    {.name = "FIX", .token = TOK_FIX},
    {.name = "FN", .token = TOK_FN},
    {.name = "FOR", .token = TOK_FOR},
    {.name = "GOSUB", .token = TOK_GOSUB},
    {.name = "GOTO", .token = TOK_GOTO},
    {.name = "HEX$", .token = TOK_HEXs},
    {.name = "IF", .token = TOK_IF},
    {.name = "IMP", .token = TOK_IMP},
    {.name = "INKEY$", .token = TOK_INKEYs},
    {.name = "INPUT", .token = TOK_INPUT},
    {.name = "INPUT$", .token = TOK_INPUTs},
    {.name = "INSTR", .token = TOK_INSTR},
    {.name = "INT", .token = TOK_INT},
    {.name = "KILL", .token = TOK_KILL},
    {.name = "LCASE$", .token = TOK_LCASEs},
    {.name = "LEFT$", .token = TOK_LEFTs},
    {.name = "LEN", .token = TOK_LEN},
    {.name = "LET", .token = TOK_LET},
    {.name = "LINE", .token = TOK_LINE},
    {.name = "LOCATE", .token = TOK_LOCATE},
    {.name = "LOF", .token = TOK_LOF},
    {.name = "LOG", .token = TOK_LOG},
    {.name = "LTRIM$", .token = TOK_LTRIMs},
    {.name = "MID$", .token = TOK_MIDs},
    {.name = "MKD$", .token = TOK_MKDs},
    {.name = "MKDIR", .token = TOK_MKDIR},
    {.name = "MKI$", .token = TOK_MKIs},
    {.name = "MKL$", .token = TOK_MKLs},
    {.name = "MKS$", .token = TOK_MKSs},
    {.name = "MOD", .token = TOK_MOD},
    {.name = "NEXT", .token = TOK_NEXT},
    {.name = "NOT", .token = TOK_NOT},
    {.name = "OCT$", .token = TOK_OCTs},
    {.name = "OFF", .token = TOK_OFF},
    {.name = "ON", .token = TOK_ON},
    {.name = "OPEN", .token = TOK_OPEN},
    {.name = "OR", .token = TOK_OR},
    {.name = "POS", .token = TOK_POS},
    {.name = "PRINT", .token = TOK_PRINT},
    {.name = "RANDOMIZE", .token = TOK_RANDOMIZE},
    {.name = "READ", .token = TOK_READ},
    {.name = "REM", .token = TOK_REM},
    {.name = "RESTORE", .token = TOK_RESTORE},
    {.name = "RESUME", .token = TOK_RESUME},
    {.name = "RETURN", .token = TOK_RETURN},
    {.name = "RIGHT$", .token = TOK_RIGHTs},
    {.name = "RMDIR", .token = TOK_RMDIR},
    {.name = "RND", .token = TOK_RND},
    {.name = "RTRIM$", .token = TOK_RTRIMs},
    {.name = "SEEK", .token = TOK_SEEK},
    {.name = "SGN", .token = TOK_SGN},
    {.name = "SIN", .token = TOK_SIN},
    {.name = "SPACE$", .token = TOK_SPACEs},
    {.name = "SPC", .token = TOK_SPC},
    {.name = "SQR", .token = TOK_SQR},
    {.name = "STEP", .token = TOK_STEP},
    {.name = "STR$", .token = TOK_STRs},
    {.name = "STRING$", .token = TOK_STRINGs},
    {.name = "SWAP", .token = TOK_SWAP},
    {.name = "TAB", .token = TOK_TAB},
    {.name = "TAN", .token = TOK_TAN},
    {.name = "THEN", .token = TOK_THEN},
    {.name = "TIMER", .token = TOK_TIMER},
    {.name = "TO", .token = TOK_TO},
    {.name = "UCASE$", .token = TOK_UCASEs},
    {.name = "USING", .token = TOK_USING},
    {.name = "VAL", .token = TOK_VAL},
    {.name = "WEND", .token = TOK_WEND},
    {.name = "WHILE", .token = TOK_WHILE},
    {.name = "WIDTH", .token = TOK_WIDTH},
    {.name = "WRITE", .token = TOK_WRITE},
    {.name = "XOR", .token = TOK_XOR},
};

static void skip_whitespace(void) {
    while (state.p_cur < state.p_line_end) {
        if (state.p_cur[0] == '\'') {
            // Comment
            state.p_cur = state.p_line_end;
            return;
        }

        if (state.p_cur[0] == ' ' || state.p_cur[0] == '\t') {
            state.p_cur++;
            continue;
        }
        break;
    }
}

static void parse_linenr(void) {
    tokval_num.val_long = 0;
    while (state.p_cur < state.p_line_end && is_decimal(state.p_cur[0])) {
        tokval_num.val_long *= 10;
        tokval_num.val_long += (state.p_cur[0]) - '0';
        state.p_cur++;
    }
}

static bool parse_number(void) {
    char tmp[64];
    char type   = 0;
    int  result = copy_num_to_buf(&state.p_cur, state.p_line_end, tmp, sizeof(tmp), &type);
    if (result < 0) {
        return false;

    } else if (result > 0) {
        int base            = result;
        tokval_num.val_long = strtol(tmp, NULL, base);
        bool is_int16_range = (tokval_num.val_long >= INT16_MIN && tokval_num.val_long <= INT16_MAX);

        if (type == 0) {
            // Infer type
            type = is_int16_range ? '%' : '&';
        } else if (type == '%' && !is_int16_range) {
            _basic_error(ERR_TYPE_MISMATCH);
        }
        cur_token = (type == '%') ? TOK_CONST_INT : TOK_CONST_LONG;

    } else {
        if (type == '#') {
            cur_token             = TOK_CONST_DOUBLE;
            tokval_num.val_double = strtod(tmp, NULL);

        } else {
            cur_token             = TOK_CONST_SINGLE;
            tokval_num.val_single = strtof(tmp, NULL);
        }
    }
    return true;
}

static bool parse_identifier(bool allow_typechar) {
    tokval_strlen = 0;
    while (state.p_cur < state.p_line_end) {
        uint8_t ch = to_upper(state.p_cur[0]);
        if (!is_alpha(ch) && !is_decimal(ch) && ch != '.')
            break;
        if (tokval_strlen >= MAX_STRLEN)
            return -1;
        tokval_str[tokval_strlen++] = ch;
        state.p_cur++;
    }
    if (tokval_strlen == 0)
        return false;

    if (state.p_cur < state.p_line_end && allow_typechar && is_typechar(state.p_cur[0]))
        tokval_str[tokval_strlen++] = *(state.p_cur++);

    tokval_str[tokval_strlen] = 0;
    return true;
}

static int keyword_cmp(const void *_key, const void *_elem) {
    const char           *key  = _key;
    const struct keyword *elem = _elem;
    return strcmp(key, elem->name);
}

static int _get_token(void) {
    if (cur_token >= 0)
        return cur_token;

    // Get next line
    if (state.p_cur == state.p_line_end) {
        if (state.cur_line >= editbuf_get_line_count(eb)) {
            cur_token = TOK_END_OF_FILE;
            return cur_token;
        }

        state.cur_line++;

        int line_len = editbuf_get_line(eb, state.cur_line, &state.p_line);
        if (line_len < 0) {
            cur_token = TOK_END_OF_FILE;
            return cur_token;
        }
        // printf("%.*s\n", line_len, state.p_line);

        if (state.p_cur != NULL)
            cur_token = TOK_EOL;

        state.p_cur      = state.p_line;
        state.p_line_end = state.p_line + line_len;

        if (cur_token >= 0)
            return cur_token;
    }

    // Check for line number or label
    if (state.p_cur == state.p_line) {
        // Skip whitespace
        skip_whitespace();
        if (state.p_cur == state.p_line_end) {
            cur_token = TOK_EOL;
            return cur_token;
        }

        if (is_decimal(state.p_cur[0])) {
            // Linenr
            parse_linenr();
            cur_token = TOK_LINENR;
            return cur_token;

        } else {
            // Check for label
            state.p_cur = state.p_line;
            if (parse_identifier(false) && state.p_cur < state.p_line_end && state.p_cur[0] == ':') {
                state.p_cur++;
                cur_token = TOK_LABEL;
                return cur_token;
            }

            state.p_cur = state.p_line;
        }
    }

    // Skip whitespace
    skip_whitespace();
    if (state.p_cur == state.p_line_end) {
        cur_token = TOK_EOL;
        return cur_token;
    }

    // Number?
    if ((state.p_cur[0] == '-' ||
         state.p_cur[0] == '.' ||
         state.p_cur[0] == '&' ||
         is_decimal(state.p_cur[0])) &&
        parse_number())
        return cur_token;

    // String?
    if (state.p_cur[0] == '"') {
        state.p_cur++;

        cur_token     = TOK_CONST_STR;
        tokval_strlen = 0;
        while (state.p_cur < state.p_line_end && state.p_cur[0] != '"')
            tokval_str[tokval_strlen++] = *(state.p_cur++);
        tokval_str[tokval_strlen] = 0;

        // Expect "
        if (state.p_cur == state.p_line_end || state.p_cur[0] != '"')
            _basic_error(ERR_SYNTAX_ERROR);
        state.p_cur++;

        return cur_token;
    }

    // Simple token?
    switch (state.p_cur[0]) {
        case '<':
            state.p_cur++;
            if (state.p_cur < state.p_line_end && state.p_cur[0] == '>') {
                state.p_cur++;
                cur_token = TOK_NE;
                return cur_token;
            }
            if (state.p_cur < state.p_line_end && state.p_cur[0] == '=') {
                state.p_cur++;
                cur_token = TOK_LE;
                return cur_token;
            }
            cur_token = TOK_LT;
            return cur_token;

        case '>':
            state.p_cur++;
            if (state.p_cur < state.p_line_end && state.p_cur[0] == '=') {
                state.p_cur++;
                cur_token = TOK_GE;
                return cur_token;
            }
            cur_token = TOK_GT;
            return cur_token;

            // clang-format off
        case '=':  state.p_cur++; cur_token = TOK_EQ;        return cur_token;
        case '-':  state.p_cur++; cur_token = TOK_MINUS;     return cur_token;
        case '*':  state.p_cur++; cur_token = TOK_MULT;      return cur_token;
        case '/':  state.p_cur++; cur_token = TOK_DIV;       return cur_token;
        case '\\': state.p_cur++; cur_token = TOK_INTDIV;    return cur_token;
        case '^':  state.p_cur++; cur_token = TOK_POW;       return cur_token;
        case '+':  state.p_cur++; cur_token = TOK_PLUS;      return cur_token;
        case ',':  state.p_cur++; cur_token = TOK_COMMA;     return cur_token;
        case ':':  state.p_cur++; cur_token = TOK_COLON;     return cur_token;
        case ';':  state.p_cur++; cur_token = TOK_SEMICOLON; return cur_token;
        case '?':  state.p_cur++; cur_token = TOK_PRINT;     return cur_token;
        case '(':  state.p_cur++; cur_token = TOK_LPAREN;    return cur_token;
        case ')':  state.p_cur++; cur_token = TOK_RPAREN;    return cur_token;
        case '#':  state.p_cur++; cur_token = TOK_HASH;      return cur_token;
            // clang-format on
    }

    // Identifier / keyword?
    if (!parse_identifier(true))
        _basic_error(ERR_SYNTAX_ERROR);

    // Search keywords
    struct keyword *kw = bsearch(
        tokval_str,
        keywords,
        sizeof(keywords) / sizeof(keywords[0]), sizeof(keywords[0]),
        keyword_cmp);
    if (kw != NULL) {
        cur_token = kw->token;

        if (cur_token == TOK_END) {
            const uint8_t *p_save = state.p_cur;

            skip_whitespace();
            if (parse_identifier(false)) {
                // END IF?
                if (strcmp(tokval_str, "IF") == 0) {
                    cur_token = TOK_ENDIF;
                }
            }

            if (cur_token == TOK_END) {
                // No keyword following END, so estore position
                state.p_cur = p_save;
            }

        } else if (cur_token == TOK_LINE) {
            const uint8_t *p_save = state.p_cur;

            skip_whitespace();
            if (parse_identifier(false)) {
                // LINE INPUT?
                if (strcmp(tokval_str, "INPUT") == 0) {
                    cur_token = TOK_LINE_INPUT;
                }
            }

            if (cur_token == TOK_LINE) {
                // No keyword following LINE, so restore position
                state.p_cur = p_save;
            }
        }

        if (cur_token == TOK_REM) {
            // Remark, ignore rest of line
            state.p_cur = state.p_line_end;
        }

    } else {
        cur_token = TOK_IDENTIFIER;
    }

    return cur_token;
}

uint8_t get_token(void) {
    if (cur_token >= 0)
        return cur_token;

    int tok = _get_token();
    return tok;
}

void ack_token(void) {
    if (cur_token == TOK_EOL)
        err_line = state.cur_line;

    if (cur_token != TOK_END_OF_FILE)
        cur_token = -1;
}

void tokenizer_init(struct editbuf *_eb) {
    eb = _eb;

    state.p_line     = NULL;
    state.p_line_end = NULL;
    state.p_cur      = NULL;
    state.cur_line   = -1;

    cur_token = -1;
    err_line  = 0;
}

int tokenizer_get_cur_line(void) {
    return state.cur_line;
}

void tokenizer_save_state(struct tokenizer_state *st) {
    *st = state;
}

void tokenizer_restore_state(const struct tokenizer_state *st) {
    cur_token = -1;
    state     = *st;
}
