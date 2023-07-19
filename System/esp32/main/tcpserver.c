#include "tcpserver.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include "aq_keyb.h"

static const char *TAG = "tcpserver";

#define PORT (23)

static void handle_connection(const int sock) {
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
                keyboard_press_key(rx_buffer[i]);
            }
        }
    } while (len > 0);
}

static void tcpserver_task(void *pvParameters) {
    while (1) {
        // Create listen socket
        int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelete(NULL);
            return;
        }

        // Allow reusing address
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        ESP_LOGI(TAG, "Socket created");

        // Bind socket
        struct sockaddr_storage dest_addr;
        struct sockaddr_in     *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr        = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family             = AF_INET;
        dest_addr_ip4->sin_port               = htons(PORT);

        int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            ESP_LOGE(TAG, "IPPROTO: %d", AF_INET);
            goto done;
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        // Listen on socket
        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
            goto done;
        }

        while (1) {
            ESP_LOGI(TAG, "Socket listening");

            // Wait for incoming connection
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

            // Handle connection
            handle_connection(sock);

            // Close socket
            shutdown(sock, 0);
            close(sock);
        }

    done:
        // Close listen socket
        close(listen_sock);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

void tcpserver_init(void) {
    xTaskCreate(tcpserver_task, "tcpserver", 4096, NULL, 12, NULL);
}
