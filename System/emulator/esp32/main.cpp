#include "Common.h"
#include "USBHost.h"
#include "FPGA.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "DisplayOverlay/DisplayOverlay.h"
#include <nvs_flash.h>
#include "VFS.h"
#include "FpgaCore.h"
#ifndef EMULATOR
#include "WiFi.h"
#include "Bluetooth.h"
#include "FileServer.h"
#include "PowerLED.h"
#include <esp_heap_caps.h>
#endif

static const char *TAG = "main";

extern "C" void app_main(void);

void loadStartupCore() {
    bool         coreOk = false;
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
        char   corePath[128];
        size_t len = sizeof(corePath);
        if (nvs_get_str(h, "core", corePath, &len) == ESP_OK) {
            if (FpgaCore::loadCore(corePath))
                coreOk = true;
        }
        nvs_close(h);
    }

    if (!coreOk)
        FpgaCore::loadAqPlus();
}

void app_main(void) {
    ESP_LOGI(TAG, "Aquarius+ ESP32 firmware");

#ifndef EMULATOR
    // Init power LED
    getPowerLED()->init();

    // Initialize the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
#endif

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());

        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Initialize timezone, keyboard layout
    {
        nvs_handle_t h;
        if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
#ifndef EMULATOR
            char   tz[128];
            size_t len = sizeof(tz);
            if (nvs_get_str(h, "tz", tz, &len) == ESP_OK) {
                setenv("TZ", tz, 1);
            }
#endif

            uint8_t kblayout = 0;
            if (nvs_get_u8(h, "kblayout", &kblayout) == ESP_OK) {
                Keyboard::instance()->setKeyLayout((KeyLayout)kblayout);
            }
            nvs_close(h);
        }
    }

    getSDCardVFS()->init();
    getUSBHost()->init();
#ifndef EMULATOR
    getWiFi()->init();
    getBluetooth()->init();
#endif
    UartProtocol::instance()->init();

#ifndef EMULATOR
    {
        bool         fileServerOn = false;
        nvs_handle_t h;
        if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
            uint8_t val8 = 0;
            if (nvs_get_u8(h, "fileserver", &val8) == ESP_OK) {
                fileServerOn = (val8 != 0);
            }
            nvs_close(h);
        }
        if (fileServerOn)
            getFileServer()->start();
    }
#endif

    FPGA::instance()->init();
    loadStartupCore();
    getDisplayOverlay()->init();

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
}
