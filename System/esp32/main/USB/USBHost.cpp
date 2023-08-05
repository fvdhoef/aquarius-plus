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
        // xTaskNotify(clientTask, (eventMsg->new_dev.address << 4) | 2, eSetBits);

        // xEventGroupSetBits(usb_flags, DEVICE_DISCONNECTED);
    }

    // switch (eventMsg->event) {
    //     case USB_HOST_CLIENT_EVENT_NEW_DEV:
    //         if (driver_obj.dev_addr == 0) {
    //             driver_obj.dev_addr = eventMsg->new_dev.address;
    //             // Open the device next
    //             driver_obj.actions |= ACTION_OPEN_DEV;
    //         }
    //         break;

    //     case USB_HOST_CLIENT_EVENT_DEV_GONE:
    //         if (driver_obj.dev_hdl != nullptr) {
    //             // Cancel any other actions and close the device next
    //             driver_obj.actions = ACTION_CLOSE_DEV;
    //         }
    //         break;

    //     default:
    //         // Should never occur
    //         abort();
    // }
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
        }
    }

#if 0
        if (driver_obj.actions & ACTION_OPEN_DEV) {
            assert(driver_obj.dev_addr != 0);


                ESP_LOGI(TAG, "Opening device at address %d", driver_obj.dev_addr);
                ESP_ERROR_CHECK(usb_host_device_open(driver_obj.client_hdl, driver_obj.dev_addr, &driver_obj.dev_hdl));
                assert(driver_obj.dev_hdl != nullptr);

                const usb_device_desc_t *dev_desc;
                ESP_ERROR_CHECK(usb_host_device_info(driver_obj.dev_hdl, &driver_obj.dev_info));
                ESP_ERROR_CHECK(usb_host_get_device_descriptor(driver_obj.dev_hdl, &dev_desc));
                ESP_ERROR_CHECK(usb_host_get_active_config_descriptor(driver_obj.dev_hdl, &driver_obj.cfg_desc));

                usb_keyboard_info_t ki;
                if (findKeyboardInfo((const uint8_t *)driver_obj.cfg_desc, &ki)) {
                    ESP_LOGI(TAG, "USB Keyboard found!");

                    // Claim interface
                    bInterfaceNumber = ki.bInterfaceNumber;
                    ESP_ERROR_CHECK(usb_host_interface_claim(driver_obj.client_hdl, driver_obj.dev_hdl, ki.bInterfaceNumber, ki.bAlternateSetting));
                    assert(driver_obj.dev_hdl != nullptr);

                    keyboardSetBootProtocol();
                    USBHost::keyboardSetLeds(0);

                    ESP_LOGI(TAG, "Starting transfer");
                    usb_transfer_t *transfer;
                    ESP_ERROR_CHECK(usb_host_transfer_alloc(8, 0, &transfer));
                    transfer->device_handle    = driver_obj.dev_hdl;
                    transfer->bEndpointAddress = ki.bEndpointAddress;
                    transfer->num_bytes        = 8;
                    transfer->callback         = _transferCb;
                    transfer->context          = this;
                    transfer->timeout_ms       = 1000;

                    esp_err_t err = usb_host_transfer_submit(transfer);
                    if (err != ESP_OK) {
                        ESP_LOGI(TAG, "usb_host_transfer_submit: %s", esp_err_to_name(err));
                    }
                }
        // Nothing to do until the device disconnects
        driver_obj.actions &= ~ACTION_OPEN_DEV;
    }

    if (driver_obj.actions & ACTION_CLOSE_DEV) {
        if (bInterfaceNumber >= 0)
            usb_host_interface_release(driver_obj.client_hdl, driver_obj.dev_hdl, bInterfaceNumber);
        bInterfaceNumber = -1;

        usb_host_device_close(driver_obj.client_hdl, driver_obj.dev_hdl);
        driver_obj.dev_addr = 0;
        driver_obj.dev_hdl  = nullptr;
        driver_obj.cfg_desc = nullptr;
        driver_obj.actions &= ~ACTION_CLOSE_DEV;
    }
#endif
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

void USBHost::keyboardSetBootProtocol() {
#if 0
    usb_transfer_t *transfer;

    ESP_ERROR_CHECK(usb_host_transfer_alloc(sizeof(usb_setup_packet_t), 0, &transfer));
    transfer->device_handle    = driver_obj.dev_hdl;
    transfer->bEndpointAddress = 0;
    transfer->callback         = _ctrlTransferCb;
    transfer->context          = this;
    transfer->timeout_ms       = 1000;
    transfer->num_bytes        = sizeof(usb_setup_packet_t);

    usb_setup_packet_t *req = (usb_setup_packet_t *)(transfer->data_buffer);
    req->bmRequestType      = USB_BM_REQUEST_TYPE_DIR_OUT | USB_BM_REQUEST_TYPE_TYPE_CLASS | USB_BM_REQUEST_TYPE_RECIP_INTERFACE;
    req->bRequest           = USB_REQUEST_SET_PROTOCOL;
    req->wValue             = 0;
    req->wIndex             = 0;
    req->wLength            = 0;

    esp_err_t err = usb_host_transfer_submit_control(driver_obj.client_hdl, transfer);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "usb_host_transfer_submit_control: %s", esp_err_to_name(err));
        usb_host_transfer_free(transfer);
    }
#endif
}

bool USBHost::findKeyboardInfo(const uint8_t *desc, usb_keyboard_info_t *ki) {
    if (!(desc[0] == 9 && desc[1] == 0x02)) {
        return false;
    }

    const uint8_t *p           = desc;
    uint16_t       totalLength = (desc[3] << 8) | desc[2];

    uint8_t bInterfaceNumber   = 0;
    uint8_t bAlternateSetting  = 0;
    uint8_t bInterfaceClass    = 0;
    uint8_t bInterfaceSubClass = 0;
    uint8_t bInterfaceProtocol = 0;
    bool    is_keyboard        = false;

    while ((p - desc) < totalLength) {
        uint8_t length = p[0];
        uint8_t type   = p[1];

        if (type == 0x04) { // Interface
            bInterfaceNumber   = p[2];
            bAlternateSetting  = p[3];
            bInterfaceClass    = p[5];
            bInterfaceSubClass = p[6];
            bInterfaceProtocol = p[7];
            is_keyboard        = bInterfaceClass == 3 && bInterfaceSubClass == 1 && bInterfaceProtocol == 1;

        } else if (type == 0x05) { // Endpoint
            uint8_t  bEndpointAddress = p[2];
            uint8_t  bmAttributes     = p[3];
            uint16_t wMaxPacketSize   = (p[5] << 8) | p[4];
            uint8_t  bInterval        = p[6];

            if (is_keyboard && (bEndpointAddress & 0xF0) == 0x80 && (bmAttributes & 3) == 3) {
                ki->bInterfaceNumber  = bInterfaceNumber;
                ki->bAlternateSetting = bAlternateSetting;
                ki->bEndpointAddress  = bEndpointAddress;
                ki->wMaxPacketSize    = wMaxPacketSize;
                ki->bInterval         = bInterval;
                return true;
            }
        }
        p += length;
    }
    return false;
}
