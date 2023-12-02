#include "common.h"

#ifdef __SDCC
#include "screen.h"
#else
#include <sys/stat.h>
#endif

enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_CONSTANT,
    TOK_STRING_LITERAL,

    TOK_BREAK,    // break
    TOK_CHAR,     // char
    TOK_CONTINUE, // continue
    TOK_ELSE,     // else
    TOK_GOTO,     // goto
    TOK_IF,       // if
    TOK_INT,      // int
    TOK_RETURN,   // return
    TOK_WHILE,    // while
    TOK_OP_EQ,    // ==
    TOK_OP_NE,    // !=
    TOK_OP_GE,    // >=
    TOK_OP_LE,    // <=
    TOK_OP_SHL,   // <<
    TOK_OP_SHR,   // >>
};

#define MAX_FILE_DEPTH 8

struct file_ctx {
    uint16_t linenr;
    int8_t   fd;
    char     path[29];
};

static int8_t          fd_out = -1;
static struct file_ctx file_ctxs[MAX_FILE_DEPTH];
static uint8_t         file_ctx_idx;
struct file_ctx       *cur_file_ctx;
static char            linebuf[256];
char                  *cur_p;
static char            basename[32];
static const char     *filename_cb;
static int             tok_value;
static char            tok_strval[256];

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
    printf("\n%s:%u Error: %s\n", cur_file_ctx->path, cur_file_ctx->linenr, str);
    exit_program();
}

void syntax_error(void) {
    error("Syntax error");
}

void check_esp_result(int16_t result) {
    if (result < 0) {
        if (cur_file_ctx)
            printf("\n%s:%u Error: ", cur_file_ctx->path, cur_file_ctx->linenr);
        else
            printf("\nError: ");

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

static void skip_whitespace(void) {
    if (!cur_p)
        return;
    while (cur_p[0] == ' ' || cur_p[0] == '\t' || cur_p[0] == '\r' || cur_p[0] == '\n')
        cur_p++;
    if (cur_p[0] != 0 && cur_p[0] < ' ')
        error("Invalid character");
}

static void expect(uint8_t ch) {
    if (cur_p[0] != ch) {
        printf("\n%s:%u Error: expected %c\n", cur_file_ctx->path, cur_file_ctx->linenr, ch);
        exit_program();
    }
    cur_p++;
}

static void expect_eol(void) {
    skip_whitespace();
    if (cur_p[0] != 0)
        error("Expected end-of-line");
}

static bool readline(void) {
    int linelen = esp_readline(cur_file_ctx->fd, linebuf, sizeof(linebuf) - 1);
    if (linelen == ERR_EOF)
        return false;
    check_esp_result(linelen);

    linebuf[linelen]     = '\n';
    linebuf[linelen + 1] = 0;

    cur_file_ctx->linenr++;
    cur_p = linebuf;
    return true;
}

static bool nextline(void) {
    if (!readline())
        return false;

    skip_whitespace();

    // Check for directives
    if (strncmp(cur_p, "#include", 8) == 0) {
        cur_p += 8;
        skip_whitespace();
        expect('"');

        const char *path = cur_p;
        while (1) {
            uint8_t ch = *cur_p;
            if (ch == '"') {
                *(cur_p++) = 0;
                break;
            }
            if (ch < ' ' && ch != '\t')
                error("Invalid string");
            cur_p++;
        }
        expect_eol();

        // Process include file
        parse_file(path);

    } else if (strncmp(cur_p, "#asm", 4) == 0) {
        cur_p += 4;
        expect_eol();

        while (1) {
            if (!readline())
                error("Expected #endasm");

            skip_whitespace();
            if (strncmp(cur_p, "#endasm", 7) == 0) {
                cur_p += 7;
                expect_eol();
                break;
            }

            // Write line verbatim to output
            esp_write(fd_out, linebuf, strlen(linebuf));
        }
    }
    return true;
}

bool is_alpha(uint8_t ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool is_decimal(uint8_t ch) {
    return (ch >= '0' && ch <= '9');
}

bool is_hexadecimal(uint8_t ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

uint8_t esq_seq(void) {
    expect('\\');

    uint8_t value = 0;

    // clang-format off
    if      (cur_p[0] == 'n')  value = '\n';
    else if (cur_p[0] == 'r')  value = '\r';
    else if (cur_p[0] == 'b')  value = '\b';
    else if (cur_p[0] == '\\') value = '\\';
    else if (cur_p[0] == '\'') value = '\'';
    else if (cur_p[0] == '\"') value = '\"';
    else                       error("Invalid escape sequence");
    // clang-format on
    cur_p++;

    return value;
}

uint8_t get_token(void) {
    while (1) {
        skip_whitespace();
        while (!cur_p || cur_p[0] == 0) {
            if (!nextline())
                return TOK_EOF;
            skip_whitespace();
        }

        // Multi-line comment?
        if (cur_p[0] == '/' && cur_p[1] == '*') {
            cur_p += 2;
            while (1) {
                if (cur_p[0] == 0) {
                    if (!nextline())
                        error("Expected */");
                }
                if (cur_p[0] == '*' && cur_p[1] == '/') {
                    cur_p += 2;
                    break;
                }
                cur_p++;
            }
        }
        // clang-format off
        else if (cur_p[0] == '/' && cur_p[1] == '/') { cur_p = NULL; continue; }
        else if (cur_p[0] == '=' && cur_p[1] == '=') { cur_p += 2; return TOK_OP_EQ; }
        else if (cur_p[0] == '!' && cur_p[1] == '=') { cur_p += 2; return TOK_OP_NE; }
        else if (cur_p[0] == '>' && cur_p[1] == '=') { cur_p += 2; return TOK_OP_GE; }
        else if (cur_p[0] == '<' && cur_p[1] == '=') { cur_p += 2; return TOK_OP_LE; }
        else if (cur_p[0] == '<' && cur_p[1] == '<') { cur_p += 2; return TOK_OP_SHL; }
        else if (cur_p[0] == '>' && cur_p[1] == '>') { cur_p += 2; return TOK_OP_SHR; }
        // clang-format on

        // Hexadecimal constant?
        else if (cur_p[0] == '0' && cur_p[1] == 'x') {
            cur_p += 2;
            tok_value = 0;

            if (!is_hexadecimal(cur_p[0]))
                syntax_error();

            while (1) {
                uint8_t ch = *(cur_p++);
                if (ch >= '0' && ch <= '9') {
                    tok_value = (tok_value << 4) | (ch - '0');
                } else if (ch >= 'a' && ch <= 'f') {
                    tok_value = (tok_value << 4) | (ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    tok_value = (tok_value << 4) | (ch - 'A' + 10);
                } else {
                    break;
                }
            }
            return TOK_CONSTANT;
        }
        // Decimal constant?
        else if (is_decimal(cur_p[0])) {
            tok_value = 0;
            while (is_decimal(cur_p[0])) {
                tok_value = (tok_value * 10) + (cur_p[0] - '0');
                cur_p++;
            }
            return TOK_CONSTANT;
        }
        // Character constant?
        else if (cur_p[0] == '\'') {
            cur_p++;
            if (cur_p[0] == '\\') {
                tok_value = esq_seq();
            } else if (cur_p[0] < ' ' && cur_p[0] != '\t') {
                error("Invalid character");
            } else {
                tok_value = *(cur_p++);
            }
            expect('\'');
            return TOK_CONSTANT;
        }
        // String literal?
        else if (cur_p[0] == '\"') {
            cur_p++;
            char *p = tok_strval;
            while (1) {
                if (cur_p[0] == 0)
                    error("Unexpected end-of-line");
                if (cur_p[0] == '"') {
                    cur_p++;
                    break;
                }
                if (cur_p[0] == '\\') {
                    *(p++) = esq_seq();
                } else {
                    *(p++) = *(cur_p++);
                }
            }
            *p = 0;
            return TOK_STRING_LITERAL;
        }
        // Identifier?
        else if (cur_p[0] == '_' || is_alpha(cur_p[0])) {
            char *p = tok_strval;
            for (int i = 0; i < (int)sizeof(tok_strval) - 1; i++) {
                if (cur_p[0] == '_' || is_alpha(cur_p[0]) || is_decimal(cur_p[0])) {
                    *(p++) = *(cur_p++);
                } else {
                    break;
                }
            }
            if (cur_p[0] == '_' || is_alpha(cur_p[0] || is_decimal(cur_p[0]))) {
                error("Identifier too long!");
            }
            *p = 0;
            return TOK_IDENTIFIER;
        }
        return *(cur_p++);
    }
}

static void parse_file(const char *path) {
    printf("- Parsing %s\n", path);

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
        cur_file_ctx->fd     = fd;

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

    while (1) {
        int token = get_token();
        if (token == TOK_EOF)
            break;

        if (token == TOK_IDENTIFIER) {
            printf("  - Identifier: %s\n", tok_strval);
        } else if (token == TOK_CONSTANT) {
            printf("  - Constant: %d\n", tok_value);
        } else if (token == TOK_STRING_LITERAL) {
            printf("  - String literal: %s\n", tok_strval);
        } else {
            printf("  - Token: %d %c\n", token, token > ' ' ? token : ' ');
        }
    }

    // Close file
    esp_close(fd);

    printf("- Parsing %s done\n", cur_file_ctx->path);

    // Pop file context
    if (file_ctx_idx == 0) {
        cur_file_ctx = NULL;
    } else {
        cur_file_ctx = &file_ctxs[--file_ctx_idx];
    }
}

void determine_basename(const char *path) {
    filename_cb             = NULL;
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

    filename_cb = base_startp;
    p           = base_startp;
    char *pd    = basename;
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

    puts("Aquarius+ Development Studio - C-flat (Cb) Compiler V1.0 by Frank van den Hoef\n");

#ifndef __SDCC
    // Get path from command line argument
    if (argc != 2) {
        printf("Syntax: compiler <inputfile.cb>\n");
        exit(1);
    }
    path = argv[1];
#else
    // Path is located in buffer at $FF00
    path = (const char *)0xFF00;
#endif

    // Determine base name (filename without extension) and path of assembler file (temporarily stored in linebuf)
    determine_basename(path);

    // Output header
    printf("Compiling %s\n", path);

    // Change directory
    if (*linebuf) {
        printf("- Changing directory to: %s\n", linebuf);
#ifndef __SDCC
        chdir(linebuf);
#else
        esp_chdir(linebuf);
#endif
    }

    // Create output directory
    printf("- Creating output directory: out\n");
#ifndef __SDCC
    mkdir("out", 0777);
#else
    esp_mkdir("out");
#endif

    // Open assembly output
    sprintf(linebuf, "out/%s.asm", basename);
    printf("- Creating assembly output: %s\n", linebuf);
    fd_out = esp_open(linebuf, FO_CREATE | FO_TRUNC | FO_WRONLY);
    check_esp_result(fd_out);

    // Parse source code
    parse_file(filename_cb);

    // Close files
    esp_close(fd_out);

    // We're done, exit the program
    puts("Done!\n");
    exit_program();
    return 0;
}
