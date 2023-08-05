#include "HIDReportHandler.h"
#include "USBInterfaceHID.h"

HIDReportHandler::HIDReportHandler(Type _type)
    : type(_type) {
}

HIDReportHandler::~HIDReportHandler() {
    if (next) {
        delete next;
    }
}

bool HIDReportHandler::init(const HIDReportDescriptor::HIDCollection *collection) {
    enumerateCollection(collection);
    return true;
}

void HIDReportHandler::enumerateCollection(const HIDReportDescriptor::HIDCollection *collection) {
    const HIDReportDescriptor::HIDItem *item = collection->subItems;
    while (item) {
        if (item->type == HIDReportDescriptor::HIDItem::TCollection) {
            enumerateCollection(static_cast<const HIDReportDescriptor::HIDCollection *>(item));

        } else {
            const HIDReportDescriptor::HIDField *field = static_cast<const HIDReportDescriptor::HIDField *>(item);

            switch (field->type) {
                case HIDReportDescriptor::HIDItem::TInputField: {
                    addInputField(*field);
                    break;
                }
                case HIDReportDescriptor::HIDItem::TOutputField: break;
                case HIDReportDescriptor::HIDItem::TFeatureField: break;
                default: break;
            }
        }
        item = item->next;
    }
}

void HIDReportHandler::addInputField(const HIDReportDescriptor::HIDField &field) {
    printf("Unhandled field:");
    printf(
        " (reportId: %d) bit %d:%d - usage: %X:%X..%X (valid range: %d..%d) attributes: %X\n",
        field.reportID,
        field.bitIdx, field.bitSize,
        field.usagePage, field.usageMin, field.usageMax,
        (int)field.minVal, (int)field.maxVal,
        (unsigned)field.attributes);
}

int32_t HIDReportHandler::readBits(const void *buf, size_t bufLen, uint32_t bitOffset, uint32_t bitLength, bool signExtend) {
    if (bitLength > 32) {
        return 0;
    }

    uint32_t result = 0;

    const uint8_t *p         = (const uint8_t *)buf;
    int            bitlen    = bitLength;
    uint32_t       inOffset  = bitOffset;
    uint32_t       outOffset = 0;

    while (bitlen > 0) {
        uint32_t byteIdx = (inOffset / 8);
        if (byteIdx >= bufLen) {
            return 0;
        }

        uint32_t bit = inOffset & 7;
        uint32_t val = p[byteIdx] >> bit;

        int bitsDone = 8 - bit;
        if (bitsDone > bitlen) {
            bitsDone = bitlen;
        }

        // Mask value if necessary
        if (bitlen == bitsDone) {
            val &= (1 << bitlen) - 1;
        }

        bitlen -= bitsDone;

        // Put value in result
        result |= val << outOffset;

        inOffset += bitsDone;
        outOffset += bitsDone;
    }

    if (signExtend) {
        if (result & (1 << (bitLength - 1))) {
            result |= ~((1 << bitLength) - 1);
        }
    }
    return result;
};
