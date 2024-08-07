#include "expr.h"
#include "symbols.h"

static bool _allow_undefined_symbol;

static bool is_decimal(uint8_t ch) {
    return ch >= '0' && ch <= '9';
}

static bool is_hexadecimal(uint8_t ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
}

static bool is_alpha(uint8_t ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static uint16_t parse_primary_expr(void) {
    skip_whitespace();

    uint8_t  ch    = *cur_p;
    uint16_t value = 0;

    if (ch == '$') {
        // Hexadecimal value
        cur_p++;
        ch = to_lower(*cur_p);
        if (is_hexadecimal(ch)) {
            while (is_hexadecimal(ch)) {
                value <<= 4;
                if (ch >= '0' && ch <= '9')
                    value += ch - '0';
                else
                    value += ch - 'a' + 10;

                cur_p++;
                ch = to_lower(*cur_p);
            }
        } else {
            // Interpret as '$', which is current address
            return cur_addr;
        }

    } else if (is_decimal(ch)) {
        // Decimal value
        while (is_decimal(ch)) {
            value *= 10;
            if (ch >= '0' && ch <= '9')
                value += ch - '0';

            cur_p++;
            ch = to_lower(*cur_p);
        }

    } else if (ch == '(') {
        cur_p++;
        value = parse_expression(_allow_undefined_symbol);
        skip_whitespace();
        if (*(cur_p++) != ')')
            error("Expected right parenthesis");

    } else if (ch == '\'') {
        cur_p++;
        if (cur_p[0] == 0 || cur_p[1] != '\'')
            error("Invalid character constant");

        value = cur_p[0];
        cur_p += 2;

    } else {
        const char *symbol = cur_p;

        while (ch == '_' || ch == '.' ||
               (ch >= '0' && ch <= '9') ||
               (ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z')) {
            ch = *(++cur_p);
        }

        if (symbol == cur_p) {
            error("Expected primary expression");
        } else
            return symbol_get(symbol, cur_p - symbol, _allow_undefined_symbol);
    }
    return value;
}

// clang-format off
static uint16_t parse_unary_expr(void) {
    skip_whitespace();
    if      (cur_p[0] == '-') { cur_p++; return -parse_unary_expr(); }
    else if (cur_p[0] == '+') { cur_p++; return  parse_unary_expr(); }
    else if (cur_p[0] == '~') { cur_p++; return ~parse_unary_expr(); }
    else if (cur_p[0] == '<') { cur_p++; return  parse_unary_expr() & 0xFF; }
    else if (cur_p[0] == '>') { cur_p++; return  parse_unary_expr() >> 8; }
    else if (cur_p[0] == 'l' && cur_p[1] == 'o' && cur_p[2] == 'w' &&                    !is_alpha(cur_p[3])) { cur_p += 3; return parse_unary_expr() & 0xFF; }
    else if (cur_p[0] == 'h' && cur_p[1] == 'i' && cur_p[2] == 'g' && cur_p[3] == 'h' && !is_alpha(cur_p[4])) { cur_p += 4; return parse_unary_expr() >> 8; }
    else                      {          return  parse_primary_expr(); }
}

static uint16_t parse_mult_expr(void) {
    uint16_t val = parse_unary_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '*') { cur_p++; val *= parse_unary_expr(); }
        else if (cur_p[0] == '/') { cur_p++; val /= parse_unary_expr(); }
        else if (cur_p[0] == '%') { cur_p++; val %= parse_unary_expr(); }
        else                      { break; }
    }
    return val;
}

static uint16_t parse_add_expr(void) {
    uint16_t val = parse_mult_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '+') { cur_p++; val += parse_mult_expr(); }
        else if (cur_p[0] == '-') { cur_p++; val -= parse_mult_expr(); }
        else                      { break; }
    }
    return val;
}

static uint16_t parse_shift_expr(void) {
    uint16_t val = parse_add_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '<' && cur_p[1] == '<') { cur_p += 2; val <<= parse_add_expr(); }
        else if (cur_p[0] == '>' && cur_p[1] == '>') { cur_p += 2; val >>= parse_add_expr(); }
        else                                         { break; }
    }
    return val;
}

static uint16_t parse_rel_expr(void) {
    uint16_t val = parse_shift_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '<' && cur_p[1] == '=') { cur_p+=2; val = (val <= parse_shift_expr()); }
        else if (cur_p[0] == '>' && cur_p[1] == '=') { cur_p+=2; val = (val >= parse_shift_expr()); }
        else if (cur_p[0] == '<' && cur_p[1] != '<') { cur_p++;  val = (val <  parse_shift_expr()); }
        else if (cur_p[0] == '>' && cur_p[1] != '>') { cur_p++;  val = (val >  parse_shift_expr()); }
        else                                         { break; }
    }
    return val;
}

static uint16_t parse_eq_expr(void) {
    uint16_t val = parse_rel_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '=' && cur_p[1] == '=') { cur_p+=2; val = (val == parse_rel_expr()); }
        else if (cur_p[0] == '=')                    { cur_p++;  val = (val == parse_rel_expr()); }
        else if (cur_p[0] == '!' && cur_p[1] == '=') { cur_p+=2; val = (val != parse_rel_expr()); }
        else                                         { break; }
    }
    return val;
}

static uint16_t parse_and_expr(void) {
    uint16_t val = parse_eq_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '&' && cur_p[1] != '&') { cur_p++; val &= parse_eq_expr(); }
        else                                         { break; }
    }
    return val;
}

static uint16_t parse_xor_expr(void) {
    uint16_t val = parse_and_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '^') { cur_p++; val ^= parse_and_expr(); }
        else                      { break; }
    }
    return val;
}

static uint16_t parse_or_expr(void) {
    uint16_t val = parse_xor_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '|' && cur_p[1] != '|') { cur_p++; val ^= parse_xor_expr(); }
        else                                         { break; }
    }
    return val;
}

static uint16_t parse_logical_and_expr(void) {
    uint16_t val = parse_or_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '&' && cur_p[1] == '&') { cur_p++; val = val && parse_or_expr(); }
        else                                         { break; }
    }
    return val;
}

static uint16_t parse_logical_or_expr(void) {
    uint16_t val = parse_logical_and_expr();
    while (1) {
        skip_whitespace();
        if      (cur_p[0] == '|' && cur_p[1] == '|') { cur_p++; val = val || parse_logical_and_expr(); }
        else                                         { break; }
    }
    return val;
}

// clang-format on

uint16_t parse_expression(bool allow_undefined_symbol) {
    _allow_undefined_symbol = allow_undefined_symbol;
    return parse_logical_or_expr();
}
