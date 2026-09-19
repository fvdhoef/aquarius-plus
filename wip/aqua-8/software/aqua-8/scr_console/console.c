#include "console.h"
#include "state.h"
#include "draw/draw.h"
#include "readline.h"
#include "ctype2.h"
#include "commands.h"

static bool    cursor_visible = false;
static uint8_t saved_cursor[4 * 6];
static bool    booted = false;

static void console_hide_cursor(void) {
    if (!cursor_visible)
        return;
    cursor_visible = false;

    for (int j = 0; j < 6; j++) {
        for (int i = 0; i < 4; i++) {
            VRAM4BIT[(game_state.y + j) * 200 + (game_state.x + i)] = saved_cursor[j * 4 + i];
        }
    }
}
static void console_show_cursor(void) {
    if (cursor_visible)
        return;
    cursor_visible = true;

    for (int j = 0; j < 6; j++) {
        for (int i = 0; i < 4; i++) {
            volatile uint8_t *p     = &VRAM4BIT[(game_state.y + j) * 200 + (game_state.x + i)];
            saved_cursor[j * 4 + i] = *p;
            if (*p != game_state.color)
                *p = 8;
        }
    }
}

static uint8_t putc_state = 0;

void console_putc(char ch) {
    console_hide_cursor();

    switch (ch) {
        case '\b': game_state.x -= 4; break;
        case '\t': game_state.x = (game_state.x + 16) & 15; break;
        case '\n': game_state.y += 7; break;
        case '\r': game_state.x = 0; break;
        case '\f': putc_state = '\f'; break;
        default: {
            if (putc_state != 0) {
                switch (putc_state) {
                    case '\f': {
                        if (is_hexadecimal(ch)) {
                            ch = to_upper(ch);
                            if (is_decimal(ch))
                                game_state.color = ch - '0';
                            else
                                game_state.color = ch - 'A' + 10;
                        }
                        break;
                    }
                }
                putc_state = 0;
                break;

            } else if (ch >= ' ' && ch <= '~') {
                rect_t r;
                r.x0 = game_state.x;
                r.y0 = game_state.y;
                r.x1 = r.x0 + 3;
                r.y1 = r.y0 + 6;
                fill_rect(&r, 0);

                draw_char_altfont(game_state.x, game_state.y, ch, game_state.color);
                game_state.x += 4;
            }
            break;
        }
    }

    while (game_state.x < 0) {
        game_state.y -= 7;
        game_state.x += 200;
    }
    while (game_state.x >= 200) {
        game_state.y += 7;
        game_state.x -= 200;
    }
    if (game_state.y < 0) {
        game_state.y = 0;
    }
    while (game_state.y + 7 > 161) {
        game_state.y -= 7;

        // Scroll screen up
        const volatile uint32_t *ps = VRAM + (25 * 7);
        volatile uint32_t       *pd = VRAM;
        for (int i = 0; i < 25 * 154; i++) {
            *(pd++) = *(ps++);
        }

        // Clear bottom line
        for (int i = 0; i < 25 * 7; i++) {
            *(pd++) = 0;
        }
    }
    console_show_cursor();
}

void console_puts(const char *str) {
    while (*str) {
        console_putc(*(str++));
    }
}

void console_putline(const char *s) {
    console_puts(s);
    console_puts("\f6\r\n");
}

void console_printf(const char *fmt, ...) {
    char tmp[64];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);

    console_puts(tmp);
}

uint8_t console_getc(void) {
    while (1) {
        int key = KEYBUF;
        if (key < 0)
            return 0;
        if (key & KEY_IS_SCANCODE)
            continue;

        uint8_t ch = key & 0xFF;
        if (ch == 3)
            ch = 27;

        if (key & KEY_MOD_CTRL) {
            uint8_t ch_upper = toupper(ch);
            if (ch_upper >= 'A' && ch_upper <= 'Z') //(ch_upper >= '@' && ch_upper <= '_')
                ch = ch_upper - '@';
            else if (ch_upper == 0x7F)
                ch = '\b';
        }
        return ch;
    }
}

static void startup_sequence(void) {
    VIDEO->PAGE = 0;

    int delay = 4;

    clear_screen(0);
    for (int i = 0; i < delay; i++) {
        frame30 = false;
        while (!frame30) {
        }
    }

    for (int x = 0; x < 200; x += 4) {
        for (int y = 0; y < 160; y += 2) {
            unsigned color = ((((y >> 2) + (x >> 2)) >> 1) & 7) + 6;
            draw_pixel(x, y, color);
        }
    }

    for (int i = 0; i < delay; i++) {
        frame30 = false;
        while (!frame30) {
        }
    }

    for (int x = 2; x < 200; x += 4) {
        for (int y = 1; y < 160; y += 2) {
            unsigned color = ((((y >> 2) + (x >> 2)) >> 1) & 7) + 6;
            draw_pixel(x, y, color);
        }
    }

    for (int i = 0; i < delay; i++) {
        frame30 = false;
        while (!frame30) {
        }
    }

    for (int x = 0; x < 200; x += 4) {
        for (int y = 0; y < 160; y += 2) {
            unsigned color = 0;
            draw_pixel(x, y, color);
        }
    }

    for (int i = 0; i < delay; i++) {
        frame30 = false;
        while (!frame30) {
        }
    }

    for (int x = 2; x < 200; x += 4) {
        for (int y = 1; y < 160; y += 2) {
            unsigned color = 0;
            draw_pixel(x, y, color);
        }
    }

    for (int i = 0; i < delay; i++) {
        frame30 = false;
        while (!frame30) {
        }
    }
}

static readline_ctx_t ctx;
static char           line[256];
static bool           readline_done = true;
static uint32_t       saved_vram[200 * 161 / 2 / sizeof(uint32_t)];

static void save_vram(void) {
    for (unsigned i = 0; i < sizeof(saved_vram) / sizeof(saved_vram[0]); i++) {
        saved_vram[i] = VRAM[i];
    }
}
static void restore_vram(void) {
    for (unsigned i = 0; i < sizeof(saved_vram) / sizeof(saved_vram[0]); i++) {
        VRAM[i] = saved_vram[i];
    }
}

typedef struct {
    const char *cmd;
    void (*handler)(const char *args);
} command_t;

static const command_t commands[] = {
    {"help", cmd_help},
    {"man", cmd_help},
    {"ls", cmd_ls},
    {"dir", cmd_ls},
    {"cd", cmd_cd},
    {"load", cmd_load},
    {"save", cmd_save},
    {"reboot", cmd_reboot},
    {"run", cmd_run},
    {NULL, NULL},
};

void console_perform(void) {
    VIDEO->PAGE = 0;
    restore_vram();

    if (!booted) {
        booted = true;

        startup_sequence();
        clear_screen(0);

        draw_text("Aqua-8", 0, 7, 7, false);

        game_state.y = 21;

        game_state.color = 6;
        console_putline("Aqua-8 " PROJECT_VERSION);
        console_putline("(C) 2026 Frank van den Hoef");
        console_putline("");

        {
            extern char _end;
            extern char __stack_start;
            console_printf("Total heap space: %u\r\n", &__stack_start - &_end);
        }

        console_putline("");
        console_putline("Type \f7help\f6 for help");
        console_putline("");
    }

    while (1) {
        if (readline_done) {
            readline_init(&ctx, line, sizeof(line));
            game_state.x     = 0;
            game_state.y     = ((game_state.y + 6) / 7) * 7;
            game_state.color = 7;
            console_putc('>');
            console_putc(' ');
        }
        readline_done = false;

        while (1) {
            uint8_t ch = console_getc();
            if (ch == 0)
                continue;

            int result = readline_process(&ctx, ch);
            if (result >= 0) {
                console_puts("\r\n");
                readline_done    = true;
                game_state.color = 6;

                // Get command from input
                char        cmd_str[64];
                const char *ps      = ctx.buf;
                unsigned    cmd_len = 0;
                while (*ps == ' ')
                    ps++;
                while (cmd_len < sizeof(cmd_str) - 1) {
                    if (ps[0] == 0 || ps[0] == ' ')
                        break;
                    cmd_str[cmd_len++] = *(ps++);
                }
                cmd_str[cmd_len] = 0;
                while (*ps == ' ')
                    ps++;

                if (cmd_str[0] == 0) {
                    // Nothing entered
                } else {
                    const command_t *cmd = commands;
                    while (cmd->cmd) {
                        if (strcmp(cmd_str, cmd->cmd) == 0)
                            break;
                        cmd++;
                    }

                    if (cmd->handler) {
                        cmd->handler(ps);
                    } else {
                        do_lua(ctx.buf);
                    }
                }
                break;
            }
            if (result == -2) {
                edit_state.editing = true;
                save_vram();
                return;
            }
        }
    }
}
