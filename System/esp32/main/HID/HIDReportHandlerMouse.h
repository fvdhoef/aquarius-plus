#pragma once

#include "HIDReportHandler.h"

class HIDReportHandlerMouse : public HIDReportHandler {
public:
    HIDReportHandlerMouse();

protected:
    void _addInputField(const HIDReportDescriptor::HIDField &field) override;
    void _inputReport(uint8_t reportId, const uint8_t *buf, size_t length) override;

    static const int maxButtons = 3;

    int buttons[maxButtons];
    int xIdx, yIdx, wheelIdx;
    int xSize, ySize, wheelSize;
};
