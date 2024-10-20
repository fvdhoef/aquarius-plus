#pragma once

#include "Common.h"

struct HIDItem {
    enum class ItemType {
        Collection,
        InputField,
        OutputField,
        FeatureField,
    };

    HIDItem(ItemType _itemType, uint16_t _usagePage)
        : itemType(_itemType), usagePage(_usagePage) {
    }

    ItemType                              itemType;
    uint16_t                              usagePage;
    std::vector<std::shared_ptr<HIDItem>> subItems;

    void dump(bool recurseChildren = true, int depth = 0);
};

struct HIDCollection : public HIDItem {
    enum class CollectionType {
        Physical      = 0x00, // CP
        Application   = 0x01, // CA
        Logical       = 0x02, // CL
        Report        = 0x03,
        NamedArray    = 0x04, // NAry
        UsageSwitch   = 0x05, // US
        UsageModifier = 0x06  // UM
    };

    HIDCollection(CollectionType _collectionType, uint16_t _usagePage, uint16_t _usage)
        : HIDItem(HIDItem::ItemType::Collection, _usagePage), collectionType(_collectionType), usage(_usage) {
    }

    CollectionType collectionType;
    uint16_t       usage;
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
        ItemType _itemType,
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
        : HIDItem(_itemType, _usagePage),
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

std::vector<std::shared_ptr<HIDCollection>> parseReportDescriptor(const void *descriptor, size_t descriptorLength);
