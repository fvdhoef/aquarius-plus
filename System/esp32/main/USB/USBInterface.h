#pragma once

#include "Common.h"

class USBDevice;

class USBInterface {
public:
    USBInterface(USBDevice *_device);
    virtual ~USBInterface();

    USBInterface *nextInterface = nullptr;

protected:
    // virtual void interruptData(EHCIQueueElementTransferDescriptor *qtd) = 0;
    // static void  interruptCallbackHelper(void *arg, EHCIQueueElementTransferDescriptor *qtd);

    // EHCIQueueHead *CreateInterruptEndpoint(unsigned epNum, unsigned maxPacketSize, unsigned interval);

    USBDevice *device            = nullptr;
    uint8_t    bInterfaceNumber  = 0;
    uint8_t    bAlternateSetting = 0;
};
