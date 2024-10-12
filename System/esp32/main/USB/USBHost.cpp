#include "USBHost.h"
#include <esp_intr_alloc.h>
#include <usb/usb_host.h>
#include <USBDevice.h>

static const char *TAG = "USBHost";

class USBHostInt : public USBHost {
public:
    usb_host_client_handle_t                    clientHdl    = nullptr;
    TaskHandle_t                                clientTask   = nullptr;
    SemaphoreHandle_t                           devicesMutex = nullptr;
    std::map<usb_device_handle_t, USBDevicePtr> devices;

    USBHostInt() {
    }

    void init() override {
        devicesMutex = xSemaphoreCreateRecursiveMutex();

        // Install USB host library
        ESP_LOGI(TAG, "Installing USB Host Library");
        usb_host_config_t hostCfg = {.intr_flags = ESP_INTR_FLAG_LEVEL1};
        ESP_ERROR_CHECK(usb_host_install(&hostCfg));

        if (xTaskCreatePinnedToCore(_taskUSBEvents, "USBEvents", 4096, this, 2, nullptr, 0) != pdPASS) {
            ESP_LOGE(TAG, "Error creating USBEvents task");
        }

        // Create class driver task
        if (xTaskCreatePinnedToCore(_taskClassDriver, "class", 4096, this, 3, nullptr, 0) != pdPASS) {
            ESP_LOGE(TAG, "Error creating class task");
        }
        vTaskDelay(10); // Short delay to let client task spin up
    }

    static void _taskUSBEvents(void *arg) {
        static_cast<USBHostInt *>(arg)->taskUSBEvents();
    }

    void taskUSBEvents() {
        while (1) {
            ESP_ERROR_CHECK(usb_host_lib_handle_events(portMAX_DELAY, nullptr));
        }
    }

    static void _clientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg) {
        static_cast<USBHostInt *>(arg)->clientEventCb(eventMsg);
    }

    void clientEventCb(const usb_host_client_event_msg_t *eventMsg) {
        if (eventMsg->event == USB_HOST_CLIENT_EVENT_NEW_DEV) {
            xTaskNotify(clientTask, (eventMsg->new_dev.address << 4) | 1, eSetBits);
        } else if (eventMsg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
            RecursiveMutexLock lock(devicesMutex);
            devices.erase(eventMsg->dev_gone.dev_hdl);
        }
    }

    static void _taskClassDriver(void *arg) {
        static_cast<USBHostInt *>(arg)->taskClassDriver();
    }

    static void _taskClient(void *arg) {
        static_cast<USBHostInt *>(arg)->taskClient();
    }

    void taskClient() {
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

    void taskClassDriver() {
        ESP_LOGI(TAG, "Registering Client");
        usb_host_client_config_t clientCfg = {
            .is_synchronous    = false,
            .max_num_event_msg = 5,
            .async             = {.client_event_callback = _clientEventCb, .callback_arg = this},
        };
        ESP_ERROR_CHECK(usb_host_client_register(&clientCfg, &clientHdl));

        if (xTaskCreatePinnedToCore(_taskClient, "client", 8192, this, 3, &clientTask, 0) != pdPASS) {
            ESP_LOGE(TAG, "Error creating client task");
        }

        while (1) {
            usb_host_client_handle_events(clientHdl, portMAX_DELAY);
        }
    }

    void keyboardSetLeds(uint8_t leds) override {
        RecursiveMutexLock lock(devicesMutex);
        for (auto it = devices.begin(); it != devices.end(); ++it) {
            auto device = it->second;
            device->setLeds(leds);
        }
    }
};

USBHost *getUSBHost() {
    static USBHostInt obj;
    return &obj;
}
