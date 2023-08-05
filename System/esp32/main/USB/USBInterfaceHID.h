#pragma once

#include "Common.h"
#include "USBInterface.h"
#include "HIDReportDescriptor.h"
#include "HIDReportHandler.h"

class USBInterfaceHID : public USBInterface {
public:
    USBInterfaceHID(USBDevice *device);
    virtual ~USBInterfaceHID();

    bool init(const void *ifDesc, size_t ifDescLen);

    enum CollectionType {
        CTPhysical      = 0x00, // CP
        CTApplication   = 0x01, // CA
        CTLogical       = 0x02, // CL
        CTReport        = 0x03,
        CTNamedArray    = 0x04, // NAry
        CTUsageSwitch   = 0x05, // US
        CTUsageModifier = 0x06  // UM
    };

    bool isKeyboard() {
        return _isKeyboard;
    }

protected:
    SemaphoreHandle_t mutex          = nullptr;
    bool              _isKeyboard    = false;
    HIDReportHandler *reportHandlers = nullptr;

    void processInterruptData(const uint8_t *buf, size_t length) override;
};
