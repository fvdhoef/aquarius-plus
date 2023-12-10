#include "parser.h"
#include "tokenizer.h"
#include "expr.h"
#include "symbols.h"
#include <stdarg.h>

static uint16_t lbl_idx;
static uint16_t flags;
static uint8_t  arg_count;
static int      cur_ix_offset;
static uint8_t  ptr_typespec;

// Flags to indicate which helper functions to generate
#define FLAGS_USES_MULTSI  (1 << 0)
#define FLAGS_USES_DIVSI   (1 << 1)
#define FLAGS_USES_MODSI   (1 << 2)
#define FLAGS_USES_SHL     (1 << 3)
#define FLAGS_USES_SHR     (1 << 4)
#define FLAGS_USES_CALL_HL (1 << 5)

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
    int len = sprintf(tmpbuf, ".l%d:\n", idx);
    output_puts(tmpbuf, len);
}

static void emit_lbl(const char *str) {
    int len = sprintf(tmpbuf, "_%s:\n", str);
    output_puts(tmpbuf, len);
}

static int gen_local_lbl(void) {
    return lbl_idx++;
}

static void emit_expr(struct expr_node *node) {
    switch (node->op) {
        case TOK_CONSTANT: {
            emit("ld      hl,%d", node->val);
            break;
        }

        case TOK_IDENTIFIER: {
            struct symbol *sym = node->sym;
            if (sym->symtype == SYM_SYMTYPE_PTR) {
                ptr_typespec = sym->typespec;
            }
            if (sym->storage == SYM_STORAGE_STATIC) {
                if (sym->typespec == SYM_TYPESPEC_INT || sym->symtype != SYM_SYMTYPE_VAR) {
                    emit("ld      hl,(_%s)", sym->name);
                } else if (sym->typespec == SYM_TYPESPEC_CHAR) {
                    emit("ld      a,(_%s)", sym->name);
                    emit("ld      h,0");
                    emit("ld      l,a");
                } else {
                    printf("Unimplemented global symbol type in expression!\n");
                    syntax_error();
                }

            } else if (sym->storage == SYM_STORAGE_STACK) {
                if (sym->typespec == SYM_TYPESPEC_INT || sym->symtype != SYM_SYMTYPE_VAR) {
                    emit("ld      l,(ix+%d)", sym->value);
                    emit("ld      h,(ix+%d)", sym->value + 1);

                } else if (sym->typespec == SYM_TYPESPEC_CHAR) {
                    emit("ld      h,0");
                    emit("ld      l,(ix+%d)", sym->value);

                } else {
                    printf("Unimplemented local symbol type in expression!\n");
                    syntax_error();
                }
            } else {
                error("Invalid storage class");
            }
            break;
        }

        case '=': {
            emit_expr(node->right_node);

            if (node->left_node->op == TOK_IDENTIFIER) {
                if (node->left_node->sym->storage == SYM_STORAGE_STATIC) {
                    if (node->left_node->sym->typespec == SYM_TYPESPEC_INT || node->left_node->sym->symtype == SYM_SYMTYPE_PTR) {
                        emit("ld      (_%s),hl", node->left_node->sym->name);

                    } else if (node->left_node->sym->typespec == SYM_TYPESPEC_CHAR) {
                        emit("ld      a,l");
                        emit("ld      (_%s),a", node->left_node->sym->name);

                    } else {
                        printf("Unimplemented identifier symbol type in expression!\n");
                        syntax_error();
                    }

                } else {
                    printf("Unimplemented identifier symbol type in LHS assignment!\n");
                    syntax_error();
                }

            } else {
                emit("push    hl");

                if (node->left_node->op == TOK_DEREF) {
                    ptr_typespec = SYM_TYPESPEC_UNDEFINED;
                    emit_expr(node->left_node->left_node);

                    if (ptr_typespec == SYM_TYPESPEC_CHAR) {
                        emit("pop     de");
                        emit("ld      (hl),e");
                        emit("ex      de,hl");

                    } else if (ptr_typespec == SYM_TYPESPEC_INT) {
                        emit("pop     de");
                        emit("ld      (hl),e");
                        emit("inc     hl");
                        emit("ld      (hl),d");
                        emit("ex      de,hl");

                    } else {
                        error("Deref non-pointer");
                    }

                } else {
                    error("Unimplemented assignment");
                }
            }
            break;
        }

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

        case '*':
            emit_binary(node);
            emit("call    __multsi");
            flags |= FLAGS_USES_MULTSI;
            break;

        case '/':
            emit_binary(node);
            emit("call    __divsi");
            flags |= FLAGS_USES_DIVSI;
            break;

        case '%':
            emit_binary(node);
            emit("call    __modsi");
            flags |= FLAGS_USES_MODSI;
            break;

        case '+':
            emit_binary(node);
            emit("add     hl,de");
            break;

        case '-':
            emit_binary(node);
            emit("or      a");
            emit("sbc     hl,de");
            break;

        case TOK_OP_SHL:
            emit_binary(node);
            emit("call    __shl");
            flags |= FLAGS_USES_SHL;
            break;

        case TOK_OP_SHR:
            emit_binary(node);
            emit("call    __shr");
            flags |= FLAGS_USES_SHR;
            break;

            // case TOK_OP_LE:
            // case TOK_OP_GE:
            // case '<':
            // case '>':
            // case TOK_OP_EQ:
            // case TOK_OP_NE:

        case '&':
            emit_binary(node);
            emit("ld      a,h");
            emit("and     d");
            emit("ld      h,a");
            emit("ld      a,l");
            emit("and     e");
            emit("ld      l,a");
            break;

        case '^':
            emit_binary(node);
            emit("ld      a,h");
            emit("xor     d");
            emit("ld      h,a");
            emit("ld      a,l");
            emit("xor     e");
            emit("ld      l,a");
            break;

        case '|':
            emit_binary(node);
            emit("ld      a,h");
            emit("or      d");
            emit("ld      h,a");
            emit("ld      a,l");
            emit("or      e");
            emit("ld      l,a");
            break;

        case TOK_OP_AND: {
            // Boolean-AND operation with short circuit
            int lbl1 = gen_local_lbl();
            int lbl2 = gen_local_lbl();
            emit_expr(node->left_node);
            emit("ld      a,h");
            emit("or      l");
            emit("jr      z,.l%d", lbl1);
            emit_expr(node->right_node);
            emit("ld      a,h");
            emit("or      l");
            emit_local_lbl(lbl1);
            emit("ld      hl,0");
            emit("jr      z,.l%d", lbl2);
            emit("inc     l");
            emit_local_lbl(lbl2);
            break;
        }

        case TOK_OP_OR: {
            // Boolean-OR operation with short circuit
            int lbl1 = gen_local_lbl();
            int lbl2 = gen_local_lbl();
            emit_expr(node->left_node);
            emit("ld      a,h");
            emit("or      l");
            emit("jr      nz,.l%d", lbl1);
            emit_expr(node->right_node);
            emit("ld      a,h");
            emit("or      l");
            emit_local_lbl(lbl1);
            emit("ld      hl,1");
            emit("jr      nz,.l%d", lbl2);
            emit("dec     l");
            emit_local_lbl(lbl2);
            break;
        }

        case TOK_FUNC_ARG: {
            // Argument should be pushed in reversed order
            if (node->right_node)
                emit_expr(node->right_node);
            emit_expr(node->left_node);
            emit("push    hl");
            arg_count++;
            break;
        }

        case TOK_FUNC_CALL: {
            arg_count = 0;
            if (node->right_node)
                emit_expr(node->right_node);

            uint8_t count = arg_count;

            if (node->left_node->op == TOK_IDENTIFIER) {
                // Directly call a label
                emit("call    _%s", node->left_node->sym->name);
            } else {
                // Indirect call
                emit_expr(node->left_node);
                emit("call    __call_hl");
                flags |= FLAGS_USES_CALL_HL;
            }
            while (count--) {
                // Clean up stack
                emit("pop     af");
            }
            break;
        }

        default:
            printf("Error: op %d (%c)!\n", node->op, node->op > ' ' ? node->op : '?');
            syntax_error();
            break;
    }
}

static void parse_compound(bool new_scope, int lbl_continue, int lbl_break);

static void parse_statement(int lbl_continue, int lbl_break) {
    int token = get_token();
    if (token == '{') {
        parse_compound(true, lbl_continue, lbl_break);

    } else if (token == TOK_IF) {
        ack_token();
        expect_ack('(');
        struct expr_node *node = parse_expression();
        expect_ack(')');
        emit_expr(node);
        emit("ld      a,h");
        emit("or      l");
        int lbl1 = gen_local_lbl();
        emit("jp      z,.l%d", lbl1);
        parse_statement(lbl_continue, lbl_break);

        bool has_else = false;
        if (get_token() == TOK_ELSE) {
            ack_token();
            has_else = true;
        }

        if (has_else) {
            int lbl2 = gen_local_lbl();
            emit("jp      .l%d", lbl2);
            emit_local_lbl(lbl1);
            parse_statement(lbl_continue, lbl_break);
            emit_local_lbl(lbl2);
        } else {
            emit_local_lbl(lbl1);
        }

    } else if (token == TOK_WHILE) {
        ack_token();
        expect_ack('(');
        struct expr_node *node = parse_expression();
        expect_ack(')');

        int lbl1 = gen_local_lbl();
        int lbl2 = gen_local_lbl();
        emit_local_lbl(lbl1);

        emit_expr(node);
        emit("ld      a,h");
        emit("or      l");
        emit("jp      z,.l%d", lbl2);

        parse_statement(lbl1, lbl2);
        emit_local_lbl(lbl2);

    } else if (token == TOK_CONTINUE) {
        ack_token();
        if (lbl_continue < 0) {
            error("Continue outside loop");
        }
        emit("jp      .l%d", lbl_continue);
        expect_ack(';');

    } else if (token == TOK_BREAK) {
        ack_token();
        if (lbl_break < 0) {
            error("Break outside loop");
        }
        emit("jp      .l%d", lbl_break);
        expect_ack(';');

    } else if (token == TOK_RETURN) {
        ack_token();
        printf("Return!\n");
        token = get_token();
        if (token != ';') {
            struct expr_node *node = parse_expression();
            emit_expr(node);
        }
        emit("jp      .return");
        expect_ack(';');

    } else {
        struct expr_node *node = parse_expression();
        emit_expr(node);
        expect_ack(';');
    }
}

static void parse_compound(bool new_scope, int lbl_continue, int lbl_break) {
    if (new_scope)
        symbol_push_scope();
    expect_ack('{');

    while (1) {
        int token = get_token();
        if (token == '}') {
            // End of compound statement
            ack_token();
            break;

        } else if (token == TOK_CHAR || token == TOK_INT) {
            uint8_t type = token;
            ack_token();
            expect(TOK_IDENTIFIER);
            printf("  - Variable: %s  (type: %d)\n", tok_strval, type);

            cur_ix_offset -= 2;

            struct symbol *sym = symbol_add(tok_strval, 0);

            sym->symtype  = SYM_SYMTYPE_VAR;
            sym->typespec = (token == TOK_CHAR) ? SYM_TYPESPEC_CHAR : SYM_TYPESPEC_INT;
            sym->storage  = SYM_STORAGE_STACK;
            sym->value    = cur_ix_offset;

            ack_token();

            token = get_token();
            if (token == '=') {
                ack_token();
                struct expr_node *node = parse_expression();
                emit_expr(node);
                emit("push    hl");
            } else {
                emit("push    af");
            }
            expect_ack(';');

        } else {
            parse_statement(lbl_continue, lbl_break);
        }
    }
    if (new_scope)
        symbol_pop_scope();
}

void parse(void) {
    while (1) {
        int token = get_token();
        if (token == TOK_EOF)
            break;

        // Function definition?
        if (token == TOK_IDENTIFIER) {
            ack_token();

            // Function definition
            expect_ack('(');
            printf("- Function: %s\n", tok_strval);
            {
                struct symbol *sym = symbol_add(tok_strval, 0);
                sym->symtype       = SYM_SYMTYPE_FUNC;
                sym->storage       = SYM_STORAGE_STATIC;
            }
            emit_lbl(tok_strval);

            symbol_push_scope();

            int offset = 4; // Return address and pushed IX

            while (1) {
                token = get_token();
                if (token == TOK_CHAR || token == TOK_INT) {
                    uint8_t type = token;
                    ack_token();
                    expect(TOK_IDENTIFIER);
                    printf("  - Argument: %s  (type: %d)\n", tok_strval, type);

                    struct symbol *sym = symbol_add(tok_strval, 0);
                    sym->symtype       = SYM_SYMTYPE_VAR;
                    sym->typespec      = (token == TOK_CHAR) ? SYM_TYPESPEC_CHAR : SYM_TYPESPEC_INT;
                    sym->storage       = SYM_STORAGE_STACK;
                    sym->value         = offset;
                    offset += 2;

                    ack_token();
                }

                token = get_token();
                if (token == ')') {
                    ack_token();
                    break;
                }
                if (token == ',') {
                    ack_token();
                } else {
                    error("Expected comma");
                }
            }

            emit("push    ix");
            emit("ld      ix,0");
            emit("add     ix,sp");

            expect('{');

            cur_ix_offset = 0;
            parse_compound(false, -1, -1);

            output_puts(".return:\n", 0);
            emit("ld      sp,ix");
            emit("pop     ix");
            emit("ret");

            symbol_pop_scope();

        }

        // Variable definition?
        else if (token == TOK_CHAR || token == TOK_INT) {
            uint8_t tok_type = token;
            uint8_t is_ptr   = 0;
            ack_token();

            // Pointer?
            if (get_token() == '*') {
                is_ptr = 1;
                ack_token();
            }

            expect(TOK_IDENTIFIER);
            struct symbol *sym = symbol_add(tok_strval, 0);
            sym->symtype       = is_ptr ? SYM_SYMTYPE_PTR : SYM_SYMTYPE_VAR;
            sym->typespec      = tok_type == TOK_CHAR ? SYM_TYPESPEC_CHAR : SYM_TYPESPEC_INT;
            sym->storage       = SYM_STORAGE_STATIC;
            symbol_dump(sym);

            sprintf(tmpbuf, "_%s:\n", tok_strval);
            output_puts(tmpbuf, 0);
            ack_token();

            int value = 0;
            if (get_token() == '=') {
                ack_token();
                struct expr_node *node = parse_expression();
                if (!node || node->op != TOK_CONSTANT)
                    syntax_error();
                value = node->val;
            }

            if (is_ptr || sym->typespec == SYM_TYPESPEC_INT) {
                sprintf(tmpbuf, "    defw %d\n", value);
            } else {
                sprintf(tmpbuf, "    defb %d\n", value);
            }
            output_puts(tmpbuf, 0);

            expect_ack(';');

        }

        // Syntax error
        else {
            syntax_error();
        }
    }

    emit("\n; --- Support functions ---");
    if (flags & FLAGS_USES_MULTSI) {
        emit_lbl("_multsi");
        emit("; To be implemented");
        emit("ret");
    }
    if (flags & FLAGS_USES_DIVSI) {
        emit_lbl("_divsi");
        emit("; To be implemented");
        emit("ret");
    }
    if (flags & FLAGS_USES_MODSI) {
        emit_lbl("_modsi");
        emit("; To be implemented");
        emit("ret");
    }
    if (flags & FLAGS_USES_SHL) {
        emit_lbl("_shl");
        emit("; To be implemented");
        emit("ret");
    }
    if (flags & FLAGS_USES_SHR) {
        emit_lbl("_shr");
        emit("; To be implemented");
        emit("ret");
    }
    if (flags & FLAGS_USES_CALL_HL) {
        emit_lbl("_call_hl");
        emit("jp      (hl)");
    }
}
