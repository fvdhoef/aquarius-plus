#pragma once

#include "Common.h"
#include "USBInterface.h"

class USBInterfaceMIDI : public USBInterface {
public:
    USBInterfaceMIDI(USBDevice *device);
    virtual ~USBInterfaceMIDI();

    bool init(const void *ifDesc, size_t ifDescLen);

protected:
    SemaphoreHandle_t mutex = nullptr;

    void processInData(const uint8_t *buf, size_t length) override;
};
