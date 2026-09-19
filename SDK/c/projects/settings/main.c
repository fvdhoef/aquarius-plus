#include <aqplus.h>
#include <file_io.h>
#include <esp.h>
#include "screen.h"

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

#define LEFT_PANE_W (21)

void scr_spaces(uint8_t count) {
    while (count--)
        scr_putchar(' ');
}

void empty_line(void) {
    scr_putchar(214);
    while (text_x < LEFT_PANE_W)
        scr_putchar(' ');

    scr_putchar(214);

    while (text_x < 79)
        scr_putchar(' ');
    scr_putchar(214);
}

struct menu_item {
    const char *title;
    void (*detail_pane)(void);
};

static void wifi_pane(void) {
    scr_to_xy(LEFT_PANE_W + 3, 4);
    scr_puts("Status     connected");
    scr_to_xy(LEFT_PANE_W + 3, 6);
    scr_puts("MAC        34:85:18:5e:8c:bc");
    scr_to_xy(LEFT_PANE_W + 3, 8);
    scr_puts("SSID       PikachuBoven2.4");
    scr_to_xy(LEFT_PANE_W + 3, 10);
    scr_puts("Channel    6");
    scr_to_xy(LEFT_PANE_W + 3, 12);
    scr_puts("RSSI       -35 dBm");
    scr_to_xy(LEFT_PANE_W + 3, 14);
    scr_puts("Hostname   aqplus");
    scr_to_xy(LEFT_PANE_W + 3, 16);
    scr_puts("IP         192.168.178.49/24");
    scr_to_xy(LEFT_PANE_W + 3, 18);
    scr_puts("Netmask    255.255.255.0");
    scr_to_xy(LEFT_PANE_W + 3, 20);
    scr_puts("Gateway    192.168.178.1");
    scr_to_xy(LEFT_PANE_W + 3, 22);
    scr_puts("DNS        192.168.178.1");
    scr_puts(", 192.168.178.1");
}

static struct menu_item menu_items[] = {
    {"Wi-Fi settings", wifi_pane},
    {"Bluetooth", NULL},
    // {"Host name", NULL},
    {"Time zone", NULL},
    {"Keyboard", NULL},
    {"Mouse", NULL},
    {"Game controllers", NULL},
    {"System update", NULL},
    {"Factory reset", NULL},
    {"Exit to BASIC", NULL},
};

void clear_right_pane(void) {
    text_color = 0x74;
    for (int j = 3; j < 24; j++) {
        scr_to_xy(LEFT_PANE_W + 1, j);
        while (text_x < 79)
            scr_putchar(' ');
    }
}

void draw_menu(int cur_item) {
    for (int i = 0; i < sizeof(menu_items) / sizeof(menu_items[0]); i++) {
        if (i == cur_item)
            text_color = 0x71;
        else
            text_color = 0x34;

        scr_to_xy(2, 4 + i * 2);
        scr_putchar(' ');
        scr_puts(menu_items[i].title);
        while (text_x < LEFT_PANE_W - 1)
            scr_putchar(' ');
    }
    clear_right_pane();

    if (menu_items[cur_item].detail_pane)
        menu_items[cur_item].detail_pane();
}

int main(void) {
    // Enable turbo mode
    IO_SYSCTRL = 4;

    // Enable keyboard repeat
    esp_send_byte(ESPCMD_KEYMODE);
    esp_send_byte(7);
    esp_get_byte();

    scr_init();

    text_color = 0x74;
    scr_to_xy(0, 0);
    scr_putchar(222);
    while (text_x < 79)
        scr_putchar(172);
    scr_putchar(206);

    scr_putchar(214);
    // const char *title = "Aquarius+ settings";

    scr_spaces(30);
    text_color = 0x34;
    scr_puts("Aquarius+ settings");
    text_color = 0x74;
    while (text_x < 79)
        scr_putchar(' ');
    scr_putchar(214);

    scr_putchar(205);
    while (text_x < LEFT_PANE_W)
        scr_putchar(172);
    scr_putchar(220);
    while (text_x < 79)
        scr_putchar(172);
    scr_putchar(221);

    while (text_y < 24) {
        empty_line();
    }

    scr_putchar(207);
    while (text_x < LEFT_PANE_W)
        scr_putchar(172);
    scr_putchar(204);
    while (text_x < 79)
        scr_putchar(172);
    scr_putchar(223);

    int cur_selected = 0;
    draw_menu(cur_selected);

    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == 0)
            continue;

        int prev_selected = cur_selected;

        switch (scancode) {
            case CH_UP: {
                if (cur_selected > 0)
                    cur_selected--;
                break;
            }
            case CH_DOWN: {
                if (cur_selected < sizeof(menu_items) / sizeof(menu_items[0]) - 1)
                    cur_selected++;
                break;
            }
        }

        if (prev_selected != cur_selected) {
            prev_selected = cur_selected;
            draw_menu(cur_selected);
        }
    }

    return 0;
}
