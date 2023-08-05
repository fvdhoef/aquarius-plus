#include "USBInterfaceHID.h"
#include "USBTypes.h"
#include "USBDevice.h"
#include "HIDReportDescriptor.h"
#include "HIDReportHandlerKeyboard.h"
#include "HIDReportHandlerMouse.h"

static const char *TAG = "USBInterfaceHID";

USBInterfaceHID::USBInterfaceHID(USBDevice *device)
    : USBInterface(device) {
}

USBInterfaceHID::~USBInterfaceHID() {
    {
        RecursiveMutexLock lock(mutex);
        if (reportHandlers) {
            delete reportHandlers;
        }
        if (ifClaimed)
            device->releaseInterface(bInterfaceNumber);
    }
    vSemaphoreDelete(mutex);
}

bool USBInterfaceHID::init(const void *ifDesc, size_t ifDescLen) {
    mutex = xSemaphoreCreateRecursiveMutex();
    RecursiveMutexLock lock(mutex);

    const uint8_t *p   = (const uint8_t *)ifDesc;
    size_t         len = ifDescLen;

    // Check for interface descriptor
    if (len < 9 || len < p[0] || p[1] != USB_DT_INTERFACE || p[5] != 0x03) {
        return false;
    }
    bInterfaceNumber  = p[2];
    bAlternateSetting = p[3];

    //	unsigned numEndpoints = p[4];

    len -= p[0];
    p += p[0];

    // HID interface descriptor
    if (len < 9 || len < p[0] || p[1] != 0x21 || p[6] != 0x22) {
        return false;
    }
    unsigned reportDescLen = p[7] | (p[8] << 8);

    len -= p[0];
    p += p[0];

    // Check for endpoint descriptor
    if (len < 7 || len < p[0] || p[1] != USB_DT_ENDPOINT || (p[2] & 0x80) == 0 || (p[3] & 3) != 3) {
        return false;
    }

    unsigned endpointAddr  = p[2];
    unsigned maxPacketSize = p[4] | (p[5] << 8);
    unsigned interval      = p[6];

    ESP_LOGI(TAG, "- Interrupt endpoint %u maxPacketSize: %u interval: %u", endpointAddr, maxPacketSize, interval);

    HIDReportDescriptor *reportDescriptor = NULL;

    uint8_t *reportDescriptorBuf = new uint8_t[reportDescLen];
    if (reportDescriptorBuf == nullptr)
        return false;

    bool result = device->controlTransfer(
        USB_ENDPOINT_IN | USB_RECIPIENT_INTERFACE, USB_REQUEST_GET_DESCRIPTOR,
        (0x22 << 8) | 0, bInterfaceNumber, reportDescriptorBuf, reportDescLen);

    ESP_LOGI(TAG, "result: %d", result);

    if (result) {
        reportDescriptor = HIDReportDescriptor::parseReportDescriptor(reportDescriptorBuf, reportDescLen);
    }
    delete[] reportDescriptorBuf;
    if (!result) {
        return false;
    }

    if (reportDescriptor) {
        // reportDescriptor->dumpItems();

        HIDReportDescriptor::HIDItem *item = reportDescriptor->items;
        while (item) {
            if (item->type == HIDReportDescriptor::HIDItem::TCollection) {
                HIDReportDescriptor::HIDCollection *collection = static_cast<HIDReportDescriptor::HIDCollection *>(item);

                HIDReportHandler *reportHandler = NULL;

                uint32_t usage = ((uint32_t)collection->usagePage << 16) | collection->usage;
                switch (usage) {
                    case 0x10002:
                        ESP_LOGI(TAG, "Mouse detected");
                        reportHandler = new HIDReportHandlerMouse();
                        break;

                    case 0x10006:
                        ESP_LOGI(TAG, "Keyboard detected");
                        reportHandler = new HIDReportHandlerKeyboard();
                        break;

                    default:
                        break;
                }

                if (reportHandler) {
                    if (!reportHandler->init(collection)) {
                        ESP_LOGE(TAG, "Could not init report handler.");
                        delete reportHandler;
                        reportHandler = NULL;
                    }
                }

                if (reportHandler) {
                    reportHandler->next = reportHandlers;
                    reportHandlers      = reportHandler;
                }
            }
            item = item->next;
        }

        delete reportDescriptor;
    }

    // At least one data handler for this interface?
    if (reportHandlers) {
        // Claim interface
        if (!device->claimInterface(bInterfaceNumber, bAlternateSetting))
            return false;

        ifClaimed = true;

        size_t transferSize = (maxPacketSize + 3) & ~3;

        ESP_LOGI(TAG, "Starting transfer on EP 0x%02X size: %u", endpointAddr, transferSize);
        device->transferIn(endpointAddr, transferSize, _inTransferCb, this);
    }

    return true;
}

void USBInterfaceHID::_inTransferCb(usb_transfer_t *transfer) {
    if (transfer->status == USB_TRANSFER_STATUS_NO_DEVICE) {
        // Device is removed, free transfer
        ESP_LOGI(TAG, "inTransferCb - no device");
        usb_host_transfer_free(transfer);
        return;
    } else if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        static_cast<USBInterfaceHID *>(transfer->context)->processInterruptData(transfer->data_buffer, transfer->actual_num_bytes);
    }

    // Retransmit transfer to get next data
    usb_host_transfer_submit(transfer);
}

void USBInterfaceHID::processInterruptData(const uint8_t *buf, size_t length) {
    RecursiveMutexLock lock(mutex);

    // ESP_LOG_BUFFER_HEXDUMP(TAG, transfer->data_buffer, transfer->actual_num_bytes, ESP_LOG_INFO);

    HIDReportHandler *reportHandler = reportHandlers;
    while (reportHandler) {
        reportHandler->inputReport(buf, length);
        reportHandler = reportHandler->next;
    }
}
