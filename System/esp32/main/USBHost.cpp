#include "USBHost.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_intr_alloc.h>
#include "AqKeyboard.h"

static const char *TAG = "usbhost";

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
    xTaskCreatePinnedToCore(_taskUSBHostLib, "USBHostLib", 4096, this, DAEMON_TASK_PRIORITY, nullptr, 0);
}

void USBHost::_taskUSBHostLib(void *arg) {
    static_cast<USBHost *>(arg)->taskUSBHostLib();
}

void USBHost::taskUSBHostLib() {
    // Install USB host library
    ESP_LOGI(TAG, "Installing USB Host Library");
    usb_host_config_t hostCfg = {.intr_flags = ESP_INTR_FLAG_LEVEL1};
    ESP_ERROR_CHECK(usb_host_install(&hostCfg));

    // Create class driver task
    xTaskCreatePinnedToCore(_taskClassDriver, "class", 4096, this, CLASS_TASK_PRIORITY, nullptr, 0);
    vTaskDelay(10); // Short delay to let client task spin up

    while (1) {
        ESP_ERROR_CHECK(usb_host_lib_handle_events(portMAX_DELAY, nullptr));
    }
}

void USBHost::_taskClassDriver(void *arg) {
    static_cast<USBHost *>(arg)->taskClassDriver();
}

void USBHost::taskClassDriver() {
    ESP_LOGI(TAG, "Registering Client");
    usb_host_client_config_t clientCfg = {
        .is_synchronous    = false,
        .max_num_event_msg = CLIENT_NUM_EVENT_MSG,
        .async             = {.client_event_callback = _clientEventCb, .callback_arg = this},
    };
    ESP_ERROR_CHECK(usb_host_client_register(&clientCfg, &driver_obj.client_hdl));

    int bInterfaceNumber = -1;

    while (1) {
        if (driver_obj.actions == 0) {
            usb_host_client_handle_events(driver_obj.client_hdl, portMAX_DELAY);
        } else {
            if (driver_obj.actions & ACTION_OPEN_DEV) {
                assert(driver_obj.dev_addr != 0);
                ESP_LOGI(TAG, "Opening device at address %d", driver_obj.dev_addr);
                ESP_ERROR_CHECK(usb_host_device_open(driver_obj.client_hdl, driver_obj.dev_addr, &driver_obj.dev_hdl));
                assert(driver_obj.dev_hdl != nullptr);

                // ESP_LOGI(TAG, "Getting device information");
                ESP_ERROR_CHECK(usb_host_device_info(driver_obj.dev_hdl, &driver_obj.dev_info));
                // ESP_LOGI(TAG, "\t%s speed", (driver_obj.dev_info.speed == USB_SPEED_LOW) ? "Low" : "Full");
                // ESP_LOGI(TAG, "\tbConfigurationValue %d", driver_obj.dev_info.bConfigurationValue);

                // if (driver_obj.dev_info.str_desc_manufacturer) {
                //     ESP_LOGI(TAG, "Getting Manufacturer string descriptor");
                //     usb_print_string_descriptor(driver_obj.dev_info.str_desc_manufacturer);
                // }
                // if (driver_obj.dev_info.str_desc_product) {
                //     ESP_LOGI(TAG, "Getting Product string descriptor");
                //     usb_print_string_descriptor(driver_obj.dev_info.str_desc_product);
                // }
                // if (driver_obj.dev_info.str_desc_serial_num) {
                //     ESP_LOGI(TAG, "Getting Serial Number string descriptor");
                //     usb_print_string_descriptor(driver_obj.dev_info.str_desc_serial_num);
                // }

                // ESP_LOGI(TAG, "Getting device descriptor");
                const usb_device_desc_t *dev_desc;
                ESP_ERROR_CHECK(usb_host_get_device_descriptor(driver_obj.dev_hdl, &dev_desc));
                // usb_print_device_descriptor(dev_desc);

                // ESP_LOGI(TAG, "Getting config descriptor");
                ESP_ERROR_CHECK(usb_host_get_active_config_descriptor(driver_obj.dev_hdl, &driver_obj.cfg_desc));
                // usb_print_config_descriptor(driver_obj.cfg_desc, nullptr);

                usb_keyboard_info_t ki;
                if (find_keyboard_info((const uint8_t *)driver_obj.cfg_desc, &ki)) {
                    ESP_LOGI(TAG, "USB Keyboard found!");

                    // Claim interface
                    bInterfaceNumber = ki.bInterfaceNumber;
                    ESP_ERROR_CHECK(usb_host_interface_claim(driver_obj.client_hdl, driver_obj.dev_hdl, ki.bInterfaceNumber, ki.bAlternateSetting));
                    assert(driver_obj.dev_hdl != nullptr);

                    keyboard_set_boot_protocol();
                    USBHost::keyboardSetLeds(0);

                    ESP_LOGI(TAG, "Starting transfer");
                    usb_transfer_t *transfer;
                    ESP_ERROR_CHECK(usb_host_transfer_alloc(8, 0, &transfer));
                    transfer->device_handle    = driver_obj.dev_hdl;
                    transfer->bEndpointAddress = ki.bEndpointAddress;
                    transfer->num_bytes        = 8;
                    transfer->callback         = _notif_xfer_cb;
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
        }
    }
}

void USBHost::_clientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg) {
    static_cast<USBHost *>(arg)->clientEventCb(eventMsg);
}

void USBHost::clientEventCb(const usb_host_client_event_msg_t *eventMsg) {
    switch (eventMsg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            if (driver_obj.dev_addr == 0) {
                driver_obj.dev_addr = eventMsg->new_dev.address;
                // Open the device next
                driver_obj.actions |= ACTION_OPEN_DEV;
            }
            break;

        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            if (driver_obj.dev_hdl != nullptr) {
                // Cancel any other actions and close the device next
                driver_obj.actions = ACTION_CLOSE_DEV;
            }
            break;

        default:
            // Should never occur
            abort();
    }
}

void USBHost::_notif_xfer_cb(usb_transfer_t *transfer) {
    static_cast<USBHost *>(transfer->context)->notif_xfer_cb(transfer);
}

void USBHost::notif_xfer_cb(usb_transfer_t *transfer) {
    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        if (transfer->data_buffer_size == 8) {
            // ESP_LOG_BUFFER_HEX(TAG, transfer->data_buffer, transfer->data_buffer_size);
            process_keyboard_data(transfer->data_buffer);
        }
    }
    if (transfer->status == USB_TRANSFER_STATUS_NO_DEVICE) {
        ESP_LOGI(TAG, "notif_xfer_cb - no device");
        usb_host_transfer_free(transfer);
    }

    if (transfer->status != USB_TRANSFER_STATUS_NO_DEVICE) {
        usb_host_transfer_submit(transfer);
    }
}

void USBHost::_notif_ctrl_xfer_cb(usb_transfer_t *transfer) {
    static_cast<USBHost *>(transfer->context)->notif_ctrl_xfer_cb(transfer);
}

void USBHost::notif_ctrl_xfer_cb(usb_transfer_t *transfer) {
    ESP_LOGI(TAG, "notif_ctrl_xfer_cb - status: %u", transfer->status);
    usb_host_transfer_free(transfer);
}

void USBHost::keyboardSetLeds(uint8_t leds) {
    usb_transfer_t *transfer;

    ESP_ERROR_CHECK(usb_host_transfer_alloc(sizeof(usb_setup_packet_t) + 1, 0, &transfer));
    transfer->device_handle    = driver_obj.dev_hdl;
    transfer->bEndpointAddress = 0;
    transfer->callback         = _notif_ctrl_xfer_cb;
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
}

void USBHost::keyboard_set_boot_protocol() {
    usb_transfer_t *transfer;

    ESP_ERROR_CHECK(usb_host_transfer_alloc(sizeof(usb_setup_packet_t), 0, &transfer));
    transfer->device_handle    = driver_obj.dev_hdl;
    transfer->bEndpointAddress = 0;
    transfer->callback         = _notif_ctrl_xfer_cb;
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
}

void USBHost::process_keyboard_data(uint8_t *buf) {
    // Don't process during rollover
    if (buf[2] == 1)
        return;

    static uint8_t prev_status[8];

    auto &kb = AqKeyboard::instance();

    // Check for modifier key changes
    uint8_t mod_released = prev_status[0] & ~buf[0];
    if (mod_released & (1 << 0))
        kb.handleScancode(0xE0, false); // Left CTRL
    if (mod_released & (1 << 1))
        kb.handleScancode(0xE1, false); // Left SHIFT
    if (mod_released & (1 << 2))
        kb.handleScancode(0xE2, false); // Left ALT
    if (mod_released & (1 << 3))
        kb.handleScancode(0xE3, false); // Left GUI
    if (mod_released & (1 << 4))
        kb.handleScancode(0xE4, false); // Right CTRL
    if (mod_released & (1 << 5))
        kb.handleScancode(0xE5, false); // Right SHIFT
    if (mod_released & (1 << 6))
        kb.handleScancode(0xE6, false); // Right ALT
    if (mod_released & (1 << 7))
        kb.handleScancode(0xE7, false); // Right GUI

    uint8_t mod_pressed = ~prev_status[0] & buf[0];
    if (mod_pressed & (1 << 0))
        kb.handleScancode(0xE0, true); // Left CTRL
    if (mod_pressed & (1 << 1))
        kb.handleScancode(0xE1, true); // Left SHIFT
    if (mod_pressed & (1 << 2))
        kb.handleScancode(0xE2, true); // Left ALT
    if (mod_pressed & (1 << 3))
        kb.handleScancode(0xE3, true); // Left GUI
    if (mod_pressed & (1 << 4))
        kb.handleScancode(0xE4, true); // Right CTRL
    if (mod_pressed & (1 << 5))
        kb.handleScancode(0xE5, true); // Right SHIFT
    if (mod_pressed & (1 << 6))
        kb.handleScancode(0xE6, true); // Right ALT
    if (mod_pressed & (1 << 7))
        kb.handleScancode(0xE7, true); // Right GUI

    uint8_t prev, cur;

    // Check for key releases
    for (int j = 2; j < 8; j++) {
        if ((prev = prev_status[j]) == 0)
            break;

        // Check if this key is in current report
        bool key_released = true;
        for (int i = 2; i < 8; i++) {
            if ((cur = buf[i]) == 0)
                break;

            if (prev == cur) {
                key_released = false;
                break;
            }
        }

        if (key_released)
            kb.handleScancode(prev, false);
    }

    // Check for key presses
    for (int j = 2; j < 8; j++) {
        if ((cur = buf[j]) == 0)
            break;

        // Check if this key is in previous report
        bool key_pressed = true;
        for (int i = 2; i < 8; i++) {
            if ((prev = prev_status[i]) == 0)
                break;

            if (prev == cur) {
                key_pressed = false;
                break;
            }
        }

        if (key_pressed)
            kb.handleScancode(cur, true);
    }

    for (int i = 0; i < 8; i++) {
        prev_status[i] = buf[i];
    }

    kb.updateMatrix();
}

bool USBHost::find_keyboard_info(const uint8_t *desc, usb_keyboard_info_t *ki) {
    if (!(desc[0] == 9 && desc[1] == 0x02)) {
        return false;
    }

    const uint8_t *p            = desc;
    uint16_t       total_length = (desc[3] << 8) | desc[2];

    uint8_t bInterfaceNumber   = 0;
    uint8_t bAlternateSetting  = 0;
    uint8_t bInterfaceClass    = 0;
    uint8_t bInterfaceSubClass = 0;
    uint8_t bInterfaceProtocol = 0;
    bool    is_keyboard        = false;

    while ((p - desc) < total_length) {
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
