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
    static void _clientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg);
    void        clientEventCb(const usb_host_client_event_msg_t *eventMsg);

    usb_host_client_handle_t                    clientHdl;
    TaskHandle_t                                clientTask;
    SemaphoreHandle_t                           devicesMutex = nullptr;
    std::map<usb_device_handle_t, USBDevicePtr> devices;
};
