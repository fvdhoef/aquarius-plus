#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "tokens.h"

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
static char           *p;
static uint8_t        *heap;

static void error(char *str) {
    if (str == NULL)
        str = "Unknown";
    printf("\n%s:%u Error: %s\n", cur_file_ctx->path, cur_file_ctx->linenr, str);
    exit(1);
}

static void skip_whitespace(void) {
    while (1) {
        uint8_t ch = *p;
        if (ch == ';' || ch == '\r' || ch == '\n') {
            *p = 0;
            break;
        }
        if (ch == 0 || ch > ' ')
            break;
        p++;
    }
}

static void parse_label(void) {
    label = p;
    while (1) {
        uint8_t ch = *p;
        if (ch <= ' ' || ch == ':')
            break;
        p++;
    }
    if (*p != 0)
        *(p++) = 0;
}

static uint8_t to_lower(uint8_t ch) {
    if (ch >= 'A' && ch <= 'Z')
        ch += 'a' - 'A';
    return ch;
}

static void parse_keyword(void) {
    skip_whitespace();
    keyword = p;

    while (1) {
        uint8_t ch = to_lower(*p);
        if (ch < 'a' || ch > 'z')
            break;
        p++;
    }

    if (keyword == p)
        keyword = NULL;
    else if (*p != 0) {
        if (*p > ' ')
            error("Syntax error");
        *(p++) = 0;
    }
}

static void parse_string(void) {
    string = NULL;
    skip_whitespace();
    uint8_t ch = *(p++);
    if (ch != '"')
        error("Expected '\"'");
    string = p;

    while (1) {
        ch = *p;
        if (ch == '"') {
            *(p++) = 0;
            break;
        }
        if (ch < ' ' && ch != '\t')
            error("Invalid string");
        p++;
    }
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
        int            cmp = memcmp(keyword, p, keyword_len);
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

static bool is_decimal(uint8_t ch) {
    return ch >= '0' && ch <= '9';
}

static bool is_hexadecimal(uint8_t ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
}

static uint16_t parse_expression(void);

static uint16_t parse_primary_expr(void) {
    skip_whitespace();

    uint8_t  ch    = *p;
    uint16_t value = 0;

    if (ch == '$') {
        // Hexadecimal value
        p++;
        ch = to_lower(*p);
        if (!is_hexadecimal(ch))
            error("Syntax error");

        while (is_hexadecimal(ch)) {
            value <<= 4;
            if (ch >= '0' && ch <= '9')
                value += ch - '0';
            else
                value += ch - 'a' + 10;

            p++;
            ch = to_lower(*p);
        }

    } else if (is_decimal(ch)) {
        // Decimal value
        while (is_decimal(ch)) {
            value *= 10;
            if (ch >= '0' && ch <= '9')
                value += ch - '0';

            p++;
            ch = to_lower(*p);
        }

    } else if (ch == '(') {
        p++;
        value = parse_expression();
        skip_whitespace();
        if (*(p++) != ')')
            error("Expected right parenthesis");

    } else {
        error("Expected primary expression");
    }
    return value;
}

// clang-format off
static uint16_t parse_unary_expr(void) {
    skip_whitespace();
    if      (p[0] == '-') { p++; return -parse_unary_expr(); }
    else if (p[0] == '+') { p++; return  parse_unary_expr(); }
    else if (p[0] == '~') { p++; return ~parse_unary_expr(); }
    else                  {      return  parse_primary_expr(); }
}

static uint16_t parse_mult_expr(void) {
    uint16_t val = parse_unary_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '*') { p++; val *= parse_unary_expr(); }
        else if (p[0] == '/') { p++; val /= parse_unary_expr(); }
        else if (p[0] == '%') { p++; val %= parse_unary_expr(); }
        else                  { break; }
    }
    return val;
}

static uint16_t parse_add_expr(void) {
    uint16_t val = parse_mult_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '+') { p++; val += parse_mult_expr(); }
        else if (p[0] == '-') { p++; val -= parse_mult_expr(); }
        else                  { break; }
    }
    return val;
}

static uint16_t parse_shift_expr(void) {
    uint16_t val = parse_add_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '<' && p[1] == '<') { p += 2; val <<= parse_add_expr(); }
        else if (p[0] == '>' && p[1] == '>') { p += 2; val >>= parse_add_expr(); }
        else                                 { break; }
    }
    return val;
}

static uint16_t parse_rel_expr(void) {
    uint16_t val = parse_shift_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '<' && p[1] != '<') { p++;  val = (val <  parse_shift_expr()); }
        else if (p[0] == '>' && p[1] != '>') { p++;  val = (val >  parse_shift_expr()); }
        else if (p[0] == '<' && p[1] == '=') { p+=2; val = (val <= parse_shift_expr()); }
        else if (p[0] == '>' && p[1] == '=') { p+=2; val = (val >= parse_shift_expr()); }
        else                                 { break; }
    }
    return val;
}

static uint16_t parse_eq_expr(void) {
    uint16_t val = parse_rel_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '=' && p[1] == '=') { p+=2; val = (val == parse_rel_expr()); }
        else if (p[0] == '!' && p[1] == '=') { p+=2; val = (val != parse_rel_expr()); }
        else                                 { break; }
    }
    return val;
}

static uint16_t parse_and_expr(void) {
    uint16_t val = parse_eq_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '&' && p[1] != '&') { p++; val &= parse_eq_expr(); }
        else                                 { break; }
    }
    return val;
}

static uint16_t parse_xor_expr(void) {
    uint16_t val = parse_and_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '^') { p++; val ^= parse_and_expr(); }
        else                  { break; }
    }
    return val;
}

static uint16_t parse_or_expr(void) {
    uint16_t val = parse_xor_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '|' && p[1] != '|') { p++; val ^= parse_xor_expr(); }
        else                                 { break; }
    }
    return val;
}

static uint16_t parse_logical_and_expr(void) {
    uint16_t val = parse_or_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '&' && p[1] == '&') { p++; val = val && parse_or_expr(); }
        else                                 { break; }
    }
    return val;
}

static uint16_t parse_logical_or_expr(void) {
    uint16_t val = parse_logical_and_expr();
    while (1) {
        skip_whitespace();
        if      (p[0] == '|' && p[1] == '|') { p++; val = val || parse_logical_and_expr(); }
        else                                 { break; }
    }
    return val;
}

// clang-format on

static uint16_t parse_expression(void) {
    return parse_logical_or_expr();
}

void parse_file(const char *path) {
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

    // FILE *f = fopen("testdata/regs.inc", "r");

    while (1) {
        cur_file_ctx->linenr++;
        if (fgets(linebuf, 256, f) == NULL)
            break;

        label      = NULL;
        keyword    = NULL;
        p          = linebuf;
        uint8_t ch = *p;
        if (ch == ';' || ch == '\r' || ch == '\n')
            continue;
        if (ch != ' ' && ch != '\t')
            parse_label();

        if (label) {
            printf("[Label: '%s']", label);
        }
        parse_keyword();
        if (keyword) {
            uint8_t token = tokenize_keyword(keyword);

            printf("[Keyword: '%s' token:%u]", keyword, token);
            if (token == TOK_UNKNOWN)
                error("Syntax error");

            if (token == TOK_INCLUDE) {
                parse_string();
                skip_whitespace();
                if (*p != 0)
                    error("Syntax error");

                printf("\nInclude file: '%s'\n", string);
                parse_file(string);
                printf("----------------------\n");
                continue;
            } else if (token == TOK_EQU) {
                if (label == NULL) {
                    error("Equ without label");
                }
                uint16_t val = parse_expression();
                printf("[equ %s = $%04x]\n", label, val);

                skip_whitespace();
                if (*p != 0)
                    error("Syntax error");
            } else if (token == TOK_ORG) {
                uint16_t addr = parse_expression();
                printf("[Org addr: $%04x]\n", addr);
            } else {
                error("Syntax error");
            }

        } else {
            if (*p != 0)
                error("Syntax error");
        }

        skip_whitespace();
        if (*p)
            printf("'%s'", p);
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
