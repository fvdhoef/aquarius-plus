#pragma once

#include "HIDReportHandler.h"

class HIDReportHandlerKeyboard : public HIDReportHandler {
public:
    HIDReportHandlerKeyboard();
    virtual ~HIDReportHandlerKeyboard();

    void addInputField(const HIDReportDescriptor::HIDField &field) override;
    void inputReport(const uint8_t *buf, size_t length) override;

protected:
    void compareKeyArrays(
        const uint8_t *keys1, const uint8_t *keys2,
        uint8_t *only1, uint8_t *only2, uint8_t *both,
        unsigned numElements);

    enum { LCtrl = 0,
           LShift,
           LAlt,
           LGui,
           RCtrl,
           RShift,
           RAlt,
           RGui,
           maxButtons };
    int buttons[maxButtons];

    int keyArrayIdx;
    int keyArrayItemSize;
    int keyArrayItems;

    static const unsigned maxKeyData = 16;
    uint8_t               prevKeyData[maxKeyData];
    uint8_t               prevModifiers;
};
