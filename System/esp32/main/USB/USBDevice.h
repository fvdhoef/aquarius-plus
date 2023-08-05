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

private:
    bool _init(usb_host_client_handle_t _clientHdl, uint8_t devAddr);
    bool parseDescriptors(const void *buf, size_t length);

    usb_host_client_handle_t clientHdl = nullptr;
    usb_device_handle_t      devHdl    = nullptr;
    usb_device_info_t        devInfo;
    const usb_device_desc_t *devDesc    = nullptr;
    const usb_config_desc_t *cfgDesc    = nullptr;
    USBInterface            *interfaces = nullptr;

    // static const char *GetSpeedString(USBDeviceSpeed speed) {
    //     switch (speed) {
    //         default:
    //         case SpeedLow: return "Low"; break;
    //         case SpeedFull: return "Full"; break;
    //         case SpeedHigh: return "High"; break;
    //     }
    // }

    // USBDevice(EHCIQueueHead *qhCtrl, USBHub *parentHub, unsigned hubPortNr, unsigned usbAddr, USBDeviceSpeed speed);
    // virtual ~USBDevice();
    // virtual bool Init();

    // virtual void Dump(int level);

    // bool ReadDeviceDescriptor(USBDeviceDesc &deviceDesc);
    // bool ReadConfigurationDescriptor(USBConfigDesc &configDesc);
    // bool ReadDescriptors(void *buf, size_t totalLength);

    // bool SetConfiguration(unsigned configValue);

    // bool BulkTransfer(uint8_t endpoint, void *data, size_t length, size_t *transferred);
    // bool InterruptTransfer(uint8_t endpoint, void *data, size_t length, size_t *transferred);

    // bool ParseDescriptors(const void *buf, size_t length);
    // bool PrintDescriptors(const void *buf, size_t length);

    // USBHub *GetHub() {
    //     return parentHub;
    // }
    // void GetHSHub(USBHub *&hub, unsigned &portNr);

    // EHCIQueueHead *qhCtrl;
    // USBHub        *parentHub;
    // unsigned         hubPortNr;
    // unsigned         usbAddr;
    // USBDeviceSpeed deviceSpeed;
    // USBDevice     *nextDevice;
};
