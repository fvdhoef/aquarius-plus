#include "common.h"
#include "tokens.h"
#include "expr.h"
#include "symbols.h"
#include "opcode_info.h"

static uint8_t fake_heap[40000];

#define MAX_FILE_DEPTH 8

struct file_ctx {
    uint16_t linenr;
    char     path[30];
};

static struct file_ctx file_ctxs[MAX_FILE_DEPTH];
static uint8_t         file_ctx_idx;
struct file_ctx       *cur_file_ctx;
static char            linebuf[256];
static const char     *label;
static const char     *keyword;
static const char     *string;
char                  *cur_p;
static uint8_t        *heap;
static uint8_t         pass     = 0;
static uint16_t        cur_addr = 0;

typedef void handler_t(void);

static void handler_unknown(void);
static void handler_defb(void);
static void handler_defs(void);
static void handler_defw(void);
static void handler_dephase(void);
static void handler_end(void);
static void handler_equ(void);
static void handler_incbin(void);
static void handler_include(void);
static void handler_org(void);
static void handler_phase(void);

static handler_t *directive_handlers[] = {
    handler_unknown,
    handler_defb,
    handler_defs,
    handler_defw,
    handler_dephase,
    handler_end,
    handler_equ,
    handler_incbin,
    handler_include,
    handler_org,
    handler_phase,
};

static void parse_file(const char *path);

void error(char *str) {
    if (str == NULL)
        str = "Unknown";
    printf("\n%s:%u Error (addr: $%04X): %s\n", cur_file_ctx->path, cur_file_ctx->linenr, cur_addr, str);
    exit(1);
}

void skip_whitespace(void) {
    while (1) {
        uint8_t ch = *cur_p;
        if (ch == ';' || ch == '\r' || ch == '\n') {
            *cur_p = 0;
            break;
        }
        if (ch == 0 || ch > ' ')
            break;
        cur_p++;
    }
}

static void parse_label(void) {
    label = cur_p;
    while (1) {
        uint8_t ch = *cur_p;
        if (ch <= ' ' || ch == ':')
            break;
        cur_p++;
    }
    if (*cur_p != 0)
        *(cur_p++) = 0;
}

uint8_t to_lower(uint8_t ch) {
    if (ch >= 'A' && ch <= 'Z')
        ch += 'a' - 'A';
    return ch;
}

static void parse_keyword(void) {
    skip_whitespace();
    keyword = cur_p;

    while (1) {
        uint8_t ch = to_lower(*cur_p);
        if (ch < 'a' || ch > 'z')
            break;
        cur_p++;
    }

    if (keyword == cur_p)
        keyword = NULL;
    else if (*cur_p != 0) {
        if (*cur_p > ' ')
            error("Syntax error");
        *(cur_p++) = 0;
    }
}

static void parse_string(void) {
    string = NULL;
    skip_whitespace();
    uint8_t ch = *(cur_p++);
    if (ch != '"')
        error("Expected '\"'");
    string = cur_p;

    while (1) {
        ch = *cur_p;
        if (ch == '"') {
            *(cur_p++) = 0;
            break;
        }
        if (ch < ' ' && ch != '\t')
            error("Invalid string");
        cur_p++;
    }
}

static int keyword_cmp(const uint8_t *s1, const uint8_t *s2, uint8_t n) {
    do {
        uint8_t ch1 = to_lower(*(s1++));
        uint8_t ch2 = *(s2++);
        if (ch1 != ch2) {
            return (int)ch1 - (int)ch2;
        }
    } while (--n != 0);
    return 0;
}

static uint8_t tokenize_keyword(const char *keyword) {
    uint8_t keyword_len = strlen(keyword);
    if (keyword_len < 2 || keyword_len > 7)
        return TOK_UNKNOWN;

    uint8_t        num_elems  = num_keywords[keyword_len - 2];
    const uint8_t *elems_base = keywords[keyword_len - 2];
    uint8_t        elem_size  = keyword_len + 1;

    for (uint8_t lim = num_elems; lim != 0; lim >>= 1) {
        const uint8_t *p   = elems_base + (lim >> 1) * elem_size;
        int            cmp = keyword_cmp((const uint8_t *)keyword, p, keyword_len);
        if (cmp == 0) {
            return p[keyword_len];
        }
        if (cmp > 0) { // key > p: move right
            elems_base = p + elem_size;
            lim--;
        } // else move left
    }
    return TOK_UNKNOWN;
}

static void emit_byte(uint16_t val) {
    if ((val & 0xFF00) != 0)
        error("Invalid byte value");

    printf("[$%04X:$%02X (%c)]\n", cur_addr, val, (val >= ' ' && val <= '~') ? val : '.');
    cur_addr++;
}

static void handler_unknown(void) {
    error("Syntax error");
}
static void handler_defb(void) {
    skip_whitespace();

    while (1) {
        if (cur_p[0] == '"') {
            // String constant
            parse_string();
            while (*string) {
                emit_byte(*(string++));
            }
        } else {
            emit_byte(parse_expression(pass == 0));
        }

        skip_whitespace();
        if (cur_p[0] == 0)
            break;
        if (cur_p[0] != ',')
            error("Syntax error: expected comma");
        cur_p++;
    }
}
static void handler_defw(void) {
    skip_whitespace();

    while (1) {
        uint16_t val = parse_expression(pass == 0);
        emit_byte(val & 0xFF);
        emit_byte(val >> 8);

        skip_whitespace();
        if (cur_p[0] == 0)
            break;
        if (cur_p[0] != ',')
            error("Syntax error: expected comma");
        cur_p++;
    }
}
static void handler_defs(void) {
    uint16_t length = parse_expression(false);
    while (length--) {
        emit_byte(0);
    }
}
static void handler_end(void) {
    error("Not implemented");
}
static void handler_equ(void) {
    if (!label)
        error("Equ without label");
    symbol_add(label, 0, parse_expression(false));
}
static void handler_incbin(void) {
    error("Not implemented");
}
static void handler_include(void) {
    parse_string();
    skip_whitespace();
    if (*cur_p != 0)
        error("Syntax error");

    printf("\nInclude file: '%s'\n", string);
    parse_file(string);
    printf("----------------------\n");
}
static void handler_org(void) {
    uint16_t addr = parse_expression(false);
    if (addr < cur_addr) {
        error("Org value < cur addr");
    } else {
        if (cur_addr == 0) {
            cur_addr = addr;
        } else {
            uint16_t pad_length = addr - cur_addr;
            while (pad_length--) {
                emit_byte(0);
            }
        }
    }
    printf("[Org addr: $%04x]\n", addr);
}
static void handler_phase(void) {
    error("Not implemented");
}
static void handler_dephase(void) {
    error("Not implemented");
}

static char *parse_argument(void) {
    skip_whitespace();
    char *result = cur_p;

    // Remove spaces and comments from rest of line
    char *ps = cur_p;
    char *pd = cur_p;
    while (ps[0] && ps[0] != ';' && ps[0] != '\r' && ps[0] != '\n') {
        if (ps[0] <= ' ') {
            ps++;
            continue;
        }
        if (ps[0] == ',') {
            ps++;
            break;
        }

        // Copy character constants as is
        if (ps[0] == '\'') {
            if (ps[1] == 0 || ps[2] != '\'')
                error("Syntax error");
            *(pd++) = *(ps++);
            *(pd++) = *(ps++);
        }
        *(pd++) = *(ps++);
    }
    *pd   = 0;
    cur_p = ps;

    return result;
}

enum {
    ARGTYPE_NONE = 0,
    ARGTYPE_REG  = 0x40,
    ARGTYPE_IND  = 0x80,
    ARGTYPE_COND = 0xC0,

    ARGTYPE_IMM     = 0x01,
    ARGTYPE_IMM_IND = ARGTYPE_IND | ARGTYPE_IMM,

    ARGTYPE_REG_A   = ARGTYPE_REG | 0,
    ARGTYPE_REG_B   = ARGTYPE_REG | 1,
    ARGTYPE_REG_C   = ARGTYPE_REG | 2,
    ARGTYPE_REG_D   = ARGTYPE_REG | 3,
    ARGTYPE_REG_E   = ARGTYPE_REG | 4,
    ARGTYPE_REG_H   = ARGTYPE_REG | 5,
    ARGTYPE_REG_L   = ARGTYPE_REG | 6,
    ARGTYPE_REG_IXH = ARGTYPE_REG | 7,
    ARGTYPE_REG_IXL = ARGTYPE_REG | 8,
    ARGTYPE_REG_IYH = ARGTYPE_REG | 9,
    ARGTYPE_REG_IYL = ARGTYPE_REG | 10,
    ARGTYPE_REG_F   = ARGTYPE_REG | 11,
    ARGTYPE_REG_I   = ARGTYPE_REG | 12,
    ARGTYPE_REG_R   = ARGTYPE_REG | 13,
    ARGTYPE_REG_AF  = ARGTYPE_REG | 14,
    ARGTYPE_REG_BC  = ARGTYPE_REG | 15,
    ARGTYPE_REG_DE  = ARGTYPE_REG | 16,
    ARGTYPE_REG_HL  = ARGTYPE_REG | 17,
    ARGTYPE_REG_IX  = ARGTYPE_REG | 18,
    ARGTYPE_REG_IY  = ARGTYPE_REG | 19,
    ARGTYPE_REG_SP  = ARGTYPE_REG | 20,

    ARGTYPE_REG_C_IND       = ARGTYPE_IND | ARGTYPE_REG_C,
    ARGTYPE_REG_BC_IND      = ARGTYPE_IND | ARGTYPE_REG_BC,
    ARGTYPE_REG_DE_IND      = ARGTYPE_IND | ARGTYPE_REG_DE,
    ARGTYPE_REG_HL_IND      = ARGTYPE_IND | ARGTYPE_REG_HL,
    ARGTYPE_REG_IX_IND      = ARGTYPE_IND | ARGTYPE_REG_IX,
    ARGTYPE_REG_IX_IND_OFFS = ARGTYPE_IND | ARGTYPE_REG_IX,
    ARGTYPE_REG_IY_IND      = ARGTYPE_IND | ARGTYPE_REG_IY,
    ARGTYPE_REG_IY_IND_OFFS = ARGTYPE_IND | ARGTYPE_REG_IY,
    ARGTYPE_REG_SP_IND      = ARGTYPE_IND | ARGTYPE_REG_SP,

    ARGTYPE_COND_NZ = ARGTYPE_COND | 0,
    ARGTYPE_COND_Z  = ARGTYPE_COND | 1,
    ARGTYPE_COND_NC = ARGTYPE_COND | 2,
    ARGTYPE_COND_C  = ARGTYPE_COND | 3,
    ARGTYPE_COND_PO = ARGTYPE_COND | 4,
    ARGTYPE_COND_PE = ARGTYPE_COND | 5,
    ARGTYPE_COND_P  = ARGTYPE_COND | 6,
    ARGTYPE_COND_M  = ARGTYPE_COND | 7,
};

struct reg {
    char    name[5];
    uint8_t argtype;
    uint8_t value;
};

static const struct reg regs[] = {
    {"a", ARGTYPE_REG_A, 7},
    {"b", ARGTYPE_REG_B, 0},
    {"c", ARGTYPE_REG_C, 1},
    {"d", ARGTYPE_REG_D, 2},
    {"e", ARGTYPE_REG_E, 3},
    {"h", ARGTYPE_REG_H, 4},
    {"l", ARGTYPE_REG_L, 5},
    {"ixh", ARGTYPE_REG_IXH, 4},
    {"ixl", ARGTYPE_REG_IXL, 5},
    {"iyh", ARGTYPE_REG_IYH, 4},
    {"iyl", ARGTYPE_REG_IYL, 5},
    {"f", ARGTYPE_REG_F, 6},
    {"i", ARGTYPE_REG_I, -1},
    {"r", ARGTYPE_REG_R, -1},
    {"af", ARGTYPE_REG_AF, 3},
    {"bc", ARGTYPE_REG_BC, 0},
    {"de", ARGTYPE_REG_DE, 1},
    {"hl", ARGTYPE_REG_HL, 2},
    {"ix", ARGTYPE_REG_IX, 2},
    {"iy", ARGTYPE_REG_IY, 2},
    {"sp", ARGTYPE_REG_SP, 3},
    {"(c)", ARGTYPE_REG_C_IND, 7},
    {"(bc)", ARGTYPE_REG_BC_IND, 0},
    {"(de)", ARGTYPE_REG_DE_IND, 1},
    {"(hl)", ARGTYPE_REG_HL_IND, 6},
    {"(ix)", ARGTYPE_REG_IX_IND, 2},
    {"(ix+", ARGTYPE_REG_IX_IND_OFFS, -1},
    {"(iy)", ARGTYPE_REG_IY_IND, 2},
    {"(iy+", ARGTYPE_REG_IY_IND_OFFS, -1},
    {"(sp)", ARGTYPE_REG_SP_IND, 3},
};

static const struct reg conditions[] = {
    {"nz", ARGTYPE_COND_NZ, 0},
    {"z", ARGTYPE_COND_Z, 1},
    {"nc", ARGTYPE_COND_NC, 2},
    {"c", ARGTYPE_COND_C, 3},
    {"po", ARGTYPE_COND_PO, 4},
    {"pe", ARGTYPE_COND_PE, 5},
    {"p", ARGTYPE_COND_P, 6},
    {"m", ARGTYPE_COND_M, 7},
};

static uint16_t arg_value;

static uint8_t get_argtype(const char *arg) {
    arg_value = 0;
    if (!arg[0])
        return ARGTYPE_NONE;

    if (arg[0] == '(') {
        if (arg[1] == 'i') {
            if (arg[2] == 'x' || arg[2] == 'y') {
                if (arg[3] == ')')
                    // Indirect IX/IY
                    return arg[2] == 'x' ? ARGTYPE_REG_IX_IND : ARGTYPE_REG_IY_IND;
                if (arg[3] == '+') {
                    // Indirect IX/IY with offset
                    cur_p     = (char *)(arg + 3);
                    arg_value = parse_expression(pass == 0);
                    if (*cur_p != ')')
                        goto err;
                    return arg[2] == 'x' ? ARGTYPE_REG_IX_IND_OFFS : ARGTYPE_REG_IY_IND_OFFS;
                }
            }
        }

        cur_p     = (char *)(arg + 1);
        arg_value = parse_expression(pass == 0);
        if (cur_p[0] != ')' || cur_p[1] != 0)
            goto err;
        return ARGTYPE_IMM_IND;
    }

    cur_p     = (char *)arg;
    arg_value = parse_expression(pass == 0);
    if (cur_p[0] != 0)
        goto err;
    return ARGTYPE_IMM;

err:
    error("Syntax error");
    return ARGTYPE_NONE;
}

static void parse_file(const char *path) {
    FILE *f = fopen(path, "r");

    if (cur_file_ctx != NULL) {
        file_ctx_idx++;
        if (file_ctx_idx >= MAX_FILE_DEPTH) {
            error("Max file depth reached");
        }
    }
    cur_file_ctx         = &file_ctxs[file_ctx_idx];
    cur_file_ctx->linenr = 0;
    {
        char *p = cur_file_ctx->path;
        for (uint8_t i = 0; i < sizeof(cur_file_ctx->path) - 1; i++) {
            char ch = path[i];
            if (ch == 0)
                break;
            *(p++) = ch;
        }
        *p = 0;
    }

    while (1) {
        cur_file_ctx->linenr++;
        if (fgets(linebuf, 256, f) == NULL)
            break;

        label      = NULL;
        keyword    = NULL;
        cur_p      = linebuf;
        uint8_t ch = *cur_p;
        if (ch == ';' || ch == '\r' || ch == '\n')
            continue;
        if (ch != ' ' && ch != '\t')
            parse_label();

        // Check for keyword
        parse_keyword();
        uint8_t token = TOK_UNKNOWN;
        if (keyword) {
            token = tokenize_keyword(keyword);
        }
        if (token == TOK_EQU) {
            directive_handlers[token]();
            skip_whitespace();
            if (*cur_p != 0)
                error("Syntax error: expected end-of-line");
            continue;
        }
        if (label)
            symbol_add(label, 0, cur_addr);
        if (!keyword && *cur_p == 0)
            continue;

        if (token <= TOK_DIR_LAST) {
            directive_handlers[token]();
            if (token == TOK_INCLUDE)
                continue;

        } else {
            const uint8_t *info    = opcode_info[token - TOK_OPCODE_FIRST];
            uint8_t        opdesc  = *info;
            uint8_t        argtype = opdesc & ARGTYPE_MASK;
            uint8_t        prefix  = opdesc & PREFIX_MASK;
            if (argtype == 0)
                error("Unimplemented opcode");

            const char *arg1 = parse_argument();
            const char *arg2 = parse_argument();
            if (*cur_p != 0)
                error("Syntax error: expected end-of-line");
            if (argtype == ARGTYPE_NONE && (arg1 || arg2))
                error("Syntax error");

            uint8_t  arg_type1  = get_argtype(arg1);
            uint16_t arg_value1 = arg_value;
            uint8_t  arg_type2  = get_argtype(arg2);
            uint16_t arg_value2 = arg_value;

            printf("opcode:%s, arg1:%s (type:%u, value:%u), arg2:%s (type:%u, value:%u)\n", keyword, arg1, arg_type1, arg_value1, arg2, arg_type2, arg_value2);

#if 0
            printf("Remaining: '%s'\n", cur_p);

            printf("[Keyword: '%s' token:%u]", keyword, token);

            const uint8_t *info = opcode_info[token - TOK_OPCODE_FIRST];

            uint8_t opdesc = *info;
            if (opdesc == 0)
                error("Unimplemented opcode");

            printf("opdesc: %02X\n", opdesc);

            uint8_t argtype = opdesc & ARGTYPE_MASK;
            uint8_t prefix  = opdesc & PREFIX_MASK;

            uint16_t value = 0;
            if (argtype != ARG_NONE) {
                parse_argument();

                // ch  = to_lower(cur_p[0]);
                // ch2 = to_lower(cur_p[1]);

                // if (ch == 'a') {
                // } else if (ch == 'b') {
                // } else if (ch == 'b') {
                // }

                // if (cur_p[0] == 'a')
            }

            // switch (argtype) {
            //     case ARG_NONE: break;
            //     case ARG_IMM16: value = parse_expression(pass == 0); break;
            //     default: error("Unimplemented argument type");
            // }

            switch (prefix) {
                case PREFIX_NONE: emit_byte(info[1]); break;
                case PREFIX_ED:
                    emit_byte(0xED);
                    emit_byte(info[1]);
                    break;
                default: error("Unimplemented prefix");
            }

            switch (argtype) {
                case ARG_NONE: break;
                case ARG_IMM8:
                    emit_byte(value & 0xFF);
                    emit_byte(value >> 8);
                    break;

                case ARG_IMM16:
                    emit_byte(value & 0xFF);
                    emit_byte(value >> 8);
                    break;
            }

            // struct opcode_info *info = &opcode_info[token - TOK_OPCODE_FIRST];

            // printf("param1: %04x  param2: %04x\n", info->param1, info->param2);
#endif
        }
    }

    fclose(f);

    if (file_ctx_idx == 0) {
        cur_file_ctx = NULL;
    } else {
        cur_file_ctx = &file_ctxs[--file_ctx_idx];
    }
}

int main(void) {
    heap = fake_heap;
    chdir("testdata");
    parse_file("goaqms.asm");

    return 0;
}
