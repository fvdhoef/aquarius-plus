#include "USBInterfaceHID.h"
#include "USBTypes.h"
#include "USBDevice.h"
#include "HIDReportDescriptor.h"
#include "HIDReportHandlerKeyboard.h"
#include "HIDReportHandlerMouse.h"
#include "HIDReportHandlerGamepad.h"

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

    uint8_t *reportDescBuf = new uint8_t[reportDescLen];
    if (reportDescBuf == nullptr)
        return false;

    bool result = device->controlTransfer(
        USB_ENDPOINT_IN | USB_RECIPIENT_INTERFACE, USB_REQUEST_GET_DESCRIPTOR,
        (0x22 << 8) | 0, bInterfaceNumber, reportDescBuf, reportDescLen);

    ESP_LOGI(TAG, "result: %d", result);

    reportHandlers = HIDReportHandler::getReportHandlersForDescriptor(reportDescBuf, reportDescLen);
    delete[] reportDescBuf;

    // At least one data handler for this interface?
    if (reportHandlers) {
        // Claim interface
        if (!device->claimInterface(bInterfaceNumber, bAlternateSetting))
            return false;

        ifClaimed = true;

        size_t transferSize = (maxPacketSize + 3) & ~3;

        ESP_LOGI(TAG, "Starting transfer on EP 0x%02X size: %u", endpointAddr, transferSize);
        device->transferIn(endpointAddr, transferSize, _inTransferCb, this);

        // if (_isKeyboard) {
        //     device->setLeds(0);
        // }
    }

    return true;
}

void USBInterfaceHID::processInData(const uint8_t *buf, size_t length) {
    RecursiveMutexLock lock(mutex);

    // ESP_LOG_BUFFER_HEXDUMP(TAG, transfer->data_buffer, transfer->actual_num_bytes, ESP_LOG_INFO);

    HIDReportHandler *reportHandler = reportHandlers;
    while (reportHandler) {
        reportHandler->inputReport(buf, length);
        reportHandler = reportHandler->next;
    }
}
