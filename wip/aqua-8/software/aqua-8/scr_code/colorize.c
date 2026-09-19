#include "colorize.h"
#include "ctype2.h"
#include "edit_ops.h"

#define COLOR_VALUE    12
#define COLOR_RESERVED 14
#define COLOR_BUILTIN  11
#define COLOR_IDENT    6
#define COLOR_NORMAL   7
#define COLOR_COMMENT  13

static uint8_t colors[EDITOR_COLUMNS];
static int     colorize_idx;

static void put_color(unsigned color) {
    if (colorize_idx >= 0 && colorize_idx < EDITOR_COLUMNS)
        colors[colorize_idx] = color;
    colorize_idx++;
}

typedef struct {
    const char *name;
    uint8_t     len;
    uint8_t     color;
} reserved_t;

#define ENTRY(a, color) \
    {a, sizeof(a) - 1, color}

static const reserved_t names[] = {
    // LUA reserved words
    ENTRY("and", COLOR_RESERVED),
    ENTRY("break", COLOR_RESERVED),
    ENTRY("do", COLOR_RESERVED),
    ENTRY("else", COLOR_RESERVED),
    ENTRY("elseif", COLOR_RESERVED),
    ENTRY("end", COLOR_RESERVED),
    ENTRY("false", COLOR_VALUE),
    ENTRY("for", COLOR_RESERVED),
    ENTRY("function", COLOR_RESERVED),
    ENTRY("goto", COLOR_RESERVED),
    ENTRY("if", COLOR_RESERVED),
    ENTRY("in", COLOR_RESERVED),
    ENTRY("local", COLOR_RESERVED),
    ENTRY("nil", COLOR_VALUE),
    ENTRY("not", COLOR_RESERVED),
    ENTRY("or", COLOR_RESERVED),
    ENTRY("repeat", COLOR_RESERVED),
    ENTRY("return", COLOR_RESERVED),
    ENTRY("then", COLOR_RESERVED),
    ENTRY("true", COLOR_VALUE),
    ENTRY("until", COLOR_RESERVED),
    ENTRY("while", COLOR_RESERVED),

    // GFX API functions
    ENTRY("camera", COLOR_BUILTIN),
    ENTRY("clip", COLOR_BUILTIN),
    ENTRY("cls", COLOR_BUILTIN),
    ENTRY("color", COLOR_BUILTIN),
    ENTRY("pal", COLOR_BUILTIN),
    ENTRY("palt", COLOR_BUILTIN),
    // ENTRY("fillp", COLOR_BUILTIN),
    ENTRY("flip", COLOR_BUILTIN),
    ENTRY("line", COLOR_BUILTIN),
    ENTRY("rect", COLOR_BUILTIN),
    ENTRY("rectfill", COLOR_BUILTIN),
    ENTRY("rrect", COLOR_BUILTIN),
    ENTRY("rrectfill", COLOR_BUILTIN),
    ENTRY("oval", COLOR_BUILTIN),
    ENTRY("ovalfill", COLOR_BUILTIN),
    ENTRY("circ", COLOR_BUILTIN),
    ENTRY("circfill", COLOR_BUILTIN),
    ENTRY("pget", COLOR_BUILTIN),
    ENTRY("pset", COLOR_BUILTIN),
    ENTRY("print", COLOR_BUILTIN),
    // ENTRY("printh", COLOR_BUILTIN),
    ENTRY("cursor", COLOR_BUILTIN),
    ENTRY("map", COLOR_BUILTIN),
    ENTRY("mget", COLOR_BUILTIN),
    ENTRY("mset", COLOR_BUILTIN),
    ENTRY("fget", COLOR_BUILTIN),
    ENTRY("fset", COLOR_BUILTIN),
    // ENTRY("tline", COLOR_BUILTIN),
    ENTRY("spr", COLOR_BUILTIN),
    // ENTRY("sspr", COLOR_BUILTIN),
    ENTRY("sget", COLOR_BUILTIN),
    ENTRY("sset", COLOR_BUILTIN),

    // Memory & data API function
    // ENTRY("peek", COLOR_BUILTIN),
    // ENTRY("poke", COLOR_BUILTIN),
    // ENTRY("memset", COLOR_BUILTIN),
    // ENTRY("memcpy", COLOR_BUILTIN),
    // ENTRY("reload", COLOR_BUILTIN),
    // ENTRY("cstore", COLOR_BUILTIN),
    // ENTRY("scoresub", COLOR_BUILTIN),
    // ENTRY("cartdata", COLOR_BUILTIN),
    // ENTRY("dget", COLOR_BUILTIN),
    // ENTRY("dset", COLOR_BUILTIN),

    // String
    ENTRY("sub", COLOR_BUILTIN),
    ENTRY("chr", COLOR_BUILTIN),
    ENTRY("ord", COLOR_BUILTIN),
    ENTRY("split", COLOR_BUILTIN),
    ENTRY("tostr", COLOR_BUILTIN),
    ENTRY("tonum", COLOR_BUILTIN),

    // Tables
    ENTRY("add", COLOR_BUILTIN),
    ENTRY("del", COLOR_BUILTIN),
    ENTRY("deli", COLOR_BUILTIN),
    ENTRY("all", COLOR_BUILTIN),
    // ENTRY("foreach", COLOR_BUILTIN),
    ENTRY("pairs", COLOR_BUILTIN),

    ENTRY("setmetatable", COLOR_BUILTIN),
    ENTRY("getmetatable", COLOR_BUILTIN),
    ENTRY("rawset", COLOR_BUILTIN),
    ENTRY("rawget", COLOR_BUILTIN),
    ENTRY("rawequal", COLOR_BUILTIN),
    ENTRY("rawlen", COLOR_BUILTIN),

    // Audio API functions
    ENTRY("music", COLOR_BUILTIN),
    ENTRY("sfx", COLOR_BUILTIN),

    // System API functions
    // ENTRY("run", COLOR_BUILTIN),
    ENTRY("stop", COLOR_BUILTIN),
    ENTRY("reset", COLOR_BUILTIN),
    ENTRY("yield", COLOR_BUILTIN),
    ENTRY("time", COLOR_BUILTIN),
    ENTRY("menuitem", COLOR_BUILTIN),
    // ENTRY("stat", COLOR_BUILTIN),
    // ENTRY("serial", COLOR_BUILTIN),
    // ENTRY("extcmd", COLOR_BUILTIN),
    ENTRY("cocreate", COLOR_BUILTIN),
    ENTRY("coresume", COLOR_BUILTIN),
    ENTRY("costatus", COLOR_BUILTIN),

    // System commands
    // ENTRY("load", COLOR_BUILTIN),

    // Input
    ENTRY("btn", COLOR_BUILTIN),
    ENTRY("btnp", COLOR_BUILTIN),

    // Math functions and operators
    ENTRY("cos", COLOR_BUILTIN),
    ENTRY("sin", COLOR_BUILTIN),
    ENTRY("atan2", COLOR_BUILTIN),
    ENTRY("sqrt", COLOR_BUILTIN),
    ENTRY("srand", COLOR_BUILTIN),
    ENTRY("rnd", COLOR_BUILTIN),
    ENTRY("max", COLOR_BUILTIN),
    ENTRY("min", COLOR_BUILTIN),
    ENTRY("mid", COLOR_BUILTIN),
    ENTRY("flr", COLOR_BUILTIN),
    ENTRY("ceil", COLOR_BUILTIN),
    ENTRY("sgn", COLOR_BUILTIN),
    ENTRY("abs", COLOR_BUILTIN),
    ENTRY("bnot", COLOR_BUILTIN),
    ENTRY("band", COLOR_BUILTIN),
    ENTRY("bor", COLOR_BUILTIN),
    ENTRY("bxor", COLOR_BUILTIN),
    ENTRY("shl", COLOR_BUILTIN),
    ENTRY("shr", COLOR_BUILTIN),
    ENTRY("lshr", COLOR_BUILTIN),
    ENTRY("rotl", COLOR_BUILTIN),
    ENTRY("rotr", COLOR_BUILTIN),
};

static unsigned get_name_color(const uint8_t *name, int len) {
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if (len == names[i].len && memcmp(names[i].name, name, len) == 0)
            return names[i].color;
    }

    return COLOR_IDENT;
}

const uint8_t *colorize(const uint8_t *p, int len, int first_pos) {
    colorize_idx         = -first_pos;
    const uint8_t *p_end = p + len;

    while (p < p_end && colorize_idx < EDITOR_COLUMNS) {
        // Comment?
        if (p[0] == '-' && p[1] == '-') {
            while (colorize_idx < len && colorize_idx < EDITOR_COLUMNS)
                put_color(COLOR_COMMENT);

            // Done for this line
            break;
        }

        if (is_alpha(p[0]) || p[0] == '_') {
            const uint8_t *name = p;

            while (p < p_end && (is_alpha(p[0]) || is_decimal(p[0]) || p[0] == '_'))
                p++;
            int name_len = p - name;

            unsigned color = get_name_color(name, name_len);
            for (int i = 0; i < name_len; i++) {
                put_color(color);
            }

        } else if (is_decimal(p[0])) {
            const uint8_t *value = p;

            bool hexadecimal = false;
            if (p[0] == '0' && p[1] == 'x') {
                p += 2;
                hexadecimal = true;
            }

            while (p < p_end && (is_decimal(p[0]) || (hexadecimal && is_hexadecimal(p[0]))))
                p++;

            int value_len = p - value;
            for (int i = 0; i < value_len; i++) {
                put_color(COLOR_VALUE);
            }

        } else {
            put_color(COLOR_NORMAL);
            p++;
        }
    }
    return colors;
}
