#pragma once

#include "Common.h"
#include <usb/usb_host.h>
#include <USBDevice.h>

class USBHost {
    USBHost();

public:
    static USBHost &instance();

    void init();
    void keyboardSetLeds(uint8_t leds);

private:
    static void _taskUSBEvents(void *arg);
    void        taskUSBEvents();
    static void _taskClassDriver(void *arg);
    void        taskClassDriver();
    static void _taskClient(void *arg);
    void        taskClient();

    struct usb_keyboard_info_t {
        uint8_t  bInterfaceNumber;
        uint8_t  bAlternateSetting;
        uint8_t  bEndpointAddress;
        uint16_t wMaxPacketSize;
        uint8_t  bInterval;
    };

    static void _clientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg);
    void        clientEventCb(const usb_host_client_event_msg_t *eventMsg);
    bool        findKeyboardInfo(const uint8_t *desc, usb_keyboard_info_t *ki);
    void        keyboardSetBootProtocol();

    static void _transferCb(usb_transfer_t *transfer);
    void        transferCb(usb_transfer_t *transfer);
    static void _ctrlTransferCb(usb_transfer_t *transfer);
    void        ctrlTransferCb(usb_transfer_t *transfer);

    // struct class_driver_t {
    //     uint32_t                 actions;
    //     usb_host_client_handle_t client_hdl;
    //     uint8_t                  dev_addr;
    //     usb_device_handle_t      dev_hdl;
    //     usb_device_info_t        dev_info;
    //     const usb_config_desc_t *cfg_desc;
    // };

    // class_driver_t driver_obj = {0};

    usb_host_client_handle_t clientHdl;
    TaskHandle_t             clientTask;
};
