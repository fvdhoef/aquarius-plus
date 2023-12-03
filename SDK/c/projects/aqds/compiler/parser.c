#include "parser.h"
#include "tokenizer.h"
#include "expr.h"
#include "symbols.h"
#include <stdarg.h>

static void emit_expr(struct expr_node *node);

static void expect_ack(uint8_t token) {
    if (get_token() != token)
        syntax_error();
    ack_token();
}

static void expect(uint8_t token) {
    if (get_token() != token)
        syntax_error();
}

static void emit(char *fmt, ...) {
    tmpbuf[0] = ' ';
    tmpbuf[1] = ' ';
    tmpbuf[2] = ' ';
    tmpbuf[3] = ' ';

    va_list ap;
    va_start(ap, fmt);
    int len = vsprintf(tmpbuf + 4, fmt, ap);
    va_end(ap);

    tmpbuf[4 + len]     = '\n';
    tmpbuf[4 + len + 1] = 0;

    output_puts(tmpbuf, 4 + len + 1);
}

static void emit_binary(struct expr_node *node) {
    emit_expr(node->left_node);
    emit("ex      de,hl");
    emit_expr(node->right_node);
}

static void emit_expr(struct expr_node *node) {
    switch (node->op) {
        case TOK_CONSTANT: {
            emit("ld      hl,%d", node->val);
            break;
        }
        case TOK_IDENTIFIER: {
            struct symbol *sym = node->sym;
            if (sym->type == (SYMTYPE_GLOBAL | SYMTYPE_VAR_CHAR)) {
                emit("ld      a,(_%s)", sym->name);
                emit("ld      h,0");
                emit("ld      l,a");

            } else if (sym->type == (SYMTYPE_GLOBAL | SYMTYPE_VAR_INT)) {
                emit("ld      hl,(_%s)", sym->name);

            } else {
                printf("Unimplemented identifier symbol type in expression!\n");
                syntax_error();
            }

            break;
        }
        case '+':
            emit_binary(node);
            emit("add     hl,de");
            break;
        case '-':
            emit_binary(node);
            emit("or      a");
            emit("sbc     hl,de");
            break;
        default:
            printf("Error: %d (%c)!\n", node->op, node->op);
            break;
    }
}

static void parse_compound(void) {
    expect_ack('{');

    while (1) {
        int token = get_token();
        ack_token();

        if (token == '}') {
            // End of compound statement
            ack_token();
            break;

        } else if (token == TOK_IDENTIFIER) {
            token = get_token();
            if (token == '(') {
                ack_token();
                printf("Function call: %s\n", tok_strval);
                expect_ack(')');
                expect_ack(';');
                emit("call    _%s", tok_strval);

            } else if (token == '=') {
                ack_token();

                struct symbol *sym = symbol_get(tok_strval, 0, false);

                printf("Variable assignment: %s\n", tok_strval);

                struct expr_node *node = parse_expression();
                emit_expr(node);

                if (sym->type == (SYMTYPE_GLOBAL | SYMTYPE_VAR_CHAR)) {
                    emit("ld      a,l");
                    emit("ld      (_%s),a", sym->name);

                } else if (sym->type == (SYMTYPE_GLOBAL | SYMTYPE_VAR_INT)) {
                    emit("ld      (_%s),hl", sym->name);

                } else {
                    printf("Unimplemented identifier symbol type in expression!\n");
                    syntax_error();
                }

                expect_ack(';');

            } else {
                syntax_error();
            }

        } else {
            syntax_error();
        }
    }
}

void parse(void) {
    while (1) {
        int token = get_token();
        if (token == TOK_EOF)
            break;
        ack_token();

        if (token == TOK_IDENTIFIER) {
            // Function definition
            expect_ack('(');
            expect_ack(')');

            printf("  - Function: %s\n", tok_strval);

            sprintf(tmpbuf, "_%s:\n", tok_strval);
            output_puts(tmpbuf, 0);

            expect('{');
            parse_compound();

            output_puts(".func_exit:\n", 0);
            output_puts("    ret\n", 0);

        } else if (token == TOK_CHAR || token == TOK_INT) {
            uint8_t type = token;
            ack_token();
            expect(TOK_IDENTIFIER);
            printf("  - Variable: %s  (type: %d)\n", tok_strval, type);

            uint8_t symtype = SYMTYPE_GLOBAL | ((token == TOK_CHAR) ? SYMTYPE_VAR_CHAR : SYMTYPE_VAR_INT);
            symbol_add(symtype, tok_strval, 0);

            sprintf(tmpbuf, "_%s:\n", tok_strval);
            output_puts(tmpbuf, 0);
            ack_token();

            int value = 0;

            token = get_token();
            if (token == '=') {
                ack_token();
                struct expr_node *node = parse_expression();
                if (!node || node->op != TOK_CONSTANT)
                    syntax_error();
                value = node->val;
            }

            if (type == TOK_CHAR) {
                sprintf(tmpbuf, "    defb %d\n", value);
            } else {
                sprintf(tmpbuf, "    defw %d\n", value);
            }
            output_puts(tmpbuf, 0);

            expect_ack(';');

        } else {
            syntax_error();
        }
    }
}
