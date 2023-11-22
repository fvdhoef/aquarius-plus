#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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

static void parse_keyword(void) {
    skip_whitespace();
    keyword = p;

    while (1) {
        uint8_t ch = *p;
        if (ch >= 'A' && ch <= 'Z')
            ch += 'a' - 'A';

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
