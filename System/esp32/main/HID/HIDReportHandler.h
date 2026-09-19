#pragma once

#include "Common.h"
#include "HIDReportDescriptor.h"

class HIDReportHandler {
public:
    enum Type {
        TUndefined,
        TKeyboard,
        TMouse,
        TGamepad,
    };

    static HIDReportHandler *getReportHandlersForDescriptor(const void *reportDescBuf, size_t reportDescLen);

    HIDReportHandler(Type type);
    virtual ~HIDReportHandler();

    virtual bool init(const HIDCollection *collection);

    void addInputField(const HIDField &field);
    void addOutputField(const HIDField &field);
    void inputReport(const uint8_t *buf, size_t length);

    HIDReportHandler *next        = nullptr;
    Type              type        = TUndefined;
    bool              hasReportId = false;

protected:
    virtual void _addInputField(const HIDField &field);
    virtual void _addOutputField(const HIDField &field);
    virtual void _inputReport(uint8_t reportId, const uint8_t *buf, size_t length) = 0;

    void enumerateCollection(const HIDCollection *collection);

    int32_t readBits(const void *buf, size_t bufLen, uint32_t bitOffset, uint32_t bitLength, bool signExtend);
};
