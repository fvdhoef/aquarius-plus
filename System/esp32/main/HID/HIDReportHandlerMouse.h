#pragma once

#include "HIDReportHandler.h"

class HIDReportHandlerMouse : public HIDReportHandler {
public:
    HIDReportHandlerMouse();

    void addInputField(const HIDReportDescriptor::HIDField &field) override;
    void inputReport(const uint8_t *buf, size_t length) override;

protected:
    static const int maxButtons = 3;

    int buttons[maxButtons];
    int xIdx, yIdx, wheelIdx;
    int xSize, ySize, wheelSize;
};
