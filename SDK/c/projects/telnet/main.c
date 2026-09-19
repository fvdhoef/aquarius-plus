#include <aqplus.h>
#include <file_io.h>
#include <esp.h>
#include "terminal.h"

// #define DEBUG

static int8_t telnet_fd;

uint8_t term_flags;

int putchar(int c) {
    terminal_putchar(c);
    return c;
}

static const uint16_t palette[16] = {
    0x000, 0xB00, 0x0B0, 0xBB0, 0x00B, 0xB0B, 0x0BB, 0xBBB,
    0x555, 0xF55, 0x5F5, 0xFF5, 0x55F, 0xF5F, 0x5FF, 0xFFF};

enum {
    TN_CMD_SE                = 240, // Subnegotiation end
    TN_CMD_NOP               = 241,
    TN_CMD_DATA_MARK         = 242,
    TN_CMD_BREAK             = 243,
    TN_CMD_INTERRUPT_PROCESS = 244,
    TN_CMD_ABORT_OUTPUT      = 245,
    TN_CMD_ARE_YOU_THERE     = 246,
    TN_CMD_ERASE_CHARACTER   = 247,
    TN_CMD_ERASE_LINE        = 248,
    TN_CMD_GO_AHEAD          = 249,
    TN_CMD_SB                = 250, // Subnegotiation begin
    TN_CMD_WILL              = 251,
    TN_CMD_WONT              = 252,
    TN_CMD_DO                = 253,
    TN_CMD_DONT              = 254,
    TN_CMD_IAC               = 255, // Interpret As Command
};

enum {
    TN_OPT_ECHO                = 1,
    TN_OPT_SUPPRESS_GO_AHEAD   = 3,
    TN_OPT_STATUS              = 5,
    TN_OPT_TERM_TYPE           = 24,
    TN_OPT_TERM_WINDOW_SIZE    = 31,
    TN_OPT_TERM_SPEED          = 32,
    TN_OPT_REMOTE_FLOW_CONTROL = 33,
    TN_OPT_LINEMODE            = 34,
    TN_OPT_XDISPLAY            = 35,
    TN_OPT_NEW_ENV_VAR         = 39,
};

enum {
    CH_BACKSPACE = 8,
    CH_TAB       = 9,
    CH_RETURN    = 13,
    CH_DELETE    = 127,
    CH_PGUP      = 0x8A,
    CH_PGDN      = 0x8B,
    CH_RIGHT     = 0x8E,
    CH_UP        = 0x8F,
    CH_END       = 0x9A,
    CH_HOME      = 0x9B,
    CH_INSERT    = 0x9D,
    CH_LEFT      = 0x9E,
    CH_DOWN      = 0x9F,
    CH_F1        = 0x80,
    CH_F2        = 0x81,
    CH_F3        = 0x82,
    CH_F4        = 0x83,
    CH_F5        = 0x84,
    CH_F6        = 0x85,
    CH_F7        = 0x86,
    CH_F8        = 0x87,
    CH_F9        = 0x90,
    CH_F10       = 0x91,
    CH_F11       = 0x92,
    CH_F12       = 0x93,
};

static void telnet_do(uint8_t val) {
    bool    will        = false;
    bool    wont        = false;
    uint8_t response[3] = {TN_CMD_IAC, TN_CMD_WILL, val};

    will =
        (val == TN_OPT_TERM_TYPE ||
         val == TN_OPT_TERM_SPEED ||
         val == TN_OPT_NEW_ENV_VAR);
    wont =
        (val == TN_OPT_XDISPLAY ||
         val == TN_OPT_REMOTE_FLOW_CONTROL ||
         val == TN_OPT_ECHO);

    if (will) {
        response[1] = TN_CMD_WILL;
    } else {
        if (!wont) {
#ifdef DEBUG
            printf("> Unhandled telnet DO: %d\r\n", val);
#endif
        }

        // Send WONT response
        response[1] = TN_CMD_WONT;
    }
    esp_write(telnet_fd, response, 3);
}

static void telnet_will(uint8_t val) {
    (void)val;
#ifdef DEBUG
    printf("- Telnet WILL: %d\r\n", val);
#endif
}

static void send_sub_neg(uint8_t cmd[], int cmd_len, uint8_t data[], int data_len) {
    uint8_t response[] = {TN_CMD_IAC, TN_CMD_SB};
    esp_write(telnet_fd, response, sizeof(response));
    esp_write(telnet_fd, cmd, cmd_len);

    if (data_len < 0)
        data_len = strlen(data);
    if (data_len > 0)
        esp_write(telnet_fd, data, data_len);

    response[1] = TN_CMD_SE;
    esp_write(telnet_fd, response, sizeof(response));
}

static void telnet_sub_neg(uint8_t cmd[], int len) {
    if (len == 2 && cmd[0] == TN_OPT_TERM_TYPE && cmd[1] == 1) {
        uint8_t resp_cmd[] = {TN_OPT_TERM_TYPE, 0};
        send_sub_neg(resp_cmd, sizeof(resp_cmd), "XTERM-COLOR", -1);
        // send_sub_neg(resp_cmd, sizeof(resp_cmd), "xterm-16color", -1);
        // send_sub_neg(resp_cmd, sizeof(resp_cmd), "ansi", -1);
        // send_sub_neg(resp_cmd, sizeof(resp_cmd), "linux", -1);

    } else if (len == 2 && cmd[0] == TN_OPT_TERM_SPEED && cmd[1] == 1) {
        uint8_t resp_cmd[] = {TN_OPT_TERM_SPEED, 0};
        send_sub_neg(resp_cmd, sizeof(resp_cmd), "38400,38400", -1);

    } else if (len == 2 && cmd[0] == TN_OPT_NEW_ENV_VAR && cmd[1] == 1) {
        uint8_t resp_cmd[] = {TN_OPT_NEW_ENV_VAR, 0};
        send_sub_neg(resp_cmd, sizeof(resp_cmd), NULL, 0);

    } else {

#ifdef DEBUG
        printf("Unhandled sub neg: %d\r\n", cmd[0]);

#endif
    }
    // for (int i = 0; i < len; i++) {
    //     printf("SB: %d\r\n", cmd[i]);
    // }
}

static uint8_t cmd_idx = 0;
static uint8_t cmd[16];

static bool escape = false;

static void process_char(uint8_t ch) {
    if (cmd_idx) {
        // printf("%02X ", ch);

        if (cmd_idx < sizeof(cmd))
            cmd[cmd_idx++] = ch;

        switch (cmd[1]) {
            case TN_CMD_DO:
                if (cmd_idx == 3) {
                    telnet_do(ch);
                    cmd_idx = 0;
                }
                break;

            case TN_CMD_DONT:
                if (cmd_idx == 3) {
                    cmd_idx = 0;
                }
                break;

            case TN_CMD_WILL:
                if (cmd_idx == 3) {
                    telnet_will(ch);
                    cmd_idx = 0;
                }
                break;

            case TN_CMD_IAC:
                terminal_putchar(255);
                cmd_idx = 0;
                break;

            case TN_CMD_SB:
                if (ch == TN_CMD_SE && cmd[cmd_idx - 2] == TN_CMD_IAC) {
                    telnet_sub_neg(cmd + 2, cmd_idx - 4);
                    cmd_idx = 0;
                }
                break;

            case TN_CMD_DATA_MARK:
                cmd_idx = 0;
                break;

            default:
#ifdef DEBUG
                printf("Unknown telnet command: %d\r\n", cmd[1]);
#endif
                cmd_idx = 0;
                break;
        }

        return;
    }
    if (ch == TN_CMD_IAC) {
        cmd[cmd_idx++] = ch;
        return;
    }
    terminal_putchar(ch);
}

static void process_keyboard(uint8_t ch) {
    switch (ch) {
        case CH_UP: esp_write(telnet_fd, (term_flags & 1) ? "\x1BOA" : "\x1B[A", 3); break;
        case CH_DOWN: esp_write(telnet_fd, (term_flags & 1) ? "\x1BOB" : "\x1B[B", 3); break;
        case CH_RIGHT: esp_write(telnet_fd, (term_flags & 1) ? "\x1BOC" : "\x1B[C", 3); break;
        case CH_LEFT: esp_write(telnet_fd, (term_flags & 1) ? "\x1BOD" : "\x1B[D", 3); break;
        case CH_HOME: esp_write(telnet_fd, "\x1B[H", 3); break;
        case CH_END: esp_write(telnet_fd, "\x1B[F", 3); break;
        case CH_F1: esp_write(telnet_fd, "\x1BOP", 3); break;
        case CH_F2: esp_write(telnet_fd, "\x1BOQ", 3); break;
        case CH_F3: esp_write(telnet_fd, "\x1BOR", 3); break;
        case CH_F4: esp_write(telnet_fd, "\x1BOS", 3); break;
        case CH_INSERT: esp_write(telnet_fd, "\x1B[2~", 4); break;
        case CH_DELETE: esp_write(telnet_fd, "\x1B[3~", 4); break;
        case CH_PGUP: esp_write(telnet_fd, "\x1B[5~", 4); break;
        case CH_PGDN: esp_write(telnet_fd, "\x1B[6~", 4); break;
        case CH_F5: esp_write(telnet_fd, "\x1B[15~", 5); break;
        case CH_F6: esp_write(telnet_fd, "\x1B[17~", 5); break;
        case CH_F7: esp_write(telnet_fd, "\x1B[18~", 5); break;
        case CH_F8: esp_write(telnet_fd, "\x1B[19~", 5); break;
        case CH_F9: esp_write(telnet_fd, "\x1B[20~", 5); break;
        case CH_F10: esp_write(telnet_fd, "\x1B[21~", 5); break;
        case CH_F11: esp_write(telnet_fd, "\x1B[23~", 5); break;
        case CH_F12: esp_write(telnet_fd, "\x1B[24~", 5); break;
        case CH_BACKSPACE: esp_write(telnet_fd, "\x7F", 1); break;
        default: esp_write(telnet_fd, &ch, 1); break;
    }
}

static void connect(const char *uri) {
    printf("\r\nConnecting to %s\r\n", uri);
    terminal_show_cursor(true);

    term_flags       = 2;
    int8_t telnet_fd = esp_open(uri, 0);

    terminal_show_cursor(false);
    if (telnet_fd < 0) {
        printf("Error opening host.\r\n");
    } else {
        printf("Connected to host.\r\n\r\n");

        static uint8_t buf[256];
        while (1) {
            while (1) {
                uint8_t val = IO_KEYBUF;
                if (val == 0)
                    break;
                process_keyboard(val);
            }

            int len = esp_read(telnet_fd, buf, sizeof(buf));
            if (len < 0)
                break;

            if (len > 0) {
                // Hide cursor
                if (term_flags & 2)
                    terminal_show_cursor(false);

                uint8_t *p = buf;
                while (len--) {
                    process_char(*(p++));
                }

                // Show cursor
                if (term_flags & 2)
                    terminal_show_cursor(true);
            }
        }
        esp_close(telnet_fd);

        terminal_show_cursor(false);
        printf("\r\n\r\nConnection lost.\r\n");
    }
}

int main(void) {
    // Enable turbo mode
    IO_SYSCTRL = 4;

    // Enable keyboard repeat
    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    // Redefine palette to ANSI colors
    for (uint8_t i = 0; i < 32; i++) {
        IO_VPALSEL  = i;
        IO_VPALDATA = ((const uint8_t *)palette)[i];
    }

    terminal_init();

    static char uri[128];

    while (1) {
        // Title bar
        {
            IO_VCTRL = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
            memset((uint8_t *)0x3000, ' ', 80);
            memcpy((uint8_t *)0x3000, "Telnet", 6);

            IO_VCTRL = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
            memset((uint8_t *)0x3000, 0x74, 80);
        }

        printf("\r\nEnter address (hostname:port): ");
        strcpy(uri, "tcp://");
        char *p       = uri + strlen(uri);
        int   max_len = sizeof(uri) - strlen(uri) - 1;
        int   idx     = 0;

        terminal_show_cursor(true);

        term_flags = 0x82;

        while (1) {
            char ch = IO_KEYBUF;
            if (ch == '\b') {
                if (idx > 0) {
                    idx--;
                    terminal_show_cursor(false);
                    putchar('\b');
                    putchar(' ');
                    putchar('\b');
                    terminal_show_cursor(true);
                }
            } else if (ch == '\r') {
                terminal_show_cursor(false);
                putchar('\r');
                putchar('\n');
                break;
            } else if (ch <= ' ') {
                continue;
            } else {
                if (idx < max_len) {
                    terminal_show_cursor(false);
                    putchar(ch);
                    terminal_show_cursor(true);
                    p[idx++] = ch;
                }
            }
        }
        p[idx] = 0;

        // Check for ':'
        {
            bool        has_port = false;
            const char *p2       = p;
            while (*p2) {
                if (*p2 == ':') {
                    has_port = true;
                }
                p2++;
            }

            if (!has_port) {
                // Append :23
                if (strlen(uri) + 3 + 1 < sizeof(uri)) {
                    strcat(uri, ":23");
                }
            }
        }

        for (int i = 0; i < 70; i++) {
            if (!p[i])
                break;

            IO_VCTRL                     = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
            *(uint8_t *)(0x3000 + 7 + i) = p[i];
        }

        connect(uri);
    }
    return 0;
}
