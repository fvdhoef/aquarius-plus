#pragma once

#include "HIDReportHandler.h"

class HIDReportHandlerGamepad : public HIDReportHandler {
public:
    HIDReportHandlerGamepad();
    virtual ~HIDReportHandlerGamepad();

private:
    void _addInputField(const HIDReportDescriptor::HIDField &field) override;
    void _inputReport(uint8_t reportId, const uint8_t *buf, size_t length) override;

    int idxLSX = -1, sizeLSX = -1, minLSX, maxLSX;
    int idxLSY = -1, sizeLSY = -1, minLSY, maxLSY;
    int idxRSX = -1, sizeRSX = -1, minRSX, maxRSX;
    int idxRSY = -1, sizeRSY = -1, minRSY, maxRSY;
    int idxLT = -1, sizeLT = -1, minLT, maxLT;
    int idxRT = -1, sizeRT = -1, minRT, maxRT;
    int idxHat = -1, sizeHat = -1, minHat, maxHat;
    int idxBtns[16];

    int8_t  getInt8(int idx, int size, int min, int max, const uint8_t *buf, size_t length);
    uint8_t getUInt8(int idx, int size, int min, int max, const uint8_t *buf, size_t length);

    int reportId = 0;
};
