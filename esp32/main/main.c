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
    // usbhost_init();
}

void app_main(void) {
    init();
}
