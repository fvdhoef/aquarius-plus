#include "tokenizer.h"

char           linebuf[256];
int            tok_value;
char           tok_strval[256];
static uint8_t cur_token;

static char *cur_p;

void error(const char *str) {
    if (str == NULL)
        str = "Unknown";
    if (cur_file_ctx)
        printf("\n%s:%u Error: %s\n", cur_file_ctx->path, cur_file_ctx->linenr, str);
    else
        printf("\nError: %s\n", str);
    exit_program();
}

void syntax_error(void) {
    error("Syntax error");
}

void eof_error(void) {
    error("Unexpected end-of-file");
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

static bool readline(bool output) {
    linebuf[0] = ';';
    linebuf[1] = ' ';

    int linelen = esp_readline(cur_file_ctx->fd, linebuf + 2, sizeof(linebuf) - 3);
    if (linelen == ERR_EOF)
        return false;
    check_esp_result(linelen);

    linebuf[2 + linelen]     = '\n';
    linebuf[2 + linelen + 1] = 0;

    cur_file_ctx->linenr++;
    cur_p = linebuf + 2;

    if (output)
        esp_write(fd_out, linebuf, 2 + linelen + 1);

    return true;
}

static bool nextline(void) {
    while (1) {
        if (!readline(true)) {
            if (pop_file())
                return false;
            continue;
        }

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
            push_file(path);

        } else if (strncmp(cur_p, "#asm", 4) == 0) {
            cur_p += 4;
            expect_eol();

            while (1) {
                if (!readline(false))
                    error("Expected #endasm");

                skip_whitespace();
                if (strncmp(cur_p, "#endasm", 7) == 0) {
                    cur_p += 7;
                    expect_eol();
                    esp_write(fd_out, linebuf, strlen(linebuf));
                    break;
                }

                // Write line verbatim to output
                esp_write(fd_out, linebuf + 2, strlen(linebuf + 2));
            }

        } else if (cur_p && cur_p[0] != 0) {
            break;
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

static uint8_t _get_token(void) {
    if (!cur_file_ctx)
        return TOK_EOF;

    while (1) {
        skip_whitespace();
        if (!cur_p || cur_p[0] == 0) {
            if (!nextline())
                return TOK_EOF;
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

            // clang-format off
            if (strcmp(tok_strval, "break") == 0) return TOK_BREAK;
            if (strcmp(tok_strval, "char") == 0) return TOK_CHAR;
            if (strcmp(tok_strval, "continue") == 0) return TOK_CONTINUE;
            if (strcmp(tok_strval, "else") == 0) return TOK_ELSE;
            if (strcmp(tok_strval, "goto") == 0) return TOK_GOTO;
            if (strcmp(tok_strval, "if") == 0) return TOK_IF;
            if (strcmp(tok_strval, "int") == 0) return TOK_INT;
            if (strcmp(tok_strval, "return") == 0) return TOK_RETURN;
            if (strcmp(tok_strval, "while") == 0) return TOK_WHILE;
            // clang-format on

            return TOK_IDENTIFIER;
        }

        // Other token
        else {
            return *(cur_p++);
        }
    }
}

void ack_token(void) {
    cur_token = 0;
}

uint8_t get_token(void) {
    if (cur_token == 0)
        cur_token = _get_token();
    return cur_token;
}
