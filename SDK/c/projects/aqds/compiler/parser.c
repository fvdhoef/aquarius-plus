#include "parser.h"
#include "tokenizer.h"

static void expect_ack(uint8_t token) {
    if (get_token() != token)
        syntax_error();
    ack_token();
}

static void expect(uint8_t token) {
    if (get_token() != token)
        syntax_error();
}

static void parse_expression(void) {
    int token = get_token();
    printf("token: %d\n", token);
    ack_token();
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

            if (type == TOK_CHAR) {
                sprintf(tmpbuf, "_%s: defb 0\n", tok_strval);
                output_puts(tmpbuf, 0);
            } else if (type == TOK_INT) {
                sprintf(tmpbuf, "_%s: defw 0\n", tok_strval);
                output_puts(tmpbuf, 0);
            }

            ack_token();
            expect_ack(';');

        } else {
            syntax_error();
        }
    }
}