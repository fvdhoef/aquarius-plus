#pragma once

#include "Common.h"

enum USBDeviceSpeed {
    SpeedLow,
    SpeedFull,
    SpeedHigh,
};
enum USBEndpointType {
    EPTAsync,
    EPTInterrupt,
    EPTIsochronous,
};

// Request Type (bmRequestType)
#define USB_ENDPOINT_OUT          (0 << 7)
#define USB_ENDPOINT_IN           (1 << 7)
#define USB_REQUEST_TYPE_STANDARD (0 << 5)
#define USB_REQUEST_TYPE_CLASS    (1 << 5)
#define USB_REQUEST_TYPE_VENDOR   (2 << 5)
#define USB_RECIPIENT_DEVICE      (0 << 0)
#define USB_RECIPIENT_INTERFACE   (1 << 0)
#define USB_RECIPIENT_ENDPOINT    (2 << 0)
#define USB_RECIPIENT_OTHER       (3 << 0)

// Standard Request Codes (bRequest)
#define USB_REQUEST_GET_STATUS        0
#define USB_REQUEST_CLEAR_FEATURE     1
#define USB_REQUEST_SET_FEATURE       3
#define USB_REQUEST_SET_ADDRESS       5
#define USB_REQUEST_GET_DESCRIPTOR    6
#define USB_REQUEST_SET_DESCRIPTOR    7
#define USB_REQUEST_GET_CONFIGURATION 8
#define USB_REQUEST_SET_CONFIGURATION 9
#define USB_REQUEST_GET_INTERFACE     10
#define USB_REQUEST_SET_INTERFACE     11
#define USB_REQUEST_SYNCH_FRAME       12

// Descriptor Types
#define USB_DT_DEVICE        1
#define USB_DT_CONFIGURATION 2
#define USB_DT_STRING        3
#define USB_DT_INTERFACE     4
#define USB_DT_ENDPOINT      5

// Hub Class Feature Selectors
#define USB_HUB_C_HUB_LOCAL_POWER  0
#define USB_HUB_C_HUB_OVER_CURRENT 1

#define USB_HUB_PORT_CONNECTION     0
#define USB_HUB_PORT_ENABLE         1
#define USB_HUB_PORT_SUSPEND        2
#define USB_HUB_PORT_OVER_CURRENT   3
#define USB_HUB_PORT_RESET          4
#define USB_HUB_PORT_POWER          8
#define USB_HUB_PORT_LOW_SPEED      9
#define USB_HUB_C_PORT_CONNECTION   16
#define USB_HUB_C_PORT_ENABLE       17
#define USB_HUB_C_PORT_SUSPEND      18
#define USB_HUB_C_PORT_OVER_CURRENT 19
#define USB_HUB_C_PORT_RESET        20
#define USB_HUB_PORT_TEST           21
#define USB_HUB_PORT_INDICATOR      22

#define USB_HID_REQUEST_GET_REPORT   0x01
#define USB_HID_REQUEST_GET_IDLE     0x02
#define USB_HID_REQUEST_GET_PROTOCOL 0x03
#define USB_HID_REQUEST_SET_REPORT   0x09
#define USB_HID_REQUEST_SET_IDLE     0x0A
#define USB_HID_REQUEST_SET_PROTOCOL 0x0B

struct USBConfigDesc {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  maxPower;

    // bool readFromBuf(BufReader &br) {
    //     bLength             = br.get8();
    //     bDescriptorType     = br.get8();
    //     wTotalLength        = br.get16();
    //     bNumInterfaces      = br.get8();
    //     bConfigurationValue = br.get8();
    //     iConfiguration      = br.get8();
    //     bmAttributes        = br.get8();
    //     maxPower            = br.get8();
    //     return true;
    // }
};

struct USBDeviceDesc {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;

    // bool readFromBuf(BufReader &br) {
    //     bLength            = br.get8();
    //     bDescriptorType    = br.get8();
    //     bcdUSB             = br.get16();
    //     bDeviceClass       = br.get8();
    //     bDeviceSubClass    = br.get8();
    //     bDeviceProtocol    = br.get8();
    //     bMaxPacketSize0    = br.get8();
    //     idVendor           = br.get16();
    //     idProduct          = br.get16();
    //     bcdDevice          = br.get16();
    //     iManufacturer      = br.get8();
    //     iProduct           = br.get8();
    //     iSerialNumber      = br.get8();
    //     bNumConfigurations = br.get8();
    //     return true;
    // }
};

struct USBEndpointDesc {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;

    // bool readFromBuf(BufReader &br) {
    //     bLength          = br.get8();
    //     bDescriptorType  = br.get8();
    //     bEndpointAddress = br.get8();
    //     bmAttributes     = br.get8();
    //     wMaxPacketSize   = br.get16();
    //     bInterval        = br.get8();
    //     return true;
    // }
};

struct USBInterfaceDesc {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;

    // bool readFromBuf(BufReader &br) {
    //     bLength            = br.get8();
    //     bDescriptorType    = br.get8();
    //     bInterfaceNumber   = br.get8();
    //     bAlternateSetting  = br.get8();
    //     bNumEndpoints      = br.get8();
    //     bInterfaceClass    = br.get8();
    //     bInterfaceSubClass = br.get8();
    //     bInterfaceProtocol = br.get8();
    //     iInterface         = br.get8();
    //     return true;
    // }
};

struct USBControlSetup {
    USBControlSetup() {
    }
    USBControlSetup(
        uint8_t  bmRequestType,
        uint8_t  bRequest,
        uint16_t wValue,
        uint16_t wIndex,
        uint16_t wLength) {
        this->bmRequestType = bmRequestType;
        this->bRequest      = bRequest;
        this->wValue        = wValue;
        this->wIndex        = wIndex;
        this->wLength       = wLength;
    }

    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
};

struct USBHubDesc {
    uint8_t bDescLength;
    uint8_t bDescriptorType;
    uint8_t bNbrPorts;
    uint8_t wHubCharacteristicsL;
    uint8_t wHubCharacteristicsH;
    uint8_t bPwrOn2PwrGood;
    uint8_t bHubContrCurrent;
    uint8_t deviceRemovable;
};
