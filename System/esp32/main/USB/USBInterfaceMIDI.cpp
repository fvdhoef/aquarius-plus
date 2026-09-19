#include "USBInterfaceMIDI.h"
#include "USBTypes.h"
#include "USBDevice.h"
#include "MidiData.h"

static const char *TAG = "USBInterfaceMIDI";

USBInterfaceMIDI::USBInterfaceMIDI(USBDevice *device)
    : USBInterface(device) {
}

USBInterfaceMIDI::~USBInterfaceMIDI() {
    {
        RecursiveMutexLock lock(mutex);
        if (ifClaimed)
            device->releaseInterface(bInterfaceNumber);
    }
    vSemaphoreDelete(mutex);
}

bool USBInterfaceMIDI::init(const void *ifDesc, size_t ifDescLen) {
    mutex = xSemaphoreCreateRecursiveMutex();
    RecursiveMutexLock lock(mutex);

    const uint8_t *p   = (const uint8_t *)ifDesc;
    size_t         len = ifDescLen;

    ESP_LOG_BUFFER_HEXDUMP(TAG, p, len, ESP_LOG_INFO);

    // Check for interface descriptor
    if (len < 9 || len < p[0] || p[1] != USB_DT_INTERFACE || p[5] != 0x01 || p[6] != 0x03)
        return false;
    bInterfaceNumber  = p[2];
    bAlternateSetting = p[3];

    // unsigned numEndpoints = p[4];

    // Find endpoint descriptor
    int endpointAddr  = -1;
    int maxPacketSize = -1;

    int remaining = len;
    while (remaining > 0) {
        unsigned       descLen = p[0];
        const uint8_t *p_next  = p + descLen;

        if (p[0] == 9 && p[1] == USB_DT_ENDPOINT && (p[2] & 0x80) != 0 && (p[3] & 3) == 2) {
            endpointAddr  = p[2];
            maxPacketSize = p[4] | (p[5] << 8);
            ESP_LOGI(TAG, "- Bulk endpoint %u maxPacketSize: %u", endpointAddr, maxPacketSize);
            break;
        }

        p = p_next;
    }
    if (endpointAddr < 0)
        return false;

    if (!device->claimInterface(bInterfaceNumber, bAlternateSetting))
        return false;

    ESP_LOGI(TAG, "Starting transfer on EP 0x%02X size: %u", endpointAddr, maxPacketSize);
    device->transferIn(endpointAddr, maxPacketSize, _inTransferCb, this);

    return true;
}

void USBInterfaceMIDI::processInData(const uint8_t *buf, size_t length) {
    RecursiveMutexLock lock(mutex);

    // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, length, ESP_LOG_INFO);

    auto midiData = MidiData::instance();

    int            numEventPackets = length / 4;
    const uint8_t *p               = buf;
    for (int i = 0; i < numEventPackets; i++, p += 4) {
        uint8_t cin = (p[0] & 0xF);

        // Filter out uninteresting messages
        if (cin < 8)
            continue;
        // Filter out MIDI clock and Active sense
        if (p[1] == 0xF8 || p[1] == 0xFE)
            continue;

        ESP_LOGI(TAG, "%02x %02x %02x %02x", p[0], p[1], p[2], p[3]);
        midiData->addData(p);
    }
}
