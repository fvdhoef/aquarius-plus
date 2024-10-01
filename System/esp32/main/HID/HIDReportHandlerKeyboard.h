#pragma once

#include "HIDReportHandler.h"

enum {
    NUM_LOCK    = (1 << 0),
    CAPS_LOCK   = (1 << 1),
    SCROLL_LOCK = (1 << 2),
};

class HIDReportHandlerKeyboard : public HIDReportHandler {
public:
    HIDReportHandlerKeyboard();
    virtual ~HIDReportHandlerKeyboard();

    uint8_t outputReport(uint8_t leds) const;

protected:
    void _addInputField(const HIDReportDescriptor::HIDField &field) override;
    void _addOutputField(const HIDReportDescriptor::HIDField &field) override;
    void _inputReport(uint8_t reportId, const uint8_t *buf, size_t length) override;

    void compareKeyArrays(
        const uint8_t *keys1, const uint8_t *keys2,
        uint8_t *only1, uint8_t *only2, uint8_t *both,
        unsigned numElements);

    enum {
        LCtrl = 0,
        LShift,
        LAlt,
        LGui,
        RCtrl,
        RShift,
        RAlt,
        RGui,
        maxButtons,
    };
    int buttons[maxButtons];

    int keyArrayIdx      = -1;
    int keyArrayItemSize = -1;
    int keyArrayItems    = -1;

    static const unsigned maxKeyData = 16;
    uint8_t               prevKeyData[maxKeyData];
    uint8_t               prevModifiers = 0;

    int ledNumLockIdx    = -1;
    int ledCapsLockIdx   = -1;
    int ledScrollLockIdx = -1;
};
