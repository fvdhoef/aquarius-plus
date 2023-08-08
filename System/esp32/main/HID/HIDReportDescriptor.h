#pragma once

#include "Common.h"

class HIDReportDescriptor {
public:
    static HIDReportDescriptor *parseReportDescriptor(const void *descriptor, size_t descriptorLength);

    HIDReportDescriptor()
        : items(nullptr) {
    }
    ~HIDReportDescriptor() {
        if (items) {
            delete items;
        }
    }

    struct HIDItem {
        enum Type {
            TCollection,
            TInputField,
            TOutputField,
            TFeatureField,
        };

        Type     type;
        uint16_t usagePage;
        HIDItem *next   = nullptr;
        HIDItem *parent = nullptr;

        HIDItem(Type _type, uint16_t _usagePage)
            : type(_type), usagePage(_usagePage) {
        }

        virtual ~HIDItem() {
            if (next) {
                delete next;
            }
        }

        void addItem(HIDItem *item) {
            if (!next) {
                item->parent = parent;
                next         = item;
            } else {
                HIDItem *list = next;
                while (list->next) {
                    list = list->next;
                }
                item->parent = parent;
                list->next   = item;
            }
        }
    };

    struct HIDCollection : public HIDItem {
        enum CollectionType {
            CTPhysical      = 0x00, // CP
            CTApplication   = 0x01, // CA
            CTLogical       = 0x02, // CL
            CTReport        = 0x03,
            CTNamedArray    = 0x04, // NAry
            CTUsageSwitch   = 0x05, // US
            CTUsageModifier = 0x06  // UM
        };

        CollectionType type;
        uint16_t       usage;
        HIDItem       *subItems = nullptr;

        HIDCollection(CollectionType _type, uint16_t _usagePage, uint16_t _usage)
            : HIDItem(TCollection, _usagePage), type(_type), usage(_usage) {
        }

        virtual ~HIDCollection() {
            if (subItems) {
                delete subItems;
            }
        }

        void addSubItem(HIDItem *item) {
            if (!subItems) {
                item->parent = this;
                subItems     = item;
            } else {
                subItems->addItem(item);
            }
        }
    };

    struct HIDField : public HIDItem {
        uint8_t  reportID;
        int16_t  bitIdx;
        int16_t  bitSize;
        uint32_t arraySize;

        // From local item
        uint16_t usageMin;
        uint16_t usageMax;

        // From global item
        int32_t  logicalMin;
        int32_t  logicalMax;
        int32_t  physicalMin;
        int32_t  physicalMax;
        uint32_t unitExponent;
        uint32_t unit;

        uint32_t attributes;

        HIDField(
            Type     _type,
            uint8_t  _reportID,
            int16_t  _bitIdx,
            uint16_t _bitSize,
            uint16_t _arraySize,
            uint16_t _usagePage,
            uint16_t _usageMin,
            uint16_t _usageMax,
            int32_t  _logicalMin,
            int32_t  _logicalMax,
            int32_t  _physicalMin,
            int32_t  _physicalMax,
            int32_t  _unitExponent,
            uint32_t _unit,
            uint32_t _attributes)
            : HIDItem(_type, _usagePage),
              reportID(_reportID),
              bitIdx(_bitIdx),
              bitSize(_bitSize),
              arraySize(_arraySize),
              usageMin(_usageMin),
              usageMax(_usageMax),
              logicalMin(_logicalMin),
              logicalMax(_logicalMax),
              physicalMin(_physicalMin),
              physicalMax(_physicalMax),
              unitExponent(_unitExponent),
              unit(_unit),
              attributes(_attributes) {

            if (physicalMin == 0 && physicalMax == 0) {
                physicalMin = logicalMin;
                physicalMax = logicalMax;
            }
        }
    };

    void     dumpItems(const HIDItem *item = nullptr, int depth = 0);
    HIDItem *items;
};
