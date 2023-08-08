#pragma once

#include "Common.h"
#include <usb/usb_host.h>

class USBDevice;

class USBInterface {
public:
    USBInterface(USBDevice *_device);
    virtual ~USBInterface();

    USBInterface *nextInterface = nullptr;

protected:
    static void  _interruptInTransferCb(usb_transfer_t *transfer);
    virtual void processInterruptData(const uint8_t *buf, size_t length) = 0;

    USBDevice *device            = nullptr;
    uint8_t    bInterfaceNumber  = 0;
    uint8_t    bAlternateSetting = 0;
    bool       ifClaimed         = false;
};
