#include "common.h"
#include "usbhost.h"
#include "sdcard.h"
#include "fpga.h"
#include <driver/uart.h>

static const char *TAG = "main";

static void init(void) {
    // Enable power LED
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << IOPIN_LED),
            .mode         = GPIO_MODE_OUTPUT,
        };
        gpio_config(&io_conf);
        gpio_set_level(IOPIN_LED, 1);
    }

    // Initialize UART to FPGA
    {
        const uart_port_t uart_num = UART_NUM_1;

        uart_config_t uart_config = {
            .baud_rate           = 1789773,
            .data_bits           = UART_DATA_8_BITS,
            .parity              = UART_PARITY_DISABLE,
            .stop_bits           = UART_STOP_BITS_1,
            .flow_ctrl           = UART_HW_FLOWCTRL_CTS_RTS,
            .rx_flow_ctrl_thresh = 122,
        };
        ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(uart_num, IOPIN_ESP_RX, IOPIN_ESP_TX, IOPIN_ESP_CTS, IOPIN_ESP_RTS));

        // Setup UART buffered IO with event queue
        const int     uart_buffer_size = (1024 * 2);
        QueueHandle_t uart_queue;
        ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

        uint32_t baudrate;
        ESP_ERROR_CHECK(uart_get_baudrate(uart_num, &baudrate));
        ESP_LOGI(TAG, "Actual baudrate: %u", baudrate);
    }

    fpga_init();

    // sdcard_init();
    usbhost_init();
}

void hexdump(const void *buf, size_t length) {
    size_t         idx = 0;
    const uint8_t *p   = (const uint8_t *)buf;

    while (length > 0) {
        size_t len = length;
        if (len > 16) {
            len = 16;
        }

        printf("%08x  ", idx);

        for (unsigned i = 0; i < 16; i++) {
            if (i < len) {
                printf("%02x ", p[i]);
            } else {
                printf("   ");
            }
            if (i == 7) {
                printf(" ");
            }
        }
        printf(" |");

        for (unsigned i = 0; i < len; i++) {
            printf("%c", (p[i] >= 32 && p[i] <= 126) ? p[i] : '.');
        }
        printf("|\n");

        idx += len;
        length -= len;
        p += len;
    }
}

#define BUF_SIZE (1024)

void app_main(void) {
    init();

    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        hexdump(data, len);
    }
}
