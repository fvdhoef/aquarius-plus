#include "USBHost.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_intr_alloc.h>
#include "AqKeyboard.h"
#include "HID.h"

static const char *TAG = "USBHost";

USBHost::USBHost() {
}

USBHost &USBHost::instance() {
    static USBHost obj;
    return obj;
}

void USBHost::init() {
    devicesMutex = xSemaphoreCreateRecursiveMutex();

    // Install USB host library
    ESP_LOGI(TAG, "Installing USB Host Library");
    usb_host_config_t hostCfg = {.intr_flags = ESP_INTR_FLAG_LEVEL1};
    ESP_ERROR_CHECK(usb_host_install(&hostCfg));

    xTaskCreatePinnedToCore(_taskUSBEvents, "USBEvents", 4096, this, 2, nullptr, 0);

    // Create class driver task
    xTaskCreatePinnedToCore(_taskClassDriver, "class", 4096, this, 3, nullptr, 0);
    vTaskDelay(10); // Short delay to let client task spin up
}

void USBHost::_taskUSBEvents(void *arg) {
    static_cast<USBHost *>(arg)->taskUSBEvents();
}

void USBHost::taskUSBEvents() {
    while (1) {
        ESP_ERROR_CHECK(usb_host_lib_handle_events(portMAX_DELAY, nullptr));
    }
}

void USBHost::_clientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg) {
    static_cast<USBHost *>(arg)->clientEventCb(eventMsg);
}

void USBHost::clientEventCb(const usb_host_client_event_msg_t *eventMsg) {
    if (eventMsg->event == USB_HOST_CLIENT_EVENT_NEW_DEV) {
        xTaskNotify(clientTask, (eventMsg->new_dev.address << 4) | 1, eSetBits);
    } else if (eventMsg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
        RecursiveMutexLock lock(devicesMutex);
        devices.erase(eventMsg->dev_gone.dev_hdl);
    }
}

void USBHost::_taskClassDriver(void *arg) {
    static_cast<USBHost *>(arg)->taskClassDriver();
}

void USBHost::_taskClient(void *arg) {
    static_cast<USBHost *>(arg)->taskClient();
}

void USBHost::taskClient() {
    // int bInterfaceNumber = -1;

    while (1) {
        uint32_t notifiedValue;
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, portMAX_DELAY);

        if (notifiedValue & 1) {
            // New device
            unsigned devAddr = (notifiedValue >> 4) & 0x7F;

            auto devPtr = USBDevice::init(clientHdl, devAddr);
            if (devPtr) {
                RecursiveMutexLock lock(devicesMutex);
                devices.insert(std::pair(devPtr->getHandle(), devPtr));
            }
        }
    }
}

void USBHost::taskClassDriver() {
    ESP_LOGI(TAG, "Registering Client");
    usb_host_client_config_t clientCfg = {
        .is_synchronous    = false,
        .max_num_event_msg = 5,
        .async             = {.client_event_callback = _clientEventCb, .callback_arg = this},
    };
    ESP_ERROR_CHECK(usb_host_client_register(&clientCfg, &clientHdl));

    xTaskCreatePinnedToCore(_taskClient, "client", 8192, this, 3, &clientTask, 0);

    while (1) {
        usb_host_client_handle_events(clientHdl, portMAX_DELAY);
    }
}

void USBHost::keyboardSetLeds(uint8_t leds) {
    RecursiveMutexLock lock(devicesMutex);
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        auto device = it->second;
        device->setLeds(leds);
    }
}
