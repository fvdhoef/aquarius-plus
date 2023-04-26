#include "tcpserver.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include "aq_keyb.h"
#include "aq_keyb_defs.h"

static const char *TAG = "tcpserver";

#define PORT (23)

#define SHFT (1 << 7)
#define CTRL (1 << 6)

static uint8_t scancode_lut[] = {
    0,                                 //  0 CTRL-@
    0,                                 //  1 CTRL-A
    0,                                 //  2 CTRL-B
    CTRL | SDL_SCANCODE_C,             //  3 CTRL-C
    0,                                 //  4 CTRL-D
    0,                                 //  5 CTRL-E
    0,                                 //  6 CTRL-F
    CTRL | SDL_SCANCODE_G,             //  7 CTRL-G
    0,                                 //  8 CTRL-H
    0,                                 //  9 CTRL-I
    SDL_SCANCODE_RETURN,               // 10 CTRL-J \n
    0,                                 // 11 CTRL-K
    0,                                 // 12 CTRL-L
    0,                                 // 13 CTRL-M \r
    0,                                 // 14 CTRL-N
    0,                                 // 15 CTRL-O
    0,                                 // 16 CTRL-P
    0,                                 // 17 CTRL-Q
    0,                                 // 18 CTRL-R
    0,                                 // 19 CTRL-S
    0,                                 // 20 CTRL-T
    0,                                 // 21 CTRL-U
    0,                                 // 22 CTRL-V
    0,                                 // 23 CTRL-W
    0,                                 // 24 CTRL-X
    0,                                 // 25 CTRL-Y
    0,                                 // 26 CTRL-Z
    0,                                 // 27 CTRL-[
    0,                                 // \x1C 28 CTRL-backslash
    0,                                 // \x1D 29 CTRL-]
    CTRL | SDL_SCANCODE_ESCAPE,        // \x1E 30 CTRL-^
    CTRL | SHFT | SDL_SCANCODE_ESCAPE, // \x1F 31 CTRL-_
    SDL_SCANCODE_SPACE,                // Space
    SHFT | SDL_SCANCODE_1,             // !
    SHFT | SDL_SCANCODE_APOSTROPHE,    // "
    SHFT | SDL_SCANCODE_3,             // #
    SHFT | SDL_SCANCODE_4,             // $
    SHFT | SDL_SCANCODE_5,             // %
    SHFT | SDL_SCANCODE_7,             // &
    SDL_SCANCODE_APOSTROPHE,           // '
    SHFT | SDL_SCANCODE_9,             // (
    SHFT | SDL_SCANCODE_0,             // )
    SHFT | SDL_SCANCODE_8,             // *
    SHFT | SDL_SCANCODE_EQUALS,        // +
    SDL_SCANCODE_COMMA,                // ,
    SDL_SCANCODE_MINUS,                // -
    SDL_SCANCODE_PERIOD,               // .
    SDL_SCANCODE_SLASH,                // /
    SDL_SCANCODE_0,                    // 0
    SDL_SCANCODE_1,                    // 1
    SDL_SCANCODE_2,                    // 2
    SDL_SCANCODE_3,                    // 3
    SDL_SCANCODE_4,                    // 4
    SDL_SCANCODE_5,                    // 5
    SDL_SCANCODE_6,                    // 6
    SDL_SCANCODE_7,                    // 7
    SDL_SCANCODE_8,                    // 8
    SDL_SCANCODE_9,                    // 9
    SHFT | SDL_SCANCODE_SEMICOLON,     // :
    SDL_SCANCODE_SEMICOLON,            // ;
    SHFT | SDL_SCANCODE_COMMA,         // <
    SDL_SCANCODE_EQUALS,               // =
    SHFT | SDL_SCANCODE_PERIOD,        // >
    SHFT | SDL_SCANCODE_SLASH,         // ?
    SHFT | SDL_SCANCODE_2,             // @
    SHFT | SDL_SCANCODE_A,             // A
    SHFT | SDL_SCANCODE_B,             // B
    SHFT | SDL_SCANCODE_C,             // C
    SHFT | SDL_SCANCODE_D,             // D
    SHFT | SDL_SCANCODE_E,             // E
    SHFT | SDL_SCANCODE_F,             // F
    SHFT | SDL_SCANCODE_G,             // G
    SHFT | SDL_SCANCODE_H,             // H
    SHFT | SDL_SCANCODE_I,             // I
    SHFT | SDL_SCANCODE_J,             // J
    SHFT | SDL_SCANCODE_K,             // K
    SHFT | SDL_SCANCODE_L,             // L
    SHFT | SDL_SCANCODE_M,             // M
    SHFT | SDL_SCANCODE_N,             // N
    SHFT | SDL_SCANCODE_O,             // O
    SHFT | SDL_SCANCODE_P,             // P
    SHFT | SDL_SCANCODE_Q,             // Q
    SHFT | SDL_SCANCODE_R,             // R
    SHFT | SDL_SCANCODE_S,             // S
    SHFT | SDL_SCANCODE_T,             // T
    SHFT | SDL_SCANCODE_U,             // U
    SHFT | SDL_SCANCODE_V,             // V
    SHFT | SDL_SCANCODE_W,             // W
    SHFT | SDL_SCANCODE_X,             // X
    SHFT | SDL_SCANCODE_Y,             // Y
    SHFT | SDL_SCANCODE_Z,             // Z
    SDL_SCANCODE_LEFTBRACKET,          // [
    SDL_SCANCODE_BACKSLASH,            // backslash
    SDL_SCANCODE_RIGHTBRACKET,         // ]
    SHFT | SDL_SCANCODE_6,             // ^
    SHFT | SDL_SCANCODE_MINUS,         // _
    SDL_SCANCODE_GRAVE,                // `
    SDL_SCANCODE_A,                    // a
    SDL_SCANCODE_B,                    // b
    SDL_SCANCODE_C,                    // c
    SDL_SCANCODE_D,                    // d
    SDL_SCANCODE_E,                    // e
    SDL_SCANCODE_F,                    // f
    SDL_SCANCODE_G,                    // g
    SDL_SCANCODE_H,                    // h
    SDL_SCANCODE_I,                    // i
    SDL_SCANCODE_J,                    // j
    SDL_SCANCODE_K,                    // k
    SDL_SCANCODE_L,                    // l
    SDL_SCANCODE_M,                    // m
    SDL_SCANCODE_N,                    // n
    SDL_SCANCODE_O,                    // o
    SDL_SCANCODE_P,                    // p
    SDL_SCANCODE_Q,                    // q
    SDL_SCANCODE_R,                    // r
    SDL_SCANCODE_S,                    // s
    SDL_SCANCODE_T,                    // t
    SDL_SCANCODE_U,                    // u
    SDL_SCANCODE_V,                    // v
    SDL_SCANCODE_W,                    // w
    SDL_SCANCODE_X,                    // x
    SDL_SCANCODE_Y,                    // y
    SDL_SCANCODE_Z,                    // z
    SHFT | SDL_SCANCODE_LEFTBRACKET,   // {
    SHFT | SDL_SCANCODE_BACKSLASH,     // |
    SHFT | SDL_SCANCODE_RIGHTBRACKET,  // }
    SHFT | SDL_SCANCODE_GRAVE,         // ~
};

static void press_key(unsigned ch) {
    if (ch == 0x1C) {
        // Delay for 100ms
        vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }
    if (ch == 0x1D) {
        // Delay for 500ms
        vTaskDelay(pdMS_TO_TICKS(500));
        return;
    }

    if (ch > '~')
        return;
    uint8_t val = scancode_lut[ch];
    if (val == 0)
        return;

    if (val & SHFT)
        keyboard_scancode(SDL_SCANCODE_LSHIFT, true);
    if (val & CTRL)
        keyboard_scancode(SDL_SCANCODE_LCTRL, true);
    keyboard_scancode(val & 0x3F, true);
    keyboard_update_matrix();
    vTaskDelay(pdMS_TO_TICKS(20));

    if (val & SHFT)
        keyboard_scancode(SDL_SCANCODE_LSHIFT, false);
    if (val & CTRL)
        keyboard_scancode(SDL_SCANCODE_LCTRL, false);
    keyboard_scancode(val & 0x3F, false);
    keyboard_update_matrix();
    vTaskDelay(pdMS_TO_TICKS(20));

    // Delay a little longer on reset
    if (ch == 0x1E)
        vTaskDelay(pdMS_TO_TICKS(500));
}

static void do_retransmit(const int sock) {
    int  len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            for (int i = 0; i < len; i++) {
                press_key(rx_buffer[i]);
            }
        }
    } while (len > 0);
}

static void tcpserver_task(void *pvParameters) {
    struct sockaddr_storage dest_addr;
    struct sockaddr_in     *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr        = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family             = AF_INET;
    dest_addr_ip4->sin_port               = htons(PORT);

    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", AF_INET);
        goto done;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto done;
    }

    while (1) {
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr;
        socklen_t               addr_len = sizeof(source_addr);
        int                     sock     = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        char addr_str[32];
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

done:
    close(listen_sock);
    vTaskDelete(NULL);
}

void tcpserver_init(void) {
    xTaskCreate(tcpserver_task, "tcpserver", 4096, NULL, 12, NULL);
}
