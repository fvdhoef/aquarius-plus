#pragma once

#include "HIDReportHandler.h"

class HIDReportHandlerGamepad : public HIDReportHandler {
public:
    HIDReportHandlerGamepad();
    virtual ~HIDReportHandlerGamepad();

    void addInputField(const HIDReportDescriptor::HIDField &field) override;
    void inputReport(const uint8_t *buf, size_t length) override;

private:
    int leftX_Idx = -1, leftX_Size = -1;
    int leftY_Idx = -1, leftY_Size = -1;
    int hat_Idx = -1, hat_Size = -1;
    int btnA_Idx  = -1;
    int btnB_Idx  = -1;
    int btnX_Idx  = -1;
    int btnY_Idx  = -1;
    int btnLB_Idx = -1;
    int btnRB_Idx = -1;

    uint8_t oldHandctrl = 0xFF;
};
