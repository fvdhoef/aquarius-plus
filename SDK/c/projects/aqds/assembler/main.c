#include "common.h"
#include "tokens.h"
#include "expr.h"
#include "symbols.h"
#include "opcode_info.h"
#include "argmatch.h"

#ifdef __SDCC
#include "screen.h"
#else
#include <sys/stat.h>
#endif

#define MAX_FILE_DEPTH 8

struct file_ctx {
    uint16_t linenr;
    char     path[30];
};

static int8_t          fd_out  = -1;
static int8_t          fd_list = -1;
static char            list_line[16 + 257];
static uint8_t         list_byte;
static bool            list_print_text;
static bool            list_disable_bytes;
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
uint8_t                cur_pass   = 0;
uint16_t               cur_addr   = 0;
uint16_t               cur_scope  = 0;
uint8_t                cur_opcode = 0;
uint8_t                d_value;
static bool            stop_parsing;
static uint16_t        phase_offset;
static bool            phase_enabled = false;
static char            basename[32];
static const char     *filename_asm;

typedef void handler_t(void);

static void handler_unknown(void);
static void handler_defb(void);
static void handler_defs(void);
static void handler_defw(void);
static void handler_end(void);
static void handler_equ(void);
static void handler_incbin(void);
static void handler_include(void);
static void handler_org(void);
static void handler_phase(void);
static void handler_dephase(void);
static void handler_assert(void);

static handler_t *directive_handlers[TOK_DIR_LAST + 1] = {
    handler_unknown, // TOK_UNKNOWN
    handler_assert,  // TOK_ASSERT
    handler_defb,    // TOK_DB
    handler_defb,    // TOK_DEFB
    handler_defs,    // TOK_DEFS
    handler_defw,    // TOK_DEFW
    handler_dephase, // TOK_DEPHASE
    handler_defs,    // TOK_DS
    handler_defw,    // TOK_DW
    handler_end,     // TOK_END
    handler_equ,     // TOK_EQU
    handler_incbin,  // TOK_INCBIN
    handler_include, // TOK_INCLUDE
    handler_org,     // TOK_ORG
    handler_phase,   // TOK_PHASE
};

static void parse_file(const char *path);

#ifdef __SDCC
static char *get_pgm_arg(void) __naked { __asm__("jp 0xF80C"); }
#endif

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
    skip_whitespace();
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
    if (ch != '"' && ch != '\'')
        error("Expected \" or '");

    uint8_t term_ch = ch;

    string = cur_p;

    while (1) {
        ch = *cur_p;
        if (ch == term_ch) {
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

    // Binary search
    uint8_t        num_elems  = num_keywords[keyword_len - 2];
    const uint8_t *elems_base = keywords[keyword_len - 2];
    uint8_t        elem_size  = keyword_len + 1;

    for (uint8_t lim = num_elems; lim != 0; lim >>= 1) {
        const uint8_t *p   = elems_base + (lim >> 1) * elem_size;
        int            cmp = keyword_cmp((const uint8_t *)keyword, p, keyword_len);
        if (cmp == 0)
            return p[keyword_len];

        if (cmp > 0) {
            elems_base = p + elem_size;
            lim--;
        }
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

static void list_output_line(void) {
    if (fd_list < 0 || !list_line[0])
        return;

    // Terminate list line with newline
    int len          = strlen(list_line);
    list_line[len++] = '\n';
    list_line[len]   = 0;

    // Output line to file
    esp_write(fd_list, list_line, len);

    // Clear list line
    list_line[0]    = 0;
    list_byte       = 0;
    list_print_text = false;
}

static void write_hex(char *p, uint8_t val) {
    static char hex[] = "0123456789ABCDEF";
    p[0]              = hex[val >> 4];
    p[1]              = hex[val & 0xF];
}

static void list_write_addr(uint16_t val) {
    if (fd_list < 0)
        return;

    write_hex(list_line + 0, val >> 8);
    write_hex(list_line + 2, val);
}

static void emit_byte(uint16_t val) {
    if ((val & 0xFF00) != 0)
        error("Invalid byte value");

    if (fd_list >= 0 && !list_disable_bytes) {
        if (list_line[0] == 0) {
            memset(list_line, ' ', 6);
            memset(list_line + 6, 0, 10);
        }

        write_hex(list_line + 6 + list_byte * 2, val);
        if (++list_byte == 4) {
            list_output_line();
        }
    }
    if (fd_out >= 0) {
        *(outbuf_p++) = val;
        if (++outbuf_idx == sizeof(outbuf)) {
            write_outbuf();
        }
    }
    cur_addr++;
}

static void handler_unknown(void) {
    syntax_error();
}
static void handler_defb(void) {
    list_write_addr(cur_addr);
    skip_whitespace();

    while (1) {
        if (cur_p[0] == '"' || cur_p[0] == '\'') {
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
    list_write_addr(cur_addr);
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
    list_write_addr(cur_addr);
    uint16_t length    = parse_expression(false);
    list_disable_bytes = true;
    while (length--) {
        emit_byte(0);
    }
    list_disable_bytes = false;
}
static void handler_end(void) {
    expect_end_of_line();
    stop_parsing = true;
}
static void handler_equ(void) {
    if (!label)
        error("Equ without label");
    uint16_t value = parse_expression(false);
    symbol_add(label, 0, value);
    expect_end_of_line();
    list_write_addr(value);
}
static void handler_incbin(void) {
    list_write_addr(cur_addr);
    parse_string();
    expect_end_of_line();
    printf("  - Processing binary file: '%s'\n", string);

    int8_t fd = esp_open(string, FO_RDONLY);
    check_esp_result(fd);
    list_disable_bytes = true;
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
    list_disable_bytes = false;

    linebuf[0] = 0;
    cur_p      = linebuf;
}
static void handler_assert(void) {
    uint16_t value = parse_expression(false);
    expect_end_of_line();

    if (!value)
        error("Assertion failed!\n");
}

static void list_path(const char *path) {
    if (fd_list >= 0) {
        sprintf(list_line, "**** %s ****\n", path);
        esp_write(fd_list, list_line, strlen(list_line));
        list_line[0] = 0;
    }
}

static void handler_include(void) {
    parse_string();
    expect_end_of_line();

    printf("  - Processing include file: '%s'\n", string);
    list_output_line();
    list_path(string);
    parse_file(string);
    list_path(cur_file_ctx->path);
}

static void handler_org(void) {
    uint16_t addr = parse_expression(false);
    expect_end_of_line();

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

    list_write_addr(cur_addr);
    // printf("[Org addr: $%04x]\n", addr);
}
static void handler_phase(void) {
    if (phase_enabled)
        error("Already in phase");

    uint16_t addr = parse_expression(false);
    expect_end_of_line();

    phase_enabled = true;
    phase_offset  = cur_addr - addr;
    cur_addr      = addr;
    list_write_addr(cur_addr);
}
static void handler_dephase(void) {
    list_write_addr(cur_addr);
    expect_end_of_line();
    if (!phase_enabled)
        error("Dephase outside phase");

    phase_enabled = false;
    phase_offset  = cur_addr + phase_offset;
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

static void parse_line(void) {
    label           = NULL;
    keyword         = NULL;
    cur_p           = linebuf;
    list_print_text = true;

    // Early bail out if empty line
    if (cur_p[0] == 0 || cur_p[0] == ';' || cur_p[0] == '\r' || cur_p[0] == '\n')
        return;

    // Check for label
    if (cur_p[0] != ' ' && cur_p[0] != '\t')
        parse_label();

    // Check for keyword
    parse_keyword();

    // Tokenize keyword
    uint8_t token = TOK_UNKNOWN;
    if (keyword)
        token = tokenize_keyword(keyword);

    // First handle equ directive instead of creating normal label
    if (token == TOK_EQU) {
        directive_handlers[token]();
        return;
    }

    // Add symbol for label
    if (label) {
        symbol_add(label, 0, cur_addr);
        if (fd_list >= 0) {
            list_write_addr(cur_addr);
        }
    }

    // End of line?
    if (!keyword && cur_p[0] == 0)
        return;

    // Directive?
    if (token <= TOK_DIR_LAST) {
        directive_handlers[token]();
        expect_end_of_line();
        return;
    }

    // Parse arguments
    char *arg1 = parse_argument();
    char *arg2 = parse_argument();
    expect_end_of_line();

    // Iterate over opcode info entries to find a matching entry
    const uint8_t *info_next   = opcode_info[token - TOK_OPCODE_FIRST];
    uint8_t        prefix_type = 0;
    bool           found       = false;
    while (info_next) {
        // Extra information from current entry
        const uint8_t *info = info_next;
        info_next           = (info[0] & 0x80) ? (info + 3) : NULL;
        uint8_t arg1_type   = info[0] & 0x3F;
        uint8_t arg2_type   = info[1] & 0x3F;
        prefix_type         = info[1] >> 6;
        cur_opcode          = info[2];
        cur_outtype         = OT_NONE;

        // Check if arguments type matches
        if (match_argtype(arg1, arg1_type) && match_argtype(arg2, arg2_type)) {
            found = true;
            break;
        }
    }
    if (!found)
        syntax_error();

    list_write_addr(cur_addr);

    // Emit IX/IY specific prefix
    switch (cur_outtype & 0xC0) {
        case OT_PREFIX_DD: emit_byte(0xDD); break;
        case OT_PREFIX_FD: emit_byte(0xFD); break;
        default: break;
    }

    // Emit prefix
    switch (prefix_type) {
        case OD_PREFIX_CB: emit_byte(0xCB); break;
        case OD_PREFIX_ED: emit_byte(0xED); break;
        default: break;
    }

    // (IX+d)/(IY+d) specific encoding?
    if (prefix_type == OD_PREFIX_CB && (cur_outtype & OT_D)) {
        emit_byte(d_value);
        emit_byte(cur_opcode);
        return;
    }

    // Emit opcode
    emit_byte(cur_opcode);

    // Emit displacement
    if (cur_outtype & OT_D)
        emit_byte(d_value);

    // Emit argument
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

static void parse_file(const char *path) {
    // Open file and check for success
    int8_t fd = esp_open(path, FO_RDONLY);
    check_esp_result(fd);

    // Keep track of file context for error reporting (line number and file path)
    {
        if (cur_file_ctx != NULL) {
            file_ctx_idx++;
            if (file_ctx_idx >= MAX_FILE_DEPTH) {
                error("Max file depth reached");
            }
        }
        cur_file_ctx         = &file_ctxs[file_ctx_idx];
        cur_file_ctx->linenr = 0;

        // Copy path into file context
        char *p = cur_file_ctx->path;
        for (uint8_t i = 0; i < sizeof(cur_file_ctx->path) - 1; i++) {
            char ch = path[i];
            if (ch == 0)
                break;
            *(p++) = ch;
        }
        *p = 0;
    }

    // Parse file
    stop_parsing = false;
    while (!stop_parsing) {
        cur_file_ctx->linenr++;

        // Read one line of text
        int linelen = esp_readline(fd, linebuf, 256);
        if (linelen == ERR_EOF)
            break;
        check_esp_result(linelen);

        if (fd_list >= 0) {
            // Copy line into listing line
            memset(list_line, ' ', 16);
            strcpy(list_line + 16, linebuf);
        }

        // Parse the line
        parse_line();

        if (fd_list >= 0) {
            list_output_line();
        }
    }

    // Close file
    esp_close(fd);

    // Pop file context
    if (file_ctx_idx == 0) {
        cur_file_ctx = NULL;
    } else {
        cur_file_ctx = &file_ctxs[--file_ctx_idx];
    }
}

void determine_basename(const char *path) {
    filename_asm            = NULL;
    int         len         = strlen(path);
    const char *p           = path + len;
    const char *base_startp = NULL;
    const char *base_endp   = NULL;

    // Find start of extension
    while (p != path) {
        p--;
        if (*p == '/' || *p == '\\' || *p == '.')
            break;
    }
    if (p == path) {
        // No extension, no directories
        base_startp = path;
        base_endp   = path + len;
    } else if (*p == '/' || *p == '\\') {
        // No extension, has directories
        base_startp = p + 1;
        base_endp   = path + len;
    } else if (*p == '.') {
        base_endp = p;
        while (p != path) {
            p--;
            if (*p == '/' || *p == '\\') {
                base_startp = p + 1;
                break;
            }
        }
        if (!base_startp) {
            base_startp = path;
        }
    }

    filename_asm = base_startp;
    p            = base_startp;
    char *pd     = basename;
    for (int i = 0; i < (int)sizeof(basename) - 1; i++) {
        if (p < base_endp) {
            *(pd++) = *(p++);
        }
    }
    *pd = 0;

    p  = path;
    pd = linebuf;
    while (p < base_startp) {
        *(pd++) = *(p++);
    }
    *pd = 0;
}

int main(
#ifdef __SDCC
    void
#else
    int argc, const char **argv
#endif
) {
#ifdef __SDCC
    // Initialize screen output
    scr_init();
#endif
    const char *path;

#ifndef __SDCC
    // Get path from command line argument
    if (argc != 2) {
        printf("Syntax: assembler <inputfile.asm>\n");
        exit(1);
    }
    path = argv[1];
#else
    // Get path
    path = get_pgm_arg();
#endif

    // Determine base name (filename without extension) and path of assembler file (temporarily stored in linebuf)
    determine_basename(path);

    // Output header
    puts("Aquarius+ Development Studio - Z80 Assembler V1.1a by Frank van den Hoef\n");
    printf("Assembling %s\n", path);

    // Change directory
    if (*linebuf) {
        printf("- Changing directory to: %s\n", linebuf);
#ifndef __SDCC
        chdir(linebuf);
#else
        esp_chdir(linebuf);
#endif
    }

    // Perform the two assembly passes
    for (cur_pass = 0; cur_pass < 2; cur_pass++) {
        printf("- Assembly pass %d\n", cur_pass + 1);

        if (cur_pass == 1) {
            // Create output directory
            printf("  - Creating output directory: out\n");
#ifndef __SDCC
            mkdir("out", 0777);
#else
            esp_mkdir("out");
#endif

            // Open output binary
            sprintf(linebuf, "out/%s.aqx", basename);
            printf("  - Creating output binary: %s\n", linebuf);
            fd_out = esp_open(linebuf, FO_CREATE | FO_TRUNC | FO_WRONLY);
            check_esp_result(fd_out);

            // Open listing
            sprintf(linebuf, "out/%s.lst", basename);
            printf("  - Creating listing: %s\n", linebuf);
            fd_list = esp_open(linebuf, FO_CREATE | FO_TRUNC | FO_WRONLY);
            check_esp_result(fd_list);
        }

        cur_addr  = 0;
        cur_scope = 0;
        parse_file(filename_asm);
    }

    // Write any left over part to output binary and close it
    if (fd_out >= 0) {
        write_outbuf();
        esp_close(fd_out);
    }

    // Close listing file
    if (fd_list >= 0)
        esp_close(fd_list);

    // We're done, exit the program
    puts("Done!\n");
#ifdef __SDCC
    exit_program();
#endif
    return 0;
}
