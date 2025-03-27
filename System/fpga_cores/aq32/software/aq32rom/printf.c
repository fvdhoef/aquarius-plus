#include "printf.h"
#include "console.h"
#include "lib.h"

enum {
    FLAG_ZERO_PADDING   = (1 << 0),
    FLAG_LEFT_JUSTIFIED = (1 << 1),
    FLAG_LONG_INT       = (1 << 2),
    FLAG_LONG_LONG_INT  = (1 << 3),
    FLAG_NEGATIVE       = (1 << 4),
};

static inline bool isdigit(char c) {
    return c >= '0' && c <= '9';
}

static inline char tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    } else {
        return c;
    }
}

static int string_out(const char *str, void (*char_out)(void *, char), void *context, int flags, int field_width) {
    int outlen = 0;

    int str_length = 0;
    while (str[str_length] != '\0') {
        str_length++;
    }

    if (!(flags & FLAG_LEFT_JUSTIFIED)) {
        while (str_length++ < field_width) {
            char_out(context, ' ');
            outlen++;
        }
    }

    while (*str != '\0') {
        char_out(context, *(str++));
        outlen++;
    }

    while (str_length++ < field_width) {
        char_out(context, ' ');
        outlen++;
    }
    return outlen;
}

int printf_handler(void (*char_out)(void *, char), void *context, const char *format, va_list ap) {
    int outlen = 0;

    while (1) {
        char c = *(format++);
        if (c == '\0') {
            // End of string
            break;
        }

        if (c != '%') {
            // Non escape character
            char_out(context, c);
            outlen++;
            continue;
        }

        int flags       = 0;
        int field_width = 0;

        c = *(format++);

        if (c == '0') {
            // Flag: '0' padding
            flags = FLAG_ZERO_PADDING;
            c     = *(format++);
        } else if (c == '-') {
            // Flag: left justified
            flags = FLAG_LEFT_JUSTIFIED;
            c     = *(format++);
        }

        while (isdigit(c)) {
            // Field width
            field_width = field_width * 10 + c - '0';
            c           = *(format++);
        }

        if (c == 'l' || c == 'L') {
            // Prefix: Size is long int
            flags |= FLAG_LONG_INT;
            c = *(format++);

            if (c == 'l' || c == 'L') {
                flags &= ~FLAG_LONG_INT;
                flags |= FLAG_LONG_LONG_INT;
                c = *(format++);
            }
        } else if (c == 'z') {
            // Prefix: size_t
            c = *(format++);
        }

        if (c == '\0') {
            // End of string
            break;
        }

        char conv_specifier = tolower(c);
        int  radix;

        switch (conv_specifier) {
            case 'c': {
                // Character
                char char_buf[2];
                char_buf[0] = (char)va_arg(ap, int);
                char_buf[1] = '\0';
                outlen += string_out(char_buf, char_out, context, flags, field_width);
                continue;
            }

            case 's': {
                // String
                const char *str = va_arg(ap, const char *);
                if (str) {
                    outlen += string_out(str, char_out, context, flags, field_width);
                } else {
                    outlen += string_out("(null)", char_out, context, flags, field_width);
                }
                continue;
            }

            case 'o':
                radix = 8;
                break; // Octal
            case 'd':
                radix = 10;
                break; // Signed decimal
            case 'u':
                radix = 10;
                break; // Unsigned decimal
            case 'x':
                radix = 16;
                break; // Hexadecimal

            case 'p': {
                char_out(context, '0');
                char_out(context, 'x');
                outlen += 2;
                flags       = FLAG_ZERO_PADDING;
                field_width = 8;

                radix = 16;
                break;
            }

            default: {
                // Unknown type (pass-through)
                char_out(context, c);
                outlen++;
                continue;
            }
        }

        // Get an argument and put it in numeral
        uint64_t value;
        if (flags & FLAG_LONG_INT) {
            if (conv_specifier == 'd') {
                long val = va_arg(ap, long);
                if (val < 0) {
                    val = -val;
                    flags |= FLAG_NEGATIVE;
                }
                value = val;
            } else {
                value = va_arg(ap, unsigned long);
            }

        } else if (flags & FLAG_LONG_LONG_INT) {
            if (conv_specifier == 'd') {
                long long val = va_arg(ap, long long);
                if (val < 0) {
                    val = -val;
                    flags |= FLAG_NEGATIVE;
                }
                value = val;
            } else {
                value = va_arg(ap, unsigned long long);
            }

        } else {
            if (conv_specifier == 'd') {
                int val = va_arg(ap, int);
                if (val < 0) {
                    val = -val;
                    flags |= FLAG_NEGATIVE;
                }
                value = val;
            } else {
                value = va_arg(ap, unsigned int);
            }
        }

        int  i = 0;
        char str[32];
        do {
            char digit = (char)(value % radix);
            value /= radix;
            if (digit > 9) {
                digit = (digit - 10) + (c == 'x' ? 'a' : 'A');
            } else {
                digit += '0';
            }
            str[i++] = digit;
        } while (value != 0);

        if (flags & FLAG_NEGATIVE) {
            str[i++] = '-';
        }

        int  j       = i;
        char padChar = (flags & FLAG_ZERO_PADDING) ? '0' : ' ';

        if ((flags & FLAG_LEFT_JUSTIFIED) == 0) {
            while (j++ < field_width) {
                char_out(context, padChar);
                outlen++;
            }
        }

        do {
            char_out(context, str[--i]);
            outlen++;
        } while (i);

        while (j++ < field_width) {
            char_out(context, ' ');
            outlen++;
        }
    }
    return outlen;
}

static void printf_char_out(void *context, char ch) {
    if (ch == '\n') {
        console_putc('\r');
        console_putc(ch);
    } else {
        console_putc(ch);
    }
}

int vprintf(const char *format, va_list ap) {
    return printf_handler(printf_char_out, NULL, format, ap);
}

int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int result = vprintf(format, ap);
    va_end(ap);
    return result;
}
