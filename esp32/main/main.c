#include "common.h"
#include "usbhost.h"
#include "sdcard.h"
#include "fpga.h"
#include "uart_protocol.h"
#include "ca_store.h"
#include "wifi.h"

#include <nvs_flash.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());

        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Initialize the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initial global CA store
    ca_store_init();

    // Wi-Fi init
    wifi_init();

    sdcard_init();
    usbhost_init();
    uart_protocol_init();
    fpga_init();
}

void app_main(void) {
    init();

    // while (1) {
    //     heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    // }
}
