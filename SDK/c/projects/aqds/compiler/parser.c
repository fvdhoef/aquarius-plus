#include "parser.h"
#include "tokenizer.h"
#include "expr.h"
#include "symbols.h"
#include <stdarg.h>

static uint16_t lbl_idx;

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
    emit("push    hl");
    emit_expr(node->right_node);
    emit("ex      de,hl");
    emit("pop     hl");
}

static void emit_local_lbl(int idx) {
    int len = sprintf(tmpbuf, ".%d:\n", idx);
    output_puts(tmpbuf, len);
}

static int gen_local_lbl(void) {
    return lbl_idx++;
}

static void emit_bitwise_and(struct expr_node *node) {
    emit_binary(node);
    emit("ld      a,h");
    emit("and     d");
    emit("ld      h,a");
    emit("ld      a,l");
    emit("and     e");
    emit("ld      l,a");
}

static void emit_bitwise_xor(struct expr_node *node) {
    emit_binary(node);
    emit("ld      a,h");
    emit("xor     d");
    emit("ld      h,a");
    emit("ld      a,l");
    emit("xor     e");
    emit("ld      l,a");
}

static void emit_bitwise_or(struct expr_node *node) {
    emit_binary(node);
    emit("ld      a,h");
    emit("or      d");
    emit("ld      h,a");
    emit("ld      a,l");
    emit("or      e");
    emit("ld      l,a");
}

static void emit_cast_boolean(void) {
    int lbl = gen_local_lbl();
    emit("ld      a,h");
    emit("or      l");
    emit("ld      hl,0");
    emit("jr      z,.%d", lbl);
    emit("inc     l");
    emit_local_lbl(lbl);
}

static void emit_expr(struct expr_node *node) {
    if (node->op == TOK_CONSTANT) {
        emit("ld      hl,%d", node->val);

    } else if (node->op == TOK_IDENTIFIER) {
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
    } else {
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
                // clang-format off
        case TOK_OP_NEG:
            emit_expr(node->left_node);
            emit("ex      de,hl");
            emit("ld      hl,0");
            emit("or      a");
            emit("sbc     hl,de");
            break;

        case '~':
            emit_expr(node->left_node);
            emit("ld      a,h");
            emit("xor     $FF");
            emit("ld      h,a");
            emit("ld      a,l");
            emit("xor     $FF");
            emit("ld      l,a");
            break;

        case '*':        emit_binary(node); emit("call    __multsi"); break;
        case '/':        emit_binary(node); emit("call    __divsi"); break;
        case '%':        emit_binary(node); emit("call    __modsi"); break;
        case '+':        emit_binary(node); emit("add     hl,de");  break;
        case '-':        emit_binary(node); emit("or      a"); emit("sbc     hl,de"); break;
        case TOK_OP_SHL: emit_binary(node); emit("call    __shl"); break;
        case TOK_OP_SHR: emit_binary(node); emit("call    __shr"); break;

        // case TOK_OP_LE:
        // case TOK_OP_GE:
        // case '<':
        // case '>':
        // case TOK_OP_EQ:
        // case TOK_OP_NE:

        case '&': emit_bitwise_and(node); break;
        case '^': emit_bitwise_xor(node); break;
        case '|': emit_bitwise_or(node); break;

        case TOK_OP_AND:
            emit_bitwise_and(node);
            emit_cast_boolean();
            break;

        case TOK_OP_OR:
            emit_bitwise_or(node);
            emit_cast_boolean();
            break;

                // clang-format on
            default:
                printf("Error: %d (%c)!\n", node->op, node->op);
                break;
        }
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
