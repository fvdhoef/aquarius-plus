#include "parser.h"
#include "tokenizer.h"
#include "expr.h"

static void expect_ack(uint8_t token) {
    if (get_token() != token)
        syntax_error();
    ack_token();
}

static void expect(uint8_t token) {
    if (get_token() != token)
        syntax_error();
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

                sprintf(tmpbuf, "    call    _%s\n", tok_strval);
                output_puts(tmpbuf, 0);

            } else if (token == '=') {
                ack_token();
                printf("Variable assignment: %s\n", tok_strval);
                parse_expression();
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
