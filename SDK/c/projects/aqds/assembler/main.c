#include "common.h"
#include "tokens.h"
#include "expr.h"
#include "symbols.h"
#include "opcode_info.h"

#ifdef __SDCC
#include "screen.h"
#endif

#define MAX_FILE_DEPTH 8

struct file_ctx {
    uint16_t linenr;
    char     path[30];
};

enum {
    OT_NONE,
    OT_8BIT,
    OT_16BIT,
    OT_D         = (1 << 5),
    OT_PREFIX_DD = (1 << 6),
    OT_PREFIX_FD = (2 << 6),
};

#if 0 // ndef __SDCC
#define ENABLE_LISTING
#endif

#ifdef ENABLE_LISTING
static FILE *f_list;
static char  list_line[256];
static char *list_p;
#endif

static int8_t          fd_out = -1;
static uint8_t         outbuf[128];
static uint8_t        *outbuf_p   = outbuf;
static uint8_t         outbuf_idx = 0;
static struct file_ctx file_ctxs[MAX_FILE_DEPTH];
static uint8_t         file_ctx_idx;
struct file_ctx       *cur_file_ctx;
static char            linebuf[256];
static const char     *label;
static const char     *keyword;
static const char     *string;
char                  *cur_p;
uint8_t                cur_pass    = 0;
static uint16_t        cur_addr    = 0;
uint16_t               cur_scope   = 0;
static uint8_t         cur_opcode  = 0;
static uint8_t         cur_outtype = 0;
static uint16_t        arg_value;
static uint8_t         d_value;
static bool            stop_parsing;

static const char *regs8_all[]        = {"b", "c", "d", "e", "h", "l", "(hl)", "a"};
static const char *regs8_bcdehlfa[]   = {"b", "c", "d", "e", "h", "l", "f", "a"};
static const char *regs8_ixhl[]       = {"b", "c", "d", "e", "ixh", "ixl", "", "a"};
static const char *regs8_iyhl[]       = {"b", "c", "d", "e", "iyh", "iyl", "", "a"};
static const char *regs_bc_de_hl_sp[] = {"bc", "de", "hl", "sp"};
static const char *regs_bc_de_hl_af[] = {"bc", "de", "hl", "af"};
static const char *regs_bc_de_ix_sp[] = {"bc", "de", "ix", "sp"};
static const char *regs_bc_de_iy_sp[] = {"bc", "de", "iy", "sp"};
static const char *cond_all[]         = {"nz", "z", "nc", "c", "po", "pe", "p", "m"};

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

void exit_program(void) {
#ifdef __SDCC
    puts("\nPress enter to quit.\n");
    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == '\r')
            break;
    }

    // Go back to file manager
    __asm__("jp 0xF806");
#else
    exit(1);
#endif
}

void error(const char *str) {
    if (str == NULL)
        str = "Unknown";
    printf("\n%s:%u Error (addr: $%04X): %s\n", cur_file_ctx->path, cur_file_ctx->linenr, cur_addr, str);
    exit_program();
}

#ifdef __SDCC
void *malloc(size_t size) {
    static uint8_t *endp;
    if (endp == NULL) {
        endp = getheap();
    }

    uint16_t remaining = (uint8_t *)0xF000 - endp;
    if (size > remaining) {
        error("Out of memory!");
    }

    uint8_t *result = endp;
    endp += size;
    return result;
}
#endif

void syntax_error(void) {
    error("Syntax error");
}

void range_error(void) {
    error("Range error");
}

void check_esp_result(int16_t result) {
    if (result < 0) {
        switch (result) {
            case ERR_NOT_FOUND: puts("File / directory not found"); break;
            case ERR_TOO_MANY_OPEN: puts("Too many open files / directories"); break;
            case ERR_PARAM: puts("Invalid parameter"); break;
            case ERR_EOF: puts("End of file / directory"); break;
            case ERR_EXISTS: puts("File already exists"); break;
            default:
            case ERR_OTHER: puts("Other puts"); break;
            case ERR_NO_DISK: puts("No disk"); break;
            case ERR_NOT_EMPTY: puts("Not empty"); break;
            case ERR_WRITE_PROTECT: puts("Write protected SD-card"); break;
        }
        exit_program();
    }
}

void expect_comma(void) {
    if (cur_p[0] != ',')
        error("Expected comma");
    cur_p++;
}

void expect_end_of_line(void) {
    if (cur_p[0] != 0)
        error("Expected end of line");
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
            syntax_error();
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

static void write_outbuf(void) {
    if (fd_out >= 0 && outbuf_idx > 0) {
        esp_write(fd_out, outbuf, outbuf_idx);
    }
    outbuf_idx = 0;
    outbuf_p   = outbuf;
}

static void emit_byte(uint16_t val) {
    if ((val & 0xFF00) != 0)
        error("Invalid byte value");

    if (cur_pass == 1) {
#ifdef ENABLE_LISTING
        list_p += sprintf(list_p, "%02X", val);
#endif

        *(outbuf_p++) = val;
        if (++outbuf_idx == sizeof(outbuf)) {
            write_outbuf();
        }
    }

    // printf("[$%04X:$%02X (%c)]\n", cur_addr, val, (val >= ' ' && val <= '~') ? val : '.');
    cur_addr++;
}

static void handler_unknown(void) {
    syntax_error();
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
            emit_byte(parse_expression(cur_pass == 0));
        }

        skip_whitespace();
        if (cur_p[0] == 0)
            break;
        expect_comma();
    }
}
static void handler_defw(void) {
    skip_whitespace();

    while (1) {
        uint16_t val = parse_expression(cur_pass == 0);
        emit_byte(val & 0xFF);
        emit_byte(val >> 8);

        skip_whitespace();
        if (cur_p[0] == 0)
            break;
        expect_comma();
    }
}
static void handler_defs(void) {
    uint16_t length = parse_expression(false);
    while (length--) {
        emit_byte(0);
    }
}
static void handler_end(void) {
    stop_parsing = true;
}
static void handler_equ(void) {
    if (!label)
        error("Equ without label");
    symbol_add(label, 0, parse_expression(false));
}
static void handler_incbin(void) {
    parse_string();
    skip_whitespace();
    if (*cur_p != 0)
        syntax_error();

    printf("  - Processing binary file: '%s'\n", string);

    int8_t fd = esp_open(string, FO_RDONLY);
    check_esp_result(fd);
    while (1) {
        // Use line buffer as temporary buffer
        int16_t result = esp_read(fd, linebuf, sizeof(linebuf));
        if (result <= 0)
            break;

        const uint8_t *p = (const uint8_t *)linebuf;
        while (result--) {
            emit_byte(*(p++));
        }
    }
    esp_close(fd);
}
static void handler_include(void) {
    parse_string();
    skip_whitespace();
    if (*cur_p != 0)
        syntax_error();

    printf("  - Processing include file: '%s'\n", string);
    parse_file(string);
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
    // printf("[Org addr: $%04x]\n", addr);
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

static bool compare_str(const char *s1, const char *s2) {
    while (to_lower(*s1) == *s2) {
        if (*s1 == 0)
            return true;
        s1++;
        s2++;
    }
    return false;
}

static bool match_imm8_ind(char *arg) {
    if (arg[0] != '(')
        return false;
    cur_p     = (char *)arg + 1;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();
    cur_p++;
    if (arg_value >> 8) {
        error("Value too large!");
    }
    cur_outtype |= OT_8BIT;
    return true;
}

static bool match_imm16_ind(char *arg) {
    if (arg[0] != '(')
        return false;
    cur_p     = (char *)arg + 1;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();

    cur_p++;
    cur_outtype |= OT_16BIT;
    return true;
}

static bool match_c_ind(char *arg) { return compare_str(arg, "(c)"); }
static bool match_bc_ind(char *arg) { return compare_str(arg, "(bc)"); }
static bool match_de_ind(char *arg) { return compare_str(arg, "(de)"); }
static bool match_hl_ind(char *arg) { return compare_str(arg, "(hl)"); }
static bool match_sp_ind(char *arg) { return compare_str(arg, "(sp)"); }
static bool match_20_reg_hl_ind(char *arg) {
    if (compare_str(arg, "(hl)")) {
        cur_opcode |= 6;
        return true;
    }
    return false;
}
static bool match_53_reg_hl_ind(char *arg) {
    if (compare_str(arg, "(hl)")) {
        cur_opcode |= 6 << 3;
        return true;
    }
    return false;
}
static bool match_ix_ind(char *arg) {
    if (compare_str(arg, "(ix)")) {
        cur_outtype |= OT_PREFIX_DD;
        return true;
    }
    return false;
}
static bool match_ix_ind_offs(char *arg) {
    if (arg[0] != '(' || to_lower(arg[1]) != 'i' || to_lower(arg[2]) != 'x' || arg[3] != '+')
        return false;
    cur_p          = (char *)arg + 4;
    uint16_t value = parse_expression(cur_pass == 0);
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();

    d_value = value;
    cur_p++;
    cur_outtype |= OT_PREFIX_DD | OT_D;
    return true;
}
static bool match_iy_ind(char *arg) {
    if (compare_str(arg, "(iy)")) {
        cur_outtype |= OT_PREFIX_FD;
        return true;
    }
    return false;
}
static bool match_iy_ind_offs(char *arg) {
    if (arg[0] != '(' || to_lower(arg[1]) != 'i' || to_lower(arg[2]) != 'y' || arg[3] != '+')
        return false;
    cur_p          = (char *)arg + 4;
    uint16_t value = parse_expression(cur_pass == 0);
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();

    d_value = value;
    cur_p++;
    cur_outtype |= OT_PREFIX_FD | OT_D;
    return true;
}

static bool match_none(char *arg) { return (*arg == 0); }
static bool match_imm_0(char *arg) { return (arg[0] == '0' && arg[1] == 0); }
static bool match_53_imm(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value > 7)
        range_error();
    cur_opcode |= arg_value << 3;
    return true;
}

static bool match_43_imode(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value > 2)
        range_error();
    if (arg_value > 0)
        arg_value += 1;
    cur_opcode |= arg_value << 3;
    return true;
}
static bool match_53_rst(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value < 7) {
        cur_opcode |= arg_value << 3;
    } else if ((arg_value & ~0x38) == 0) {
        cur_opcode |= arg_value;
    } else {
        return false;
    }
    return true;
}
static bool match_imm8(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value >> 8)
        range_error();
    cur_outtype |= OT_8BIT;
    return true;
}
static bool match_imm16(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    cur_outtype |= OT_16BIT;
    return true;
}
static bool match_rel_addr(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (cur_pass == 0) {
        arg_value = 0;
    } else {
        int16_t val = arg_value - (cur_addr + 2);
        if (val < -128 || val > 127)
            range_error();
        arg_value = val & 0xFF;
    }
    cur_outtype |= OT_8BIT;
    return true;
}
static bool match_a(char *arg) { return to_lower(arg[0]) == 'a' && arg[1] == 0; }
static bool match_r(char *arg) { return to_lower(arg[0]) == 'r' && arg[1] == 0; }
static bool match_i(char *arg) { return to_lower(arg[0]) == 'i' && arg[1] == 0; }
static bool match_de(char *arg) { return compare_str(arg, "de"); }
static bool match_hl(char *arg) { return compare_str(arg, "hl"); }
static bool match_sp(char *arg) { return compare_str(arg, "sp"); }
static bool match_af(char *arg) { return compare_str(arg, "af"); }
static bool match_af_alt(char *arg) { return compare_str(arg, "af'"); }
static bool match_20_reg_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_all[i])) {
            cur_opcode |= i;
            return true;
        }
    }
    return false;
}
static bool match_20_bcdehla(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_all[i])) {
            cur_opcode |= i;
            return true;
        }
    }
    return false;
}
static bool match_53_reg_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}
static bool match_53_bcdehlfa(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_bcdehlfa[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}
static bool match_53_bcdehla(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}

static bool match_53_cond(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, cond_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}

static bool match_43_cond(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, cond_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}

static bool match_54_bc_de_hl_af(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_hl_af[i])) {
            cur_opcode |= i << 4;
            return true;
        }
    }
    return false;
}
static bool match_54_bc_de_hl_sp(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_hl_sp[i])) {
            cur_opcode |= i << 4;
            return true;
        }
    }
    return false;
}
static bool match_ix(char *arg) {
    if (compare_str(arg, "ix")) {
        cur_outtype |= OT_PREFIX_DD;
        return true;
    }
    return false;
}

static bool match_20_ixhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_53_ixhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_20_ixhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_53_ixhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_54_bc_de_iy_sp(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_iy_sp[i])) {
            cur_opcode |= i << 4;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_iy(char *arg) {
    if (compare_str(arg, "iy")) {
        cur_outtype |= OT_PREFIX_FD;
        return true;
    }
    return false;
}

static bool match_20_iyhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_53_iyhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_20_iyhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_53_iyhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_54_bc_de_ix_sp(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_ix_sp[i])) {
            cur_opcode |= i << 4;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool return_false(char *arg) {
    (void)arg;
    return false;
}

typedef bool match_handler_t(char *arg);

static match_handler_t *ind_match_handlers[] = {
    return_false,        // OD_AT_NONE
    return_false,        // OD_AT_IMM_0
    return_false,        // OD_AT_53_IMM
    return_false,        // OD_AT_43_IMODE
    return_false,        // OD_AT_53_RST
    return_false,        // OD_AT_IMM8
    match_imm8_ind,      // OD_AT_IMM8_IND
    return_false,        // OD_AT_IMM16
    match_imm16_ind,     // OD_AT_IMM16_IND
    return_false,        // OD_AT_REL_ADDR
    return_false,        // OD_AT_A
    return_false,        // OD_AT_R
    return_false,        // OD_AT_I
    return_false,        // OD_AT_DE
    return_false,        // OD_AT_HL
    return_false,        // OD_AT_SP
    return_false,        // OD_AT_AF
    return_false,        // OD_AT_AF_ALT
    match_c_ind,         // OD_AT_C_IND
    match_bc_ind,        // OD_AT_BC_IND
    match_de_ind,        // OD_AT_DE_IND
    match_hl_ind,        // OD_AT_HL_IND
    match_sp_ind,        // OD_AT_SP_IND
    match_20_reg_hl_ind, // OD_AT_20_REG_ALL
    return_false,        // OD_AT_20_BCDEHLA
    match_53_reg_hl_ind, // OD_AT_53_REG_ALL
    return_false,        // OD_AT_53_BCDEHLFA
    return_false,        // OD_AT_53_BCDEHLA
    return_false,        // OD_AT_53_COND
    return_false,        // OD_AT_43_COND
    return_false,        // OD_AT_54_BC_DE_HL_AF
    return_false,        // OD_AT_54_BC_DE_HL_SP
    return_false,        // OD_AT_IX
    match_ix_ind,        // OD_AT_IX_IND
    match_ix_ind_offs,   // OD_AT_IX_IND_OFFS
    return_false,        // OD_AT_20_IXHL
    return_false,        // OD_AT_53_IXHL
    return_false,        // OD_AT_20_IXHL_ALL
    return_false,        // OD_AT_53_IXHL_ALL
    return_false,        // OD_AT_54_BC_DE_IY_SP
    return_false,        // OD_AT_IY
    match_iy_ind,        // OD_AT_IY_IND
    match_iy_ind_offs,   // OD_AT_IY_IND_OFFS
    return_false,        // OD_AT_20_IYHL
    return_false,        // OD_AT_53_IYHL
    return_false,        // OD_AT_20_IYHL_ALL
    return_false,        // OD_AT_53_IYHL_ALL
    return_false,        // OD_AT_54_BC_DE_IX_SP
};

static match_handler_t *dir_match_handlers[] = {
    match_none,           // OD_AT_NONE
    match_imm_0,          // OD_AT_IMM_0
    match_53_imm,         // OD_AT_53_IMM
    match_43_imode,       // OD_AT_43_IMODE
    match_53_rst,         // OD_AT_53_RST
    match_imm8,           // OD_AT_IMM8
    return_false,         // OD_AT_IMM8_IND
    match_imm16,          // OD_AT_IMM16
    return_false,         // OD_AT_IMM16_IND
    match_rel_addr,       // OD_AT_REL_ADDR
    match_a,              // OD_AT_A
    match_r,              // OD_AT_R
    match_i,              // OD_AT_I
    match_de,             // OD_AT_DE
    match_hl,             // OD_AT_HL
    match_sp,             // OD_AT_SP
    match_af,             // OD_AT_AF
    match_af_alt,         // OD_AT_AF_ALT
    return_false,         // OD_AT_C_IND
    return_false,         // OD_AT_BC_IND
    return_false,         // OD_AT_DE_IND
    return_false,         // OD_AT_HL_IND
    return_false,         // OD_AT_SP_IND
    match_20_reg_all,     // OD_AT_20_REG_ALL
    match_20_bcdehla,     // OD_AT_20_BCDEHLA
    match_53_reg_all,     // OD_AT_53_REG_ALL
    match_53_bcdehlfa,    // OD_AT_53_BCDEHLFA
    match_53_bcdehla,     // OD_AT_53_BCDEHLA
    match_53_cond,        // OD_AT_53_COND
    match_43_cond,        // OD_AT_43_COND
    match_54_bc_de_hl_af, // OD_AT_54_BC_DE_HL_AF
    match_54_bc_de_hl_sp, // OD_AT_54_BC_DE_HL_SP
    match_ix,             // OD_AT_IX
    return_false,         // OD_AT_IX_IND
    return_false,         // OD_AT_IX_IND_OFFS
    match_20_ixhl,        // OD_AT_20_IXHL
    match_53_ixhl,        // OD_AT_53_IXHL
    match_20_ixhl_all,    // OD_AT_20_IXHL_ALL
    match_53_ixhl_all,    // OD_AT_53_IXHL_ALL
    match_54_bc_de_iy_sp, // OD_AT_54_BC_DE_IY_SP
    match_iy,             // OD_AT_IY
    return_false,         // OD_AT_IY_IND
    return_false,         // OD_AT_IY_IND_OFFS
    match_20_iyhl,        // OD_AT_20_IYHL
    match_53_iyhl,        // OD_AT_53_IYHL
    match_20_iyhl_all,    // OD_AT_20_IYHL_ALL
    match_53_iyhl_all,    // OD_AT_53_IYHL_ALL
    match_54_bc_de_ix_sp, // OD_AT_54_BC_DE_IX_SP
};

static bool match_argtype(char *arg, uint8_t arg_type) {
    // printf("    - match_argtype(\"%s\", %d)\n", arg, arg_type);
    if (arg[0] == 0 && arg_type != OD_AT_NONE)
        return false;
    if (arg[0] == '(') {
        return ind_match_handlers[arg_type](arg);
    } else {
        return dir_match_handlers[arg_type](arg);
    }
}

static void parse_file(const char *path) {
    stop_parsing = false;

    int8_t fd = esp_open(path, FO_RDONLY);
    check_esp_result(fd);

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

    while (!stop_parsing) {
#ifdef ENABLE_LISTING
        list_p  = list_line;
        *list_p = 0;
#endif

        cur_file_ctx->linenr++;
        int linelen = esp_readline(fd, linebuf, 256);
        if (linelen == ERR_EOF)
            break;
        check_esp_result(linelen);

        label      = NULL;
        keyword    = NULL;
        cur_p      = linebuf;
        uint8_t ch = *cur_p;
        if (ch == 0 || ch == ';' || ch == '\r' || ch == '\n')
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
            expect_end_of_line();
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
            char          *arg1      = parse_argument();
            char          *arg2      = parse_argument();
            expect_end_of_line();

            bool done = false;

#ifdef ENABLE_LISTING
            list_p += sprintf(list_p, "%04X  ", cur_addr);
#endif

            // printf("- %s:%u: %s %s%s%s\n", cur_file_ctx->path, cur_file_ctx->linenr, keyword, arg1, arg2[0] ? "," : "", arg2);

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
                // printf("  - arg1:%2u arg2:%2u prefix:%u opcode:$%02X\n", arg1_type, arg2_type, prefix_type, cur_opcode);

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
                syntax_error();
            }

#ifdef ENABLE_LISTING
            while (list_p - list_line < 14) {
                *(list_p++) = ' ';
            }
            *(list_p++) = '\t';
            list_p += sprintf(list_p, "    %s%s%s%s%s", keyword, arg1[0] ? " " : "", arg1, arg2[0] ? "," : "", arg2);
#endif
        }

#ifdef ENABLE_LISTING
        *(list_p++) = '\n';
        *list_p     = 0;
        fputs(list_line, f_list);
#endif
    }

    esp_close(fd);

    if (file_ctx_idx == 0) {
        cur_file_ctx = NULL;
    } else {
        cur_file_ctx = &file_ctxs[--file_ctx_idx];
    }
}

int main(void) {
#ifdef __SDCC
    scr_init();
#endif

#ifndef __SDCC
    // const char *path = "goaqms.asm";
    const char *path = "all_opcodes.asm";
#else
    const char *path = (const char *)0xFF00;
#endif

    puts("Aquarius+ Development Studio - Z80 Assembler V1.0 by Frank van den Hoef\n");
    printf("Assembling %s\n", path);

#ifndef __SDCC
    chdir("testdata");
#else
    esp_chdir("/testdata");
#endif

#ifdef ENABLE_LISTING
    f_list = fopen("listing.txt", "wt");
#endif
    fd_out = esp_open("result.bin", FO_CREATE | FO_TRUNC | FO_WRONLY);
    for (cur_pass = 0; cur_pass < 2; cur_pass++) {
        printf("- Pass %d\n", cur_pass + 1);
        cur_addr  = 0;
        cur_scope = 0;
        parse_file(path);
    }

#ifdef ENABLE_LISTING
    fclose(f_list);
#endif

    write_outbuf();
    esp_close(fd_out);

    puts("Done!\n");

    exit_program();
    return 0;
}
