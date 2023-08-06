#include "USBDevice.h"
#include "USBInterfaceHID.h"
#include "HIDReportHandlerKeyboard.h"

static const char *TAG = "USBDevice";

USBDevice::USBDevice() {
}

USBDevice::~USBDevice() {
    ESP_LOGI(TAG, "Removing device");

    while (interfaces) {
        USBInterface *interface = interfaces;
        interfaces              = interfaces->nextInterface;
        delete interface;
    }
    usb_host_device_close(clientHdl, devHdl);
}

USBDevicePtr USBDevice::init(usb_host_client_handle_t clientHdl, uint8_t devAddr) {
    auto result = USBDevicePtr(new USBDevice());
    if (!result->_init(clientHdl, devAddr))
        return nullptr;
    return result;
}

bool USBDevice::_init(usb_host_client_handle_t _clientHdl, uint8_t devAddr) {
    this->clientHdl = _clientHdl;

    ESP_LOGI(TAG, "Opening device at address %d", devAddr);

    esp_err_t err;
    if ((err = usb_host_device_open(clientHdl, devAddr, &devHdl)) != ESP_OK ||
        (err = usb_host_device_info(devHdl, &devInfo)) != ESP_OK ||
        (err = usb_host_get_device_descriptor(devHdl, &devDesc)) != ESP_OK ||
        (err = usb_host_get_active_config_descriptor(devHdl, &cfgDesc)) != ESP_OK) {

        ESP_LOGE(TAG, "Failed to open device %d", devAddr);
        return false;
    }

    // ESP_LOG_BUFFER_HEXDUMP(TAG, cfgDesc, cfgDesc->wTotalLength, ESP_LOG_INFO);

    if (!parseDescriptors(cfgDesc, cfgDesc->wTotalLength)) {
        return false;
    }

    return true;
}

bool USBDevice::parseDescriptors(const void *buf, size_t length) {
    size_t len = length;

    const uint8_t *p = static_cast<const uint8_t *>(buf);

    if (len < 2) {
        return false;
    }
    if (p[1] != USB_DT_CONFIGURATION) {
        ESP_LOGE(TAG, "No configuration descriptor found");
        return false;
    }

    if (len < p[0]) {
        return false;
    }
    len -= p[0];
    p += p[0];

    while (len > 0) {
        const uint8_t *ifBuf = p;
        size_t         ifLen = 0;

        if (len < 9) {
            return false;
        }

        if (p[1] != USB_DT_INTERFACE) {
            ESP_LOGE(TAG, "No interface descriptor found");
            return false;
        }

        uint8_t ifClass = p[5];

        while (len > 0) {
            if (len < p[0]) {
                return false;
            }

            ifLen += p[0];
            len -= p[0];
            p += p[0];

            if (len > 2 && p[1] == USB_DT_INTERFACE) {
                break;
            }

            if (len > 2) {
                //				printf("%02X\n", p[1]);
            }
        }

        ESP_LOGI(TAG, "Interface found - class %02X: %u %u", ifClass, ifBuf - static_cast<const uint8_t *>(buf), ifLen);

        switch (ifClass) {
            case 0x03: {
                ESP_LOGI(TAG, "HID interface!");

                USBInterfaceHID *hidInterface = new USBInterfaceHID(this);
                if (hidInterface->init(ifBuf, ifLen)) {
                    hidInterface->nextInterface = interfaces;
                    interfaces                  = hidInterface;
                } else {
                    delete hidInterface;
                }
                break;
            }

            default: {
                ESP_LOGW(TAG, "Unsupported interface class type!");
                break;
            }
        }
    }
    return true;
}

bool USBDevice::claimInterface(uint8_t bInterfaceNumber, uint8_t bAlternateSetting) {
    esp_err_t err = usb_host_interface_claim(clientHdl, devHdl, bInterfaceNumber, bAlternateSetting);

    ESP_LOGI(TAG, "claimInterface(%u, %u) -> %u", bInterfaceNumber, bAlternateSetting, err);

    return err == ESP_OK;
}

bool USBDevice::releaseInterface(uint8_t bInterfaceNumber) {
    esp_err_t err = usb_host_interface_release(clientHdl, devHdl, bInterfaceNumber);

    ESP_LOGI(TAG, "releaseInterface(%u) -> %u", bInterfaceNumber, err);

    return err == ESP_OK;
}

static void controlTransferCb(usb_transfer_t *transfer) {
    if (transfer->context) {
        xTaskNotify((TaskHandle_t)transfer->context, 1, eSetBits);
    } else {
        usb_host_transfer_free(transfer);
    }
}

bool USBDevice::controlTransfer(
    uint8_t bmRequestType, uint8_t bRequest,
    uint16_t wValue, uint16_t wIndex,
    void *buf, uint16_t wLength, bool waitResult) {

    // ESP_LOGI(TAG, "controlTransfer(0x%02X, 0x%02X, 0x%04X, 0x%04X, %p, %u)", bmRequestType, bRequest, wValue, wIndex, buf, wLength);
    // if ((bmRequestType & 0x80) == 0) {
    //     ESP_LOG_BUFFER_HEXDUMP(TAG, buf, wLength, ESP_LOG_INFO);
    // }

    usb_transfer_t *transfer;
    if (usb_host_transfer_alloc(sizeof(usb_setup_packet_t) + wLength, 0, &transfer) != ESP_OK) {
        ESP_LOGE(TAG, "usb_host_transfer_alloc: Out of memory");
        return false;
    }
    transfer->device_handle    = devHdl;
    transfer->bEndpointAddress = 0;
    transfer->callback         = controlTransferCb;
    transfer->context          = waitResult ? xTaskGetCurrentTaskHandle() : nullptr;
    transfer->timeout_ms       = 1000;
    transfer->num_bytes        = sizeof(usb_setup_packet_t) + wLength;

    usb_setup_packet_t *req = (usb_setup_packet_t *)(transfer->data_buffer);
    req->bmRequestType      = bmRequestType;
    req->bRequest           = bRequest;
    req->wValue             = wValue;
    req->wIndex             = wIndex;
    req->wLength            = wLength;

    if ((bmRequestType & 0x80) == 0) {
        // Out transfer, copy data to buffer
        uint8_t *p = (uint8_t *)req + sizeof(usb_setup_packet_t);
        memcpy(p, buf, wLength);
    }

    esp_err_t err = usb_host_transfer_submit_control(clientHdl, transfer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "usb_host_transfer_submit_control: %s", esp_err_to_name(err));
        usb_host_transfer_free(transfer);
        return false;
    }

    if (!waitResult)
        return true;

    uint32_t notifiedValue;
    xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, portMAX_DELAY);

    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        if ((bmRequestType & 0x80) != 0) {
            // In transfer, copy data from buffer
            uint8_t *p = (uint8_t *)req + sizeof(usb_setup_packet_t);
            memcpy(buf, p, wLength);

            // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, wLength, ESP_LOG_INFO);
        }

    } else {
        ESP_LOGE(TAG, "controlTransfer failed: %d", transfer->status);
    }
    usb_host_transfer_free(transfer);

    return (transfer->status == USB_TRANSFER_STATUS_COMPLETED);
}

bool USBDevice::transferIn(uint8_t epAddr, size_t length, usb_transfer_cb_t transferCb, void *cbContext) {
    usb_transfer_t *transfer;
    if (usb_host_transfer_alloc(length, 0, &transfer) != ESP_OK) {
        ESP_LOGE(TAG, "transferIn: Out of memory");
        return false;
    }
    transfer->device_handle    = devHdl;
    transfer->bEndpointAddress = epAddr;
    transfer->callback         = transferCb;
    transfer->context          = cbContext;
    transfer->timeout_ms       = 1000;
    transfer->num_bytes        = length;

    esp_err_t err = usb_host_transfer_submit(transfer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "usb_host_transfer_submit: %s", esp_err_to_name(err));
        usb_host_transfer_free(transfer);
        return false;
    }
    return true;
}

void USBDevice::setLeds(uint8_t leds) {
    bool    isKeyboard = false;
    uint8_t reportData = 0;

    auto interface = interfaces;
    while (interface) {
        auto hidIf = dynamic_cast<USBInterfaceHID *>(interface);
        if (hidIf) {
            auto reportHandler = hidIf->getReportHandlers();
            while (reportHandler) {
                auto kbRH = dynamic_cast<const HIDReportHandlerKeyboard *>(reportHandler);
                if (kbRH) {
                    isKeyboard = true;
                    reportData = kbRH->outputReport(leds);
                    break;
                }
                reportHandler = reportHandler->next;
            }
        }
        interface = interface->nextInterface;
    }

    if (isKeyboard) {
        controlTransfer(
            USB_BM_REQUEST_TYPE_DIR_OUT | USB_BM_REQUEST_TYPE_TYPE_CLASS | USB_BM_REQUEST_TYPE_RECIP_INTERFACE,
            USB_HID_REQUEST_SET_REPORT,
            (2 << 8) | 0,
            0, &reportData, 1, false);
    }
}
