#include "USBHost.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_intr_alloc.h>
#include "AqKeyboard.h"
#include "HID.h"

static const char *TAG = "USBHost";

#define DAEMON_TASK_PRIORITY 2
#define CLASS_TASK_PRIORITY 3
#define CLIENT_NUM_EVENT_MSG 5
#define USB_REQUEST_SET_REPORT 0x09
#define USB_REQUEST_SET_PROTOCOL 0x0B

enum {
    ACTION_OPEN_DEV  = (1 << 0),
    ACTION_CLOSE_DEV = (1 << 1),
};

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

    xTaskCreatePinnedToCore(_taskUSBEvents, "USBEvents", 4096, this, DAEMON_TASK_PRIORITY, nullptr, 0);

    // Create class driver task
    xTaskCreatePinnedToCore(_taskClassDriver, "class", 4096, this, CLASS_TASK_PRIORITY, nullptr, 0);
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
        .max_num_event_msg = CLIENT_NUM_EVENT_MSG,
        .async             = {.client_event_callback = _clientEventCb, .callback_arg = this},
    };
    ESP_ERROR_CHECK(usb_host_client_register(&clientCfg, &clientHdl));

    xTaskCreatePinnedToCore(_taskClient, "client", 4096, this, CLASS_TASK_PRIORITY, &clientTask, 0);

    while (1) {
        usb_host_client_handle_events(clientHdl, portMAX_DELAY);
    }
}

void USBHost::_transferCb(usb_transfer_t *transfer) {
    static_cast<USBHost *>(transfer->context)->transferCb(transfer);
}

void USBHost::transferCb(usb_transfer_t *transfer) {
    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        if (transfer->data_buffer_size == 8) {
            handleKeyboardData(transfer->data_buffer);
        }

    } else if (transfer->status == USB_TRANSFER_STATUS_NO_DEVICE) {
        // Device is removed, free transfer
        ESP_LOGI(TAG, "transferCb - no device");
        usb_host_transfer_free(transfer);
        return;
    }

    // Retransmit transfer to get next data
    usb_host_transfer_submit(transfer);
}

void USBHost::_ctrlTransferCb(usb_transfer_t *transfer) {
    static_cast<USBHost *>(transfer->context)->ctrlTransferCb(transfer);
}

void USBHost::ctrlTransferCb(usb_transfer_t *transfer) {
    ESP_LOGI(TAG, "ctrlTransferCb - status: %u", transfer->status);
    usb_host_transfer_free(transfer);
}

void USBHost::keyboardSetLeds(uint8_t leds) {
#if 0
    usb_transfer_t *transfer;

    ESP_ERROR_CHECK(usb_host_transfer_alloc(sizeof(usb_setup_packet_t) + 1, 0, &transfer));
    transfer->device_handle    = driver_obj.dev_hdl;
    transfer->bEndpointAddress = 0;
    transfer->callback         = _ctrlTransferCb;
    transfer->context          = this;
    transfer->timeout_ms       = 1000;
    transfer->num_bytes        = sizeof(usb_setup_packet_t) + 1;

    usb_setup_packet_t *req = (usb_setup_packet_t *)(transfer->data_buffer);
    req->bmRequestType      = USB_BM_REQUEST_TYPE_DIR_OUT | USB_BM_REQUEST_TYPE_TYPE_CLASS | USB_BM_REQUEST_TYPE_RECIP_INTERFACE;
    req->bRequest           = USB_REQUEST_SET_REPORT;
    req->wValue             = (2 << 8) | 0;
    req->wIndex             = 0;
    req->wLength            = 1;

    uint8_t *start_of_data = (uint8_t *)req + sizeof(usb_setup_packet_t);
    start_of_data[0]       = leds;

    esp_err_t err = usb_host_transfer_submit_control(driver_obj.client_hdl, transfer);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "usb_host_transfer_submit_control: %s", esp_err_to_name(err));
        usb_host_transfer_free(transfer);
    }
#endif
}
