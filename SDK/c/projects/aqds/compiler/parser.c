#include "parser.h"
#include "tokenizer.h"
#include "expr.h"
#include "symbols.h"
#include <stdarg.h>

static uint16_t lbl_idx;
static uint16_t flags;
static uint8_t  arg_count;
static int      cur_ix_offset;

// Flags to indicate which helper functions to generate
#define FLAGS_USES_MULTSI  (1 << 0)
#define FLAGS_USES_DIVSI   (1 << 1)
#define FLAGS_USES_MODSI   (1 << 2)
#define FLAGS_USES_SHL     (1 << 3)
#define FLAGS_USES_SHR     (1 << 4)
#define FLAGS_USES_CALL_HL (1 << 5)
#define FLAGS_USES_LTU     (1 << 6)
#define FLAGS_USES_LEU     (1 << 7)
#define FLAGS_USES_GTU     (1 << 8)
#define FLAGS_USES_GEU     (1 << 9)
#define FLAGS_USES_LTS     (1 << 10)
#define FLAGS_USES_LES     (1 << 11)
#define FLAGS_USES_GTS     (1 << 12)
#define FLAGS_USES_GES     (1 << 13)

static void emit_expr(struct expr_node *node);
static int  emit_str_bytes(char *str, uint16_t buf_len);

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

static int gen_lbl_idx(void) {
    return lbl_idx++;
}

static void emit_16b_sub(struct expr_node *node) {
    emit_binary(node);
    emit("or      a");
    emit("sbc     hl,de");
}

static void emit_expr(struct expr_node *node) {
    switch (node->op) {
        case TOK_CONSTANT: {
            emit("ld      hl,%d", node->val);
            break;
        }

        case TOK_IDENTIFIER: {
            struct symbol *sym = node->sym;
            if (sym->storage == SYM_STORAGE_STATIC) {
                if (sym->symtype == SYM_SYMTYPE_ARRAY) {
                    emit("ld      hl,_%s", sym->name);
                } else if (sym->typespec == SYM_TYPESPEC_INT || sym->symtype != SYM_SYMTYPE_VAR) {
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

        case TOK_STRING_LITERAL: {
            emit("ld      hl,__str%d", node->str->idx);
            break;
        }

        case TOK_DEREF: {
            if (node->left_node->symtype != SYM_SYMTYPE_PTR)
                error("Deref non-pointer");

            emit_expr(node->left_node);
            if (node->left_node->typespec == SYM_TYPESPEC_CHAR) {
                emit("ld      a,(hl)");
                emit("ld      l,a");
                emit("ld      h,0");
            } else {
                emit("ld      e,(hl)");
                emit("inc     hl");
                emit("ld      d,(hl)");
                emit("ex      de,hl");
            }
            break;
        }

        case TOK_ADDR_OF: {
            if (node->left_node->symtype == SYM_SYMTYPE_VAR) {
                if (node->left_node->sym->storage == SYM_STORAGE_STATIC) {
                    emit("ld      hl,_%s", node->left_node->sym->name);

                } else if (node->left_node->sym->storage == SYM_STORAGE_STACK) {
                    emit("ld      hl,%d", node->left_node->sym->value);
                    emit("ld      d,ixh");
                    emit("ld      e,ixl");
                    emit("add     hl,de");

                } else {
                    syntax_error();
                }

            } else {
                error("Illegal addr of");
            }
            break;
        }

        case '=': {
            emit_expr(node->right_node);

            // Handle left hand side of assignment

            if (node->left_node->op == TOK_IDENTIFIER) {
                if (node->left_node->sym->storage == SYM_STORAGE_STATIC) {
                    if (node->left_node->sym->symtype == SYM_SYMTYPE_ARRAY)
                        syntax_error();

                    if (node->left_node->sym->typespec == SYM_TYPESPEC_INT || node->left_node->sym->symtype == SYM_SYMTYPE_PTR) {
                        emit("ld      (_%s),hl", node->left_node->sym->name);

                    } else if (node->left_node->sym->typespec == SYM_TYPESPEC_CHAR) {
                        emit("ld      a,l");
                        emit("ld      (_%s),a", node->left_node->sym->name);

                    } else {
                        printf("Unimplemented identifier symbol type in expression!\n");
                        syntax_error();
                    }

                } else if (node->left_node->sym->storage == SYM_STORAGE_STACK) {
                    emit("ld      (ix+%d),l", node->left_node->sym->value);
                    if (node->left_node->sym->typespec == SYM_TYPESPEC_INT || node->left_node->sym->symtype == SYM_SYMTYPE_PTR) {
                        emit("ld      (ix+%d),h", node->left_node->sym->value + 1);
                    }
                } else {
                    syntax_error();
                }

            } else {
                emit("push    hl");

                if (node->left_node->op == TOK_DEREF) {
                    emit_expr(node->left_node->left_node);

                    if (node->left_node->typespec == SYM_TYPESPEC_CHAR) {
                        emit("pop     de");
                        emit("ld      (hl),e");
                        emit("ex      de,hl");

                    } else if (node->left_node->typespec == SYM_TYPESPEC_INT) {
                        emit("pop     de");
                        emit("ld      (hl),e");
                        emit("inc     hl");
                        emit("ld      (hl),d");
                        emit("ex      de,hl");

                    } else {
                        error("Deref non-pointer");
                    }

                    // } else if (node->left_node->op == TOK_INDEX) {

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

        case '+':
            emit_binary(node);
            emit("add     hl,de");
            if (node->symtype == SYM_SYMTYPE_PTR && node->typespec == SYM_TYPESPEC_INT)
                emit("add     hl,de");
            break;

        case '-':
            emit_16b_sub(node);
            if (node->symtype == SYM_SYMTYPE_PTR && node->typespec == SYM_TYPESPEC_INT) {
                emit("or      a");
                emit("sbc     hl,de");
            }
            break;

            // clang-format off
        case '*':        emit_binary(node); emit("call    __multsi"); flags |= FLAGS_USES_MULTSI; break;
        case '/':        emit_binary(node); emit("call    __divsi");  flags |= FLAGS_USES_DIVSI;  break;
        case '%':        emit_binary(node); emit("call    __modsi");  flags |= FLAGS_USES_MODSI;  break;
        case TOK_OP_SHL: emit_binary(node); emit("call    __shl");    flags |= FLAGS_USES_SHL;    break;
        case TOK_OP_SHR: emit_binary(node); emit("call    __shr");    flags |= FLAGS_USES_SHR;    break;
            // clang-format on

        case TOK_OP_LE:
            emit_binary(node);
            if (node->left_node->symtype == SYM_SYMTYPE_PTR) {
                // Unsigned comparison
                emit("call    __leu");
                flags |= FLAGS_USES_LEU;
            } else {
                // Signed comparison
                emit("call    __les");
                flags |= FLAGS_USES_LES;
            }
            break;

        case TOK_OP_GE:
            emit_binary(node);
            if (node->left_node->symtype == SYM_SYMTYPE_PTR) {
                // Unsigned comparison
                emit("call    __geu");
                flags |= FLAGS_USES_GEU;
            } else {
                // Signed comparison
                emit("call    __ges");
                flags |= FLAGS_USES_GES;
            }
            break;

        case '<':
            emit_binary(node);
            if (node->left_node->symtype == SYM_SYMTYPE_PTR) {
                // Unsigned comparison
                emit("call    __ltu");
                flags |= FLAGS_USES_LTU;
            } else {
                // Signed comparison
                emit("call    __lts");
                flags |= FLAGS_USES_LTS;
            }
            break;

        case '>':
            emit_binary(node);
            if (node->left_node->symtype == SYM_SYMTYPE_PTR) {
                // Unsigned comparison
                emit("call    __gtu");
                flags |= FLAGS_USES_GTU;
            } else {
                // Signed comparison
                emit("call    __gts");
                flags |= FLAGS_USES_GTS;
            }
            break;

        case TOK_OP_EQ: {
            int lbl1 = gen_lbl_idx();
            int lbl2 = gen_lbl_idx();
            emit_16b_sub(node);
            emit("jp      z,.l%d", lbl1);
            emit("ld      hl,0");
            emit("jr      .l%d", lbl2);
            emit_local_lbl(lbl1);
            emit("inc     hl");
            emit_local_lbl(lbl2);
            break;
        }

        case TOK_OP_NE: {
            int lbl = gen_lbl_idx();

            emit_16b_sub(node);
            emit("jr      z,.l%d", lbl);
            emit("ld      hl,1");
            emit_local_lbl(lbl);
            break;
        }

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
            int lbl1 = gen_lbl_idx();
            int lbl2 = gen_lbl_idx();
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
            int lbl1 = gen_lbl_idx();
            int lbl2 = gen_lbl_idx();
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
        expect_tok_ack('(');
        struct expr_node *node = parse_expression();
        expect_tok_ack(')');
        emit_expr(node);
        emit("ld      a,h");
        emit("or      l");
        int lbl1 = gen_lbl_idx();
        emit("jp      z,.l%d", lbl1);
        parse_statement(lbl_continue, lbl_break);

        bool has_else = false;
        if (get_token() == TOK_ELSE) {
            ack_token();
            has_else = true;
        }

        if (has_else) {
            int lbl2 = gen_lbl_idx();
            emit("jp      .l%d", lbl2);
            emit_local_lbl(lbl1);
            parse_statement(lbl_continue, lbl_break);
            emit_local_lbl(lbl2);
        } else {
            emit_local_lbl(lbl1);
        }

    } else if (token == TOK_WHILE) {
        ack_token();
        expect_tok_ack('(');
        struct expr_node *node = parse_expression();
        expect_tok_ack(')');

        int lbl1 = gen_lbl_idx();
        int lbl2 = gen_lbl_idx();
        emit_local_lbl(lbl1);

        emit_expr(node);
        emit("ld      a,h");
        emit("or      l");
        emit("jp      z,.l%d", lbl2);

        parse_statement(lbl1, lbl2);
        emit("jp      .l%d", lbl1);
        emit_local_lbl(lbl2);

    } else if (token == TOK_CONTINUE) {
        ack_token();
        if (lbl_continue < 0) {
            error("Continue outside loop");
        }
        emit("jp      .l%d", lbl_continue);
        expect_tok_ack(';');

    } else if (token == TOK_BREAK) {
        ack_token();
        if (lbl_break < 0) {
            error("Break outside loop");
        }
        emit("jp      .l%d", lbl_break);
        expect_tok_ack(';');

    } else if (token == TOK_RETURN) {
        ack_token();
        printf("Return!\n");
        token = get_token();
        if (token != ';') {
            struct expr_node *node = parse_expression();
            emit_expr(node);
        }
        emit("jp      .return");
        expect_tok_ack(';');

    } else {
        struct expr_node *node = parse_expression();
        emit_expr(node);
        expect_tok_ack(';');
    }
}

static struct symbol *parse_var(uint8_t storage, int value) {
    uint8_t token = get_token();
    if (token == TOK_CHAR || token == TOK_INT) {
        uint8_t tok_type = token;
        ack_token();

        // Pointer?
        uint8_t is_ptr = 0;
        if (get_token() == '*') {
            is_ptr = 1;
            ack_token();
        }

        expect_tok(TOK_IDENTIFIER);
        struct symbol *sym = symbol_add(tok_strval, 0);
        ack_token();

        sym->symtype  = is_ptr ? SYM_SYMTYPE_PTR : SYM_SYMTYPE_VAR;
        sym->typespec = (tok_type == TOK_CHAR) ? SYM_TYPESPEC_CHAR : SYM_TYPESPEC_INT;
        sym->storage  = storage;
        sym->value    = value;

        if (get_token() == '[') {
            ack_token();
            if (is_ptr || storage != SYM_STORAGE_STATIC)
                syntax_error();

            sym->symtype = SYM_SYMTYPE_ARRAY;

            if (get_token() == ']') {
                sym->value = -1;
            } else {
                struct expr_node *node = parse_expression();
                if (!node || (node->op != TOK_CONSTANT))
                    syntax_error();
                sym->value = node->val;
            }
            expect_tok_ack(']');
        }

        symbol_dump(sym);
        return sym;

    } else {
        syntax_error();
        return NULL;
    }
}

static void parse_compound(bool new_scope, int lbl_continue, int lbl_break) {
    if (new_scope)
        symbol_push_scope();
    expect_tok_ack('{');

    while (1) {
        int token = get_token();
        if (token == '}') {
            // End of compound statement
            ack_token();
            break;

        } else if (token == TOK_CHAR || token == TOK_INT) {
            cur_ix_offset -= 2;
            parse_var(SYM_STORAGE_STACK, cur_ix_offset);

            token = get_token();
            if (token == '=') {
                ack_token();
                struct expr_node *node = parse_expression();
                emit_expr(node);
                emit("push    hl");
            } else {
                emit("push    af");
            }
            expect_tok_ack(';');

        } else {
            parse_statement(lbl_continue, lbl_break);
        }
    }
    if (new_scope)
        symbol_pop_scope();
}

static int emit_str_bytes(char *str, uint16_t buf_len) {
    if (buf_len == 0) {
        buf_len = strlen(str) + 1;
    }
    int result = buf_len;

    const uint8_t *ps = (uint8_t *)str;
    char          *pd = tmpbuf;
    int            len;
    while (buf_len) {
        if (pd == tmpbuf)
            len = sprintf(pd, "    defb %u", *ps);
        else
            len = sprintf(pd, ",%u", *ps);

        ps++;
        pd += len;
        buf_len--;

        if (pd - tmpbuf > 70) {
            *(pd++) = '\n';
            output_puts(tmpbuf, pd - tmpbuf);
            pd = tmpbuf;
        }
    }

    *(pd++) = '\n';
    output_puts(tmpbuf, pd - tmpbuf);

    return result;
}

static void emit_string_literals(void) {
    struct string *last = strings_last();
    struct string *str  = strings_first();
    while (str != last) {
        int len = sprintf(tmpbuf, "__str%d:\n", str->idx);
        output_puts(tmpbuf, len);
        emit_str_bytes(str->buf, str->buf_len + 1);
        str = (struct string *)((uint8_t *)str + sizeof(*str) + str->buf_len + 1);
    }
    strings_clear();
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
            expect_tok_ack('(');
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
                    parse_var(SYM_STORAGE_STACK, offset);
                    offset += 2;
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

            expect_tok('{');

            cur_ix_offset = 0;
            parse_compound(false, -1, -1);

            output_puts(".return:\n", 0);
            emit("ld      sp,ix");
            emit("pop     ix");
            emit("ret");

            symbol_pop_scope();
            emit_string_literals();
        }

        // Variable definition?
        else if (token == TOK_CHAR || token == TOK_INT) {
            struct symbol *sym = parse_var(SYM_STORAGE_STATIC, 0);

            sprintf(tmpbuf, "_%s:\n", tok_strval);
            output_puts(tmpbuf, 0);

            if (sym->symtype == SYM_SYMTYPE_ARRAY) {
                int elements = 0;
                if (get_token() == '=') {
                    ack_token();

                    if (get_token() == TOK_STRING_LITERAL) {
                        if (sym->typespec != SYM_TYPESPEC_CHAR)
                            syntax_error();

                        elements = emit_str_bytes(tok_strval, 0);
                        ack_token();

                    } else {
                        expect_tok_ack('{');

                        while (1) {
                            struct expr_node *node = parse_expression();
                            if (!node || node->op != TOK_CONSTANT)
                                syntax_error();

                            if (sym->typespec == SYM_TYPESPEC_CHAR)
                                emit("defb %d", node->val);
                            else
                                emit("defw %d", node->val);

                            elements++;

                            if (get_token() != ',')
                                break;
                            ack_token();
                        }
                        expect_tok_ack('}');
                    }
                }

                if (sym->value >= 0) {
                    if (elements > sym->value)
                        error("Too many initializers");

                    int pad = sym->value - elements;
                    if (pad > 0) {
                        if (sym->typespec == SYM_TYPESPEC_INT)
                            pad *= 2;
                        emit("defs %d", pad);
                    }
                }

            } else {
                int value  = 0;
                int stridx = -1;
                if (get_token() == '=') {
                    ack_token();
                    struct expr_node *node = parse_expression();
                    if (!node || (node->op != TOK_CONSTANT && node->op != TOK_STRING_LITERAL))
                        syntax_error();

                    if (node->op == TOK_CONSTANT)
                        value = node->val;
                    else {
                        stridx = node->str->idx;
                    }
                }

                if (stridx >= 0) {
                    if (sym->symtype != SYM_SYMTYPE_PTR || sym->typespec != SYM_TYPESPEC_CHAR)
                        syntax_error();

                    emit("defw __str%d", stridx);
                } else if (sym->symtype == SYM_SYMTYPE_PTR || sym->typespec == SYM_TYPESPEC_INT) {
                    emit("defw %d", value);
                } else {
                    emit("defb %d", value);
                }
            }

            expect_tok_ack(';');
        }

        // Syntax error
        else {
            syntax_error();
        }
    }

    emit_string_literals();

    emit("\n; --- Support functions ---");
    if (flags & FLAGS_USES_MULTSI) {
        int lbl1 = gen_lbl_idx();
        int lbl2 = gen_lbl_idx();
        int lbl3 = gen_lbl_idx();

        emit_lbl("_multsi");
        emit("ld      c,l");
        emit("ld      b,h");
        emit("xor     a");
        emit("ld      l,a");
        emit("or      b");
        emit("ld      b,16");
        emit("jr      nz,.l%d", lbl2);
        emit("ld      b,8");
        emit("ld      a,c");
        emit_local_lbl(lbl1);
        emit("add     hl,hl");
        emit_local_lbl(lbl2);
        emit("rl      c");
        emit("rla");
        emit("jr      nc,.l%d", lbl3);
        emit("add     hl,de");
        emit_local_lbl(lbl3);
        emit("djnz    .l%d", lbl1);
        emit("ret");
    }

    if (flags & (FLAGS_USES_DIVSI | FLAGS_USES_MODSI)) {
        int lbl1 = gen_lbl_idx();
        int lbl2 = gen_lbl_idx();
        int lbl3 = gen_lbl_idx();
        int lbl4 = gen_lbl_idx();
        int lbl5 = gen_lbl_idx();

        // hl:dividend, de:divisor
        emit_lbl("_divu16");
        emit("ld      a,e");
        emit("and     $80");
        emit("or      d");
        emit("jr      nz,.l%d", lbl3); // de >= 128? Use second algorithm

        // Unsigned 16/7-bit division
        emit("ld      b,16");
        emit("adc     hl,hl");
        emit_local_lbl(lbl1);
        emit("rla");
        emit("sub     e");
        emit("jr      nc,.l%d", lbl2);
        emit("add     a,e");
        emit_local_lbl(lbl2);
        emit("ccf");
        emit("adc     hl,hl");
        emit("djnz    .l%d", lbl1);
        emit("ld      e,a");
        emit("ex      de,hl");
        emit("ret");

        // Unsigned 16/16-bit division
        emit_local_lbl(lbl3);
        emit("ld      b,9");
        emit("ld      a,l");
        emit("ld      l,h");
        emit("ld      h,0");
        emit("rr      l");
        emit_local_lbl(lbl4);
        emit("adc     hl,hl");
        emit("sbc     hl,de");
        emit("jr      nc,.l%d", lbl5);
        emit("add     hl,de");
        emit_local_lbl(lbl5);
        emit("ccf");
        emit("rla");
        emit("djnz    .l%d", lbl4);
        emit("rl      b");
        emit("ld      d,b");
        emit("ld      e,a");
        emit("ret");
    }
    if (flags & (FLAGS_USES_DIVSI | FLAGS_USES_MODSI)) {
        int lbl1 = gen_lbl_idx();
        int lbl2 = gen_lbl_idx();

        emit_lbl("_divsi");
        emit("ld      a,h");
        emit("xor     d");
        emit("rla");
        emit("ld      a,h");
        emit("push    af");
        emit("rla");
        emit("jr      nc,.l%d", lbl1);
        emit("sub     a");
        emit("sub     l");
        emit("ld      l,a");
        emit("sbc     a,a");
        emit("sub     h");
        emit("ld      h,a");
        emit_local_lbl(lbl1);
        emit("bit     7,d");
        emit("jr      z,.l%d", lbl2);
        emit("sub     a,a");
        emit("sub     a,e");
        emit("ld      e,a");
        emit("sbc     a,a");
        emit("sub     a,d");
        emit("ld      d,a");
        emit_local_lbl(lbl2);
        emit("call    __divu16");
        emit("pop     af");
        emit("ret     nc");
        emit("ld      b,a");
        emit("sub     a");
        emit("sub     e");
        emit("ld      e,a");
        emit("sbc     a,a");
        emit("sub     d");
        emit("ld      d,a");
        emit("ld      a,b");   // de:quotient, hl:remainder
        emit("ex      de,hl"); // hl:quotient, de:remainder
        emit("ret");
    }
    if (flags & FLAGS_USES_MODSI) {
        emit_lbl("_modsi");
        emit("call    __divsi"); // hl:quotient, de:remainder
        emit("ex      de,hl");   // de:quotient, hl:remainder
        emit("rla");
        emit("ret     nc");
        emit("ex      de,hl"); // hl:quotient, de:remainder
        emit("sub     a");
        emit("sub     e");
        emit("ld      e,a");
        emit("sbc     a,a");
        emit("sub     d");
        emit("ld      d,a");
        emit("ex      de,hl");
        emit("ret");
    }
    if (flags & FLAGS_USES_SHL) {
        int lbl1 = gen_lbl_idx();
        int lbl2 = gen_lbl_idx();

        emit_lbl("_shl");
        emit("ld      b,e");
        emit("inc     b");
        emit("jr      .l%d", lbl2);
        emit_local_lbl(lbl1);
        emit("add     hl,hl");
        emit_local_lbl(lbl2);
        emit("djnz    .l%d", lbl1);
        emit("ret");
    }
    if (flags & FLAGS_USES_SHR) {
        int lbl1 = gen_lbl_idx();
        int lbl2 = gen_lbl_idx();

        emit_lbl("_shr");
        emit("ld      b, e");
        emit("inc     b");
        emit("jr      .l%d", lbl2);
        emit_local_lbl(lbl1);
        emit("sra     h");
        emit("rr      l");
        emit_local_lbl(lbl2);
        emit("djnz    .l%d", lbl1);
        emit("ret");
    }
    if (flags & FLAGS_USES_CALL_HL) {
        emit_lbl("_call_hl");
        emit("jp      (hl)");
    }
    if (flags & FLAGS_USES_LTU) {
        emit_lbl("_ltu");
        emit("xor     a");
        emit("sbc     hl,de");
        emit("ld      a,0");
        emit("rla");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_LEU) {
        emit_lbl("_leu");
        emit("ld      a,e");
        emit("sub     l");
        emit("ld      a,d");
        emit("sbc     a,h");
        emit("ld      a,0");
        emit("rla");
        emit("xor     1");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_GTU) {
        emit_lbl("_gtu");
        emit("ld      a,e");
        emit("sub     l");
        emit("ld      a,d");
        emit("sbc     a,h");
        emit("ld      a,0");
        emit("rla");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_GEU) {
        emit_lbl("_geu");
        emit("xor     a");
        emit("sbc     hl,de");
        emit("ld      a,0");
        emit("rla");
        emit("xor     1");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_LTS) {
        int lbl = gen_lbl_idx();
        emit_lbl("_lts");
        emit("ld      a,l");
        emit("sub     e");
        emit("ld      a,h");
        emit("sbc     a,d");
        emit("jp      po,.l%d", lbl);
        emit("xor     $80");
        emit_local_lbl(lbl);
        emit("rlca");
        emit("and     1");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_LES) {
        int lbl = gen_lbl_idx();
        emit_lbl("_les");
        emit("ld      a,e");
        emit("sub     l");
        emit("ld      a,d");
        emit("sbc     a,h");
        emit("jp      po,.l%d", lbl);
        emit("xor     $80");
        emit_local_lbl(lbl);
        emit("rlca");
        emit("and     1");
        emit("xor     1");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_GTS) {
        int lbl = gen_lbl_idx();
        emit_lbl("_gts");
        emit("ld      a,e");
        emit("sub     l");
        emit("ld      a,d");
        emit("sbc     a,h");
        emit("jp      po,.l%d", lbl);
        emit("xor     $80");
        emit_local_lbl(lbl);
        emit("rlca");
        emit("and     1");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
    if (flags & FLAGS_USES_GES) {
        int lbl = gen_lbl_idx();
        emit_lbl("_ges");
        emit("ld      a,l");
        emit("sub     e");
        emit("ld      a,h");
        emit("sbc     a,d");
        emit("jp      po,.l%d", lbl);
        emit("xor     $80");
        emit_local_lbl(lbl);
        emit("rlca");
        emit("and     1");
        emit("xor     1");
        emit("ld      l,a");
        emit("ld      h,0");
        emit("ret");
    }
}
