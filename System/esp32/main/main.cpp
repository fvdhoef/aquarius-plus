#include "Common.h"
#include "USBHost.h"
#include "SDCardVFS.h"
#include "FPGA.h"
#include "AqUartProtocol.h"
#include "WiFi.h"
#include "BLE.h"
#include "FileServer.h"
#include "AqKeyboard.h"
#include "PowerLED.h"

#include <nvs_flash.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "main";

static void init(void) {
    // Init power LED
    PowerLED::instance().init();

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

    WiFi::instance().init();
    BLE::instance().init();

    AqKeyboard::instance().init();
    SDCardVFS::instance().init();
    USBHost::instance().init();
    AqUartProtocol::instance().init();
    FileServer::instance().init();

    {
        auto &fpga = FPGA::instance();
        fpga.init();

        extern const uint8_t fpga_image_start[] asm("_binary_top_bit_start");
        extern const uint8_t fpga_image_end[] asm("_binary_top_bit_end");

        fpga.loadBitstream(fpga_image_start, fpga_image_end - fpga_image_start);
    }
}

extern "C" void app_main(void);

void app_main(void) {
    init();

#if 0
    while (1) {
        char *buf = (char *)malloc(16384);
        if (buf) {
            vTaskList(buf);
            printf(
                "Name           State  Priority  Stack   Num     Core\n"
                "----------------------------------------------------\n"
                "%s\n",
                buf);
            free(buf);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
#endif
    // while (1) {
    //     heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    // }
}
