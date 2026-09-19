#include "HIDReportHandler.h"
#include "HIDReportHandlerKeyboard.h"
#include "HIDReportHandlerMouse.h"
#include "HIDReportHandlerGamepad.h"

static const char *TAG = "HIDReportHandler";

HIDReportHandler::HIDReportHandler(Type _type)
    : type(_type) {
}

HIDReportHandler::~HIDReportHandler() {
    if (next) {
        delete next;
    }
}

bool HIDReportHandler::init(const HIDCollection *collection) {
    enumerateCollection(collection);
    return true;
}

void HIDReportHandler::enumerateCollection(const HIDCollection *collection) {
    for (auto subItem : collection->subItems) {
        if (subItem->itemType == HIDItem::ItemType::Collection) {
            enumerateCollection(static_cast<const HIDCollection *>(subItem.get()));

        } else {
            const HIDField *field = static_cast<const HIDField *>(subItem.get());

            switch (field->itemType) {
                case HIDItem::ItemType::InputField: addInputField(*field); break;
                case HIDItem::ItemType::OutputField: addOutputField(*field); break;
                case HIDItem::ItemType::FeatureField: break;
                default: break;
            }
        }
    }
}

void HIDReportHandler::addInputField(const HIDField &field) {
    if (field.reportID > 0)
        hasReportId = true;

    _addInputField(field);
}

void HIDReportHandler::addOutputField(const HIDField &field) {
    _addOutputField(field);
}

void HIDReportHandler::inputReport(const uint8_t *buf, size_t length) {
    uint8_t reportId = 0;
    if (hasReportId) {
        if (length < 1)
            return;

        reportId = buf[0];
        buf++;
        length--;
    }
    _inputReport(reportId, buf, length);
}

void HIDReportHandler::_addInputField(const HIDField &field) {
    printf("Unhandled input field:");
    printf(
        " (reportId: %d) bit %d:%d - usage: %X:%X..%X (valid range: %d..%d) attributes: %X\n",
        field.reportID,
        field.bitIdx, field.bitSize,
        field.usagePage, field.usageMin, field.usageMax,
        (int)field.logicalMin, (int)field.logicalMax,
        (unsigned)field.attributes);
}

void HIDReportHandler::_addOutputField(const HIDField &field) {
#if 0
    printf("Unhandled output field:");
    printf(
        " (reportId: %d) bit %d:%d - usage: %X:%X..%X (valid range: %d..%d) attributes: %X\n",
        field.reportID,
        field.bitIdx, field.bitSize,
        field.usagePage, field.usageMin, field.usageMax,
        (int)field.logicalMin, (int)field.logicalMax,
        (unsigned)field.attributes);
#endif
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

HIDReportHandler *HIDReportHandler::getReportHandlersForDescriptor(const void *reportDescBuf, size_t reportDescLen) {
    // ESP_LOG_BUFFER_HEX(TAG, reportDescBuf, reportDescLen);

    auto collections = parseReportDescriptor(reportDescBuf, reportDescLen);

    HIDReportHandler *reportHandlers = nullptr;

    for (auto collection : collections) {
        if (collection->collectionType != HIDCollection::CollectionType::Application)
            continue;

        ESP_LOGI(TAG, "- Application collection %04X:%04X", collection->usagePage, collection->usage);

        // collection->dump(false);

        HIDReportHandler *reportHandler = NULL;

        uint32_t usage = ((uint32_t)collection->usagePage << 16) | collection->usage;
        switch (usage) {
            case 0x10002:
                ESP_LOGI(TAG, "  -> Mouse detected");
                reportHandler = new HIDReportHandlerMouse();
                break;

            case 0x10005:
                ESP_LOGI(TAG, "  -> Gamepad detected");
                reportHandler = new HIDReportHandlerGamepad();
                break;

            case 0x10006:
                ESP_LOGI(TAG, "  -> Keyboard detected");
                reportHandler = new HIDReportHandlerKeyboard();
                break;

            default:
                break;
        }

        if (reportHandler) {
            if (!reportHandler->init(collection.get())) {
                ESP_LOGE(TAG, "Could not init report handler.");
                delete reportHandler;
                reportHandler = NULL;
            }
        }

        if (reportHandler) {
            reportHandler->next = reportHandlers;
            reportHandlers      = reportHandler;
        }
    }
    return reportHandlers;
}
