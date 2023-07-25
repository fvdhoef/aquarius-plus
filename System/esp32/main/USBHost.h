#pragma once

#include "Common.h"
#include <usb/usb_host.h>

class USBHost {
    USBHost();

public:
    static USBHost &instance();

    void init();
    void keyboardSetLeds(uint8_t leds);

private:
    static void _taskUSBHostLib(void *arg);
    void        taskUSBHostLib();
    static void _taskClassDriver(void *arg);
    void        taskClassDriver();

    struct usb_keyboard_info_t {
        uint8_t  bInterfaceNumber;
        uint8_t  bAlternateSetting;
        uint8_t  bEndpointAddress;
        uint16_t wMaxPacketSize;
        uint8_t  bInterval;
    };

    static void _clientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg);
    void        clientEventCb(const usb_host_client_event_msg_t *eventMsg);
    void        process_keyboard_data(uint8_t *buf);
    bool        find_keyboard_info(const uint8_t *desc, usb_keyboard_info_t *ki);
    void        keyboard_set_boot_protocol();

    static void _notif_xfer_cb(usb_transfer_t *transfer);
    void        notif_xfer_cb(usb_transfer_t *transfer);
    static void _notif_ctrl_xfer_cb(usb_transfer_t *transfer);
    void        notif_ctrl_xfer_cb(usb_transfer_t *transfer);

    struct class_driver_t {
        uint32_t                 actions;
        usb_host_client_handle_t client_hdl;
        uint8_t                  dev_addr;
        usb_device_handle_t      dev_hdl;
        usb_device_info_t        dev_info;
        const usb_config_desc_t *cfg_desc;
    };

    class_driver_t driver_obj = {0};
};
