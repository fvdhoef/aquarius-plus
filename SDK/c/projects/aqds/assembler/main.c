#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

static int         linenr = 0;
static char        linebuf[256];
static const char *label;
static const char *keyword;
static char       *p;

static void error(void) {
    printf("Error in line %d!\n", linenr);
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
            error();
        *(p++) = 0;
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

int main(void) {
    // FILE *f = fopen("testdata/regs.inc", "r");
    FILE *f = fopen("testdata/goaqms.asm", "r");

    while (1) {
        linenr++;
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
                error();
        } else {
            if (*p != 0)
                error();
        }

        skip_whitespace();
        if (*p)
            printf("'%s'", p);
    }
    return 0;
}
