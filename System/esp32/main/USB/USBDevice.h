#pragma once

#include "Common.h"
#include "USBTypes.h"
#include <usb/usb_host.h>

class USBDevice;
class USBInterface;

using USBDevicePtr = std::shared_ptr<USBDevice>;

class USBDevice {
    USBDevice();

public:
    virtual ~USBDevice();

    static USBDevicePtr init(usb_host_client_handle_t clientHdl, uint8_t devAddr);

    bool claimInterface(uint8_t bInterfaceNumber, uint8_t bAlternateSetting);
    bool releaseInterface(uint8_t bInterfaceNumber);

    bool controlTransfer(
        uint8_t bmRequestType, uint8_t bRequest,
        uint16_t wValue, uint16_t wIndex,
        void *buf, uint16_t wLength);

    bool transferIn(uint8_t epAddr, size_t length, usb_transfer_cb_t transferCb, void *cbContext);

    usb_device_handle_t getHandle() {
        return devHdl;
    }

private:
    bool _init(usb_host_client_handle_t _clientHdl, uint8_t devAddr);
    bool parseDescriptors(const void *buf, size_t length);

    usb_host_client_handle_t clientHdl = nullptr;
    usb_device_handle_t      devHdl    = nullptr;
    usb_device_info_t        devInfo;
    const usb_device_desc_t *devDesc    = nullptr;
    const usb_config_desc_t *cfgDesc    = nullptr;
    USBInterface            *interfaces = nullptr;
};
