#include <aqplus.h>
#include <file_io.h>
#include <esp.h>
#include "terminal.h"

// #define DEBUG

static int8_t telnet_fd;

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
    TN_OPT_TERM_TYPE        = 24,
    TN_OPT_TERM_WINDOW_SIZE = 31,
    TN_OPT_TERM_SPEED       = 32,
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
    uint8_t response[3] = {TN_CMD_IAC, TN_CMD_WILL, val};

    will = (val == TN_OPT_TERM_TYPE);

    if (will) {
        response[1] = TN_CMD_WILL;

    } else {
#ifdef DEBUG
        printf("> Unhandled telnet DO: %d\r\n", val);
#endif

        // Send WONT response
        response[1] = TN_CMD_WONT;
    }
    esp_write(telnet_fd, response, 3);
}

static void telnet_will(uint8_t val) {
#ifdef DEBUG
    printf("- Telnet WILL: %d\r\n", val);
#endif
}

static void telnet_sub_neg(uint8_t cmd[], int len) {
    if (len == 2 && cmd[0] == TN_OPT_TERM_TYPE && cmd[1] == 1) {
        uint8_t response[] = {
            TN_CMD_IAC, TN_CMD_SB,
            TN_OPT_TERM_TYPE, 0,
            'a', 'n', 's', 'i',
            TN_CMD_IAC, TN_CMD_SE};
        esp_write(telnet_fd, response, sizeof(response));
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
        // printf("%d\r\n", ch);

        if (cmd_idx < sizeof(cmd))
            cmd[cmd_idx++] = ch;

        switch (cmd[1]) {
            case TN_CMD_DO:
                if (cmd_idx == 3) {
                    telnet_do(ch);
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
        case CH_UP: esp_write(telnet_fd, "\x1B[A", 3); break;
        case CH_DOWN: esp_write(telnet_fd, "\x1B[B", 3); break;
        case CH_RIGHT: esp_write(telnet_fd, "\x1B[C", 3); break;
        case CH_LEFT: esp_write(telnet_fd, "\x1B[D", 3); break;
        default:
            esp_write(telnet_fd, &ch, 1);
            break;
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

    // Title bar
    {
        IO_VCTRL = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
        memset((uint8_t *)0x3000, ' ', 80);
        memcpy((uint8_t *)0x3000, "Telnet", 6);

        IO_VCTRL = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
        memset((uint8_t *)0x3000, 0x74, 80);
    }

    int8_t telnet_fd = esp_open("tcp://192.168.178.10:23", 0);
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
                uint8_t *p = buf;
                while (len--) {
                    process_char(*(p++));
                }
            }
        }
        esp_close(telnet_fd);
        printf("\r\n\r\nConnection lost.\r\n");
    }
    return 0;
}
