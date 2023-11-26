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

static FILE *f_list;
static char  list_line[256];
static char *list_p;

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

uint16_t        cur_scope   = 0;
static uint8_t  cur_opcode  = 0;
static uint8_t  cur_outtype = 0;
static uint16_t arg_value;
static uint8_t  d_value;

enum {
    OT_NONE,
    OT_8BIT,
    OT_16BIT,

    OT_D         = (1 << 5),
    OT_PREFIX_DD = (1 << 6),
    OT_PREFIX_FD = (2 << 6),
};

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

    list_p += sprintf(list_p, "%02X", val);

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
        if (ps[0] == '\'' && ps[1] != 0 && ps[2] == '\'') {
            *(pd++) = *(ps++);
            *(pd++) = *(ps++);
        }
        *(pd++) = *(ps++);
    }
    *pd   = 0;
    cur_p = ps;

    return result;
}

static const char *regs8_all[]      = {"b", "c", "d", "e", "h", "l", "(hl)", "a"};
static const char *regs8_bcdehlfa[] = {"b", "c", "d", "e", "h", "l", "f", "a"};
static const char *regs8_ixhl[]     = {"b", "c", "d", "e", "ixh", "ixl", "", "a"};
static const char *regs8_iyhl[]     = {"b", "c", "d", "e", "iyh", "iyl", "", "a"};

static const char *regs_bc_de_hl_sp[] = {"bc", "de", "hl", "sp"};
static const char *regs_bc_de_hl_af[] = {"bc", "de", "hl", "af"};
static const char *regs_bc_de_ix_sp[] = {"bc", "de", "ix", "sp"};
static const char *regs_bc_de_iy_sp[] = {"bc", "de", "iy", "sp"};
static const char *cond_all[]         = {"nz", "z", "nc", "c", "po", "pe", "p", "m"};

static bool match_argtype(const char *arg, uint8_t arg_type) {
    printf("    - match_argtype(\"%s\", %d)\n", arg, arg_type);
    if (arg[0] == 0 && arg_type != OD_AT_NONE)
        return false;

    if (arg[0] == '(') {
        // Indirect access

        switch (arg_type) {
            case OD_AT_IMM8_IND:
                if (arg[0] != '(')
                    return false;
                cur_p     = (char *)arg + 1;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != ')' && cur_p[1] != 0)
                    goto err;
                cur_p++;
                if (arg_value >> 8) {
                    error("Value too large!");
                }
                cur_outtype |= OT_8BIT;
                return true;
            case OD_AT_IMM16_IND:
                if (arg[0] != '(')
                    return false;
                cur_p     = (char *)arg + 1;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != ')' && cur_p[1] != 0)
                    goto err;
                cur_p++;
                cur_outtype |= OT_16BIT;
                return true;
            case OD_AT_C_IND: return strcasecmp(arg, "(c)") == 0;
            case OD_AT_BC_IND: return strcasecmp(arg, "(bc)") == 0;
            case OD_AT_DE_IND: return strcasecmp(arg, "(de)") == 0;
            case OD_AT_HL_IND: return strcasecmp(arg, "(hl)") == 0;
            case OD_AT_SP_IND: return strcasecmp(arg, "(sp)") == 0;
            case OD_AT_20_REG_ALL:
                if (strcasecmp(arg, "(hl)") == 0) {
                    cur_opcode |= 6;
                    return true;
                }
                return false;
            case OD_AT_53_REG_ALL:
                if (strcasecmp(arg, "(hl)") == 0) {
                    cur_opcode |= 6 << 3;
                    return true;
                }
                return false;
            case OD_AT_IX_IND:
                if (strcasecmp(arg, "(ix)") == 0) {
                    cur_outtype |= OT_PREFIX_DD;
                    return true;
                }
                return false;
            case OD_AT_IX_IND_OFFS: {
                if (arg[0] != '(' || to_lower(arg[1]) != 'i' || to_lower(arg[2]) != 'x' || arg[3] != '+')
                    return false;
                cur_p          = (char *)arg + 1;
                uint16_t value = parse_expression(pass == 0);
                if (cur_p[0] != ')' && cur_p[1] != 0)
                    goto err;
                d_value = value;
                cur_p++;
                cur_outtype |= OT_PREFIX_DD | OT_D;
                return true;
            }
            case OD_AT_IY_IND:
                if (strcasecmp(arg, "(iy)") == 0) {
                    cur_outtype |= OT_PREFIX_FD;
                    return true;
                }
                return false;
            case OD_AT_IY_IND_OFFS: {
                if (arg[0] != '(' || to_lower(arg[1]) != 'i' || to_lower(arg[2]) != 'y' || arg[3] != '+')
                    return false;
                cur_p          = (char *)arg + 1;
                uint16_t value = parse_expression(pass == 0);
                if (cur_p[0] != ')' && cur_p[1] != 0)
                    goto err;
                d_value = value;
                cur_p++;
                cur_outtype |= OT_PREFIX_FD | OT_D;
                return true;
            }
            default: return false;
        }

    } else {

        switch (arg_type) {
            case OD_AT_NONE: return (*arg == 0);
            case OD_AT_IMM_0: return (arg[0] == '0' && arg[1] == 0);
            case OD_AT_53_IMM:
                cur_p     = (char *)arg;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != 0)
                    goto err;
                if (arg_value > 7)
                    error("Value out of range!");
                cur_opcode |= arg_value << 3;
                return true;

            case OD_AT_43_IMODE:
                cur_p     = (char *)arg;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != 0)
                    goto err;
                if (arg_value > 2)
                    error("Value out of range!");
                if (arg_value > 0)
                    arg_value += 1;
                cur_opcode |= arg_value << 3;
                return true;

            case OD_AT_53_RST: error("OD_AT_53_RST");
            case OD_AT_IMM8:
                cur_p     = (char *)arg;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != 0)
                    goto err;
                if (arg_value >> 8)
                    error("Value out of range!");
                cur_outtype |= OT_8BIT;
                return true;
            case OD_AT_IMM16:
                cur_p     = (char *)arg;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != 0)
                    goto err;
                cur_outtype |= OT_16BIT;
                return true;
            case OD_AT_REL_ADDR:
                cur_p     = (char *)arg;
                arg_value = parse_expression(pass == 0);
                if (cur_p[0] != 0)
                    goto err;
                arg_value = 0; // FIXME!!!
                cur_outtype |= OT_8BIT;
                return true;
            case OD_AT_A: return to_lower(arg[0]) == 'a' && arg[1] == 0;
            case OD_AT_R: return to_lower(arg[0]) == 'r' && arg[1] == 0;
            case OD_AT_I: return to_lower(arg[0]) == 'i' && arg[1] == 0;
            case OD_AT_DE: return strcasecmp(arg, "de") == 0;
            case OD_AT_HL: return strcasecmp(arg, "hl") == 0;
            case OD_AT_SP: return strcasecmp(arg, "sp") == 0;
            case OD_AT_AF: return strcasecmp(arg, "af") == 0;
            case OD_AT_AF_ALT: return strcasecmp(arg, "af'") == 0;
            case OD_AT_20_REG_ALL:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_all[i]) == 0) {
                        cur_opcode |= i;
                        return true;
                    }
                }
                return false;
            case OD_AT_20_BCDEHLA:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_all[i]) == 0) {
                        cur_opcode |= i;
                        return true;
                    }
                }
                return false;
            case OD_AT_53_REG_ALL:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_all[i]) == 0) {
                        cur_opcode |= i << 3;
                        return true;
                    }
                }
                return false;
            case OD_AT_53_BCDEHLFA:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_bcdehlfa[i]) == 0) {
                        cur_opcode |= i << 3;
                        return true;
                    }
                }
                return false;
            case OD_AT_53_BCDEHLA:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_all[i]) == 0) {
                        cur_opcode |= i << 3;
                        return true;
                    }
                }
                return false;

            case OD_AT_53_BCDEA: error("OD_AT_53_BCDEA");
            case OD_AT_53_COND:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, cond_all[i]) == 0) {
                        cur_opcode |= i << 3;
                        return true;
                    }
                }
                return false;

            case OD_AT_43_COND:
                for (int i = 0; i < 4; i++) {
                    if (strcasecmp(arg, cond_all[i]) == 0) {
                        cur_opcode |= i << 3;
                        return true;
                    }
                }
                return false;

            case OD_AT_54_BC_DE_HL_AF:
                for (int i = 0; i < 4; i++) {
                    if (strcasecmp(arg, regs_bc_de_hl_af[i]) == 0) {
                        cur_opcode |= i << 4;
                        return true;
                    }
                }
                return false;
            case OD_AT_54_BC_DE_HL_SP:
                for (int i = 0; i < 4; i++) {
                    if (strcasecmp(arg, regs_bc_de_hl_sp[i]) == 0) {
                        cur_opcode |= i << 4;
                        return true;
                    }
                }
                return false;
            case OD_AT_IX:
                if (strcasecmp(arg, "ix") == 0) {
                    cur_outtype |= OT_PREFIX_DD;
                    return true;
                }
                return false;

            case OD_AT_20_IXHL:
                for (int i = 4; i <= 5; i++) {
                    if (strcasecmp(arg, regs8_ixhl[i]) == 0) {
                        cur_opcode |= i;
                        cur_outtype |= OT_PREFIX_DD;
                        return true;
                    }
                }
                return false;

            case OD_AT_53_IXHL:
                for (int i = 4; i <= 5; i++) {
                    if (strcasecmp(arg, regs8_ixhl[i]) == 0) {
                        cur_opcode |= i << 3;
                        cur_outtype |= OT_PREFIX_DD;
                        return true;
                    }
                }
                return false;

            case OD_AT_20_IXHL_ALL:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_ixhl[i]) == 0) {
                        cur_opcode |= i;
                        cur_outtype |= OT_PREFIX_DD;
                        return true;
                    }
                }
                return false;

            case OD_AT_53_IXHL_ALL:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_ixhl[i]) == 0) {
                        cur_opcode |= i << 3;
                        cur_outtype |= OT_PREFIX_DD;
                        return true;
                    }
                }
                return false;

            case OD_AT_54_BC_DE_IY_SP:
                for (int i = 0; i < 4; i++) {
                    if (strcasecmp(arg, regs_bc_de_iy_sp[i]) == 0) {
                        cur_opcode |= i << 4;
                        cur_outtype |= OT_PREFIX_FD;
                        return true;
                    }
                }
                return false;

            case OD_AT_IY:
                if (strcasecmp(arg, "iy") == 0) {
                    cur_outtype |= OT_PREFIX_FD;
                    return true;
                }
                return false;

            case OD_AT_20_IYHL:
                for (int i = 4; i <= 5; i++) {
                    if (strcasecmp(arg, regs8_iyhl[i]) == 0) {
                        cur_opcode |= i;
                        cur_outtype |= OT_PREFIX_FD;
                        return true;
                    }
                }
                return false;

            case OD_AT_53_IYHL:
                for (int i = 4; i <= 5; i++) {
                    if (strcasecmp(arg, regs8_iyhl[i]) == 0) {
                        cur_opcode |= i << 3;
                        cur_outtype |= OT_PREFIX_FD;
                        return true;
                    }
                }
                return false;

            case OD_AT_20_IYHL_ALL:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_iyhl[i]) == 0) {
                        cur_opcode |= i;
                        cur_outtype |= OT_PREFIX_FD;
                        return true;
                    }
                }
                return false;

            case OD_AT_53_IYHL_ALL:
                for (int i = 0; i < 8; i++) {
                    if (strcasecmp(arg, regs8_iyhl[i]) == 0) {
                        cur_opcode |= i << 3;
                        cur_outtype |= OT_PREFIX_FD;
                        return true;
                    }
                }
                return false;

            case OD_AT_54_BC_DE_IX_SP:
                for (int i = 0; i < 4; i++) {
                    if (strcasecmp(arg, regs_bc_de_ix_sp[i]) == 0) {
                        cur_opcode |= i << 4;
                        cur_outtype |= OT_PREFIX_DD;
                        return true;
                    }
                }
                return false;

            default: return false;
        }
    }

err:
    error("Syntax error");
    return false;
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
        list_p  = list_line;
        *list_p = 0;

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
            const uint8_t *info_next = opcode_info[token - TOK_OPCODE_FIRST];
            const char    *arg1      = parse_argument();
            const char    *arg2      = parse_argument();
            if (*cur_p != 0)
                error("Syntax error: expected end-of-line");

            bool done = false;

            list_p += sprintf(list_p, "%04X  ", cur_addr);

            printf("- %s:%u: %s %s%s%s\n", cur_file_ctx->path, cur_file_ctx->linenr, keyword, arg1, arg2[0] ? "," : "", arg2);

            while (info_next) {
                const uint8_t *info = info_next;
                info_next           = (info[0] & 0x80) ? (info + 3) : NULL;
                uint8_t arg1_type   = info[0] & 0x3F;
                uint8_t arg2_type   = info[1] & 0x3F;
                uint8_t prefix_type = info[1] >> 6;
                cur_opcode          = info[2];
                if (arg1_type == 0x3F)
                    error("Unimplemented opcode");

                cur_outtype = OT_NONE;
                printf("  - arg1:%2u arg2:%2u prefix:%u opcode:$%02X\n", arg1_type, arg2_type, prefix_type, cur_opcode);

                if (!match_argtype(arg1, arg1_type) || !match_argtype(arg2, arg2_type))
                    continue;

                switch (cur_outtype & 0xC0) {
                    case OT_PREFIX_DD: emit_byte(0xDD); break;
                    case OT_PREFIX_FD: emit_byte(0xFD); break;
                    default: break;
                }
                switch (prefix_type) {
                    case OD_PREFIX_CB: emit_byte(0xCB); break;
                    case OD_PREFIX_ED: emit_byte(0xED); break;
                    default: break;
                }
                if (prefix_type == OD_PREFIX_CB && (cur_outtype & OT_D)) {
                    emit_byte(d_value);
                    emit_byte(cur_opcode);
                } else {
                    emit_byte(cur_opcode);
                    if (cur_outtype & OT_D) {
                        emit_byte(d_value);
                    }
                    switch (cur_outtype & 0x1F) {
                        case OT_NONE: break;
                        case OT_8BIT: emit_byte(arg_value); break;
                        case OT_16BIT:
                            emit_byte(arg_value & 0xFF);
                            emit_byte(arg_value >> 8);
                            break;
                        default: error("Invalid outtype");
                    }
                }

                done = true;
                break;
            }
            if (!done) {
                error("Syntax error");
            }

            while (list_p - list_line < 14) {
                *(list_p++) = ' ';
            }
            *(list_p++) = '\t';
            list_p += sprintf(list_p, "    %s%s%s%s%s", keyword, arg1[0] ? " " : "", arg1, arg2[0] ? "," : "", arg2);
        }

        *(list_p++) = '\n';
        *list_p     = 0;
        fputs(list_line, f_list);
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

    f_list = fopen("listing.txt", "wt");

    cur_scope = 0;
    pass      = 0;
    // parse_file("goaqms.asm");
    parse_file("all_opcodes.asm");

    fclose(f_list);

    return 0;
}
