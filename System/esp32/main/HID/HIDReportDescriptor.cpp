#include "HIDReportDescriptor.h"

//   7  6  5  4  3  2  1  0
// +-----------+-----+-----+
// |   bTag    |bType|bSize|
// +-----------+-----+-----+

// bType values
enum {
    EType_MainItem   = 0,
    EType_GlobalItem = 1,
    EType_LocalItem  = 2,
};

// Main Item bTag values
enum {
    ETagM_Input         = 8,
    ETagM_Output        = 9,
    ETagM_Feature       = 11,
    ETagM_Collection    = 10,
    ETagM_EndCollection = 12,
};

// Global Item bTag values
enum {
    ETagG_UsagePage       = 0,
    ETagG_LogicalMinimum  = 1,
    ETagG_LogicalMaximum  = 2,
    ETagG_PhysicalMinimum = 3,
    ETagG_PhysicalMaximum = 4,
    ETagG_UnitExponent    = 5,
    ETagG_Unit            = 6,
    ETagG_ReportSize      = 7,
    ETagG_ReportID        = 8,
    ETagG_ReportCount     = 9,
    ETagG_Push            = 10,
    ETagG_Pop             = 11,
};

// Local Item bTag values
enum {
    ETagL_Usage             = 0,
    ETagL_UsageMinimum      = 1,
    ETagL_UsageMaximum      = 2,
    ETagL_DesignatorIndex   = 3,
    ETagL_DesignatorMinimum = 4,
    ETagL_DesignatorMaximum = 5,
    ETagL_StringIndex       = 7,
    ETagL_StringMinimum     = 8,
    ETagL_StringMaximum     = 9,
    ETagL_Delimiter         = 10,
};

struct GlobalItems {
    GlobalItems() {
    }

    ~GlobalItems() {
        if (next) {
            delete next;
        }
    }

    uint32_t     itemUsed     = 0; // Bitmap indicating which tags are valid
    uint32_t     usagePage    = 0; // bTag 0
    uint32_t     logicalMin   = 0; // bTag 1
    uint32_t     logicalMax   = 0; // bTag 2
    uint32_t     physicalMin  = 0; // bTag 3
    uint32_t     physicalMax  = 0; // bTag 4
    int32_t      unitExponent = 0; // bTag 5
    uint32_t     unit         = 0; // bTag 6
    uint32_t     reportSize   = 0; // bTag 7
    uint8_t      reportID     = 0; // bTag 8
    uint32_t     reportCount  = 0; // bTag 9
    GlobalItems *next         = nullptr;

    uint8_t getReportID() {
        if (itemUsed & (1 << ETagG_ReportID)) {
            return reportID;
        }
        return 0;
    }

    void push() {
        GlobalItems *newStackItem  = new GlobalItems();
        newStackItem->itemUsed     = itemUsed;
        newStackItem->usagePage    = usagePage;
        newStackItem->logicalMin   = logicalMin;
        newStackItem->logicalMax   = logicalMax;
        newStackItem->physicalMin  = physicalMin;
        newStackItem->physicalMax  = physicalMax;
        newStackItem->unitExponent = unitExponent;
        newStackItem->unit         = unit;
        newStackItem->reportSize   = reportSize;
        newStackItem->reportID     = reportID;
        newStackItem->reportCount  = reportCount;
        newStackItem->next         = next;
        next                       = newStackItem;
    }

    void pop() {
        if (!next) {
            return;
        }

        GlobalItems *stackItem = next;
        next                   = next->next;

        itemUsed     = stackItem->itemUsed;
        usagePage    = stackItem->usagePage;
        logicalMin   = stackItem->logicalMin;
        logicalMax   = stackItem->logicalMax;
        physicalMin  = stackItem->physicalMin;
        physicalMax  = stackItem->physicalMax;
        unitExponent = stackItem->unitExponent;
        unit         = stackItem->unit;
        reportSize   = stackItem->reportSize;
        reportID     = stackItem->reportID;
        reportCount  = stackItem->reportCount;

        delete stackItem;
    }
};

struct LocalItems {
    LocalItems() {
    }

    ~LocalItems() {
        if (usages) {
            delete usages;
        }
    }

    struct Usage {
        Usage(uint32_t _usageMin, uint32_t _usageMax)
            : usageMin(_usageMin), usageMax(_usageMax) {
        }
        ~Usage() {
            if (next) {
                delete next;
            }
        }

        uint32_t usageMin;
        uint32_t usageMax;
        Usage   *next = nullptr;
    };

    Usage   *usages            = nullptr;
    uint32_t itemUsed          = 0; // Bitmap indicating which tags are valid
    uint32_t usageMinimum      = 0; // bTag 1
    uint32_t usageMaximum      = 0; // bTag 2
    uint32_t designatorIndex   = 0; // bTag 3
    uint32_t designatorMinimum = 0; // bTag 4
    uint32_t designatorMaximum = 0; // bTag 5
    uint32_t stringIndex       = 0; // bTag 7
    uint32_t stringMinimum     = 0; // bTag 8
    uint32_t stringMaximum     = 0; // bTag 9
    uint32_t delimiter         = 0; // bTag 10

    void clear() {
        itemUsed = 0;
        if (usages) {
            delete usages;
            usages = nullptr;
        }
    }

    void addUsage(uint32_t minUsage, uint32_t maxUsage) {
        Usage *newUsage = new Usage(minUsage, maxUsage);
        if (!usages) {
            usages = newUsage;
        } else {
            Usage *list = usages;
            while (list->next) {
                list = list->next;
            }
            list->next = newUsage;
        }
        itemUsed &= ~((1 << usageMinimum) | (1 << usageMaximum));
    }
};

struct ReportInfo {
    struct ReportID {
        ReportID(uint8_t _id)
            : id(_id) {
        }
        ~ReportID() {
            if (next) {
                delete next;
            }
        }

        uint8_t   id;
        uint32_t  size = 0;
        ReportID *next = nullptr;
    };

    ReportInfo() {
    }
    ~ReportInfo() {
        if (reportIDs) {
            delete reportIDs;
        }
    }

    ReportID *reportIDs   = nullptr;
    bool      noReportIDs = false;

    bool getReportID(uint8_t id, ReportID *&reportID) {
        if (id == 0) {
            noReportIDs = true;

            if (!reportIDs) {
                reportIDs = new ReportID(0);
            }
            reportID = reportIDs;
            return true;
        }
        if (noReportIDs)
            return false;

        // Check if already used
        reportID               = reportIDs;
        ReportID *lastReportID = nullptr;
        while (reportID) {
            if (reportID->id == id)
                return true;

            lastReportID = reportID;
            reportID     = reportID->next;
        }

        reportID = new ReportID(id);
        if (lastReportID) {
            lastReportID->next = reportID;
        } else {
            reportIDs = reportID;
        }
        return true;
    }

    bool addBits(uint8_t id, uint32_t numBits) {
        ReportID *reportID;
        if (!getReportID(id, reportID)) {
            return false;
        }
        reportID->size += numBits;
        return true;
    }

    bool getBitOffset(uint8_t id, uint32_t &bitOffset) {
        ReportID *reportID;
        if (!getReportID(id, reportID)) {
            return false;
        }
        bitOffset = reportID->size;
        return true;
    }
};

HIDReportDescriptor *HIDReportDescriptor::parseReportDescriptor(const void *descriptor, size_t descriptorLength) {
    auto           reportDesc        = new HIDReportDescriptor();
    HIDCollection *currentCollection = nullptr;
    GlobalItems    global;
    LocalItems     local;
    size_t         len  = 0;
    const uint8_t *data = (const uint8_t *)descriptor;

    ReportInfo inputReportInfo;
    ReportInfo outputReportInfo;
    ReportInfo featureReportInfo;

    while (len < descriptorLength) {
        if (data[0] == 0xFE) {
            // Skip long item
            int size = data[1];
            data += 3 + size;
            len += 3 + size;
        } else {
            // Read short item
            unsigned bSize = (data[0] >> 0) & 0x3;
            unsigned bType = (data[0] >> 2) & 0x3;
            unsigned bTag  = (data[0] >> 4) & 0xF;

            // Determine length
            size_t itemLength = 1 + ((bSize == 3) ? 4 : bSize);
            if (len + itemLength > descriptorLength)
                break;

            // Read value
            uint32_t value;
            switch (bSize) {
                default:
                case 0: value = 0; break;
                case 1: value = data[1]; break;
                case 2: value = (data[2] << 8) | data[1]; break;
                case 3: value = (data[4] << 24) | (data[3] << 16) | (data[2] << 8) | data[1]; break;
            }

            // Increment data pointer
            data += itemLength;
            len += itemLength;

            switch (bType) {
                // Main item
                case EType_MainItem: {
                    // Collection
                    if (bTag == ETagM_Collection) {
                        uint16_t usagePage = (global.itemUsed & (1 << ETagG_UsagePage)) ? global.usagePage : 0;
                        uint16_t usage     = 0;

                        if (local.usages) {
                            if ((local.usages->usageMin & 0xFFFF0000) != 0) {
                                usagePage = local.usages->usageMin >> 16;
                            }
                            usage = local.usages->usageMin & 0xFFFF;
                        }

                        // Create new collection
                        auto newCollection = new HIDCollection((HIDCollection::CollectionType)value, usagePage, usage);
                        if (currentCollection) {
                            currentCollection->addSubItem(newCollection);
                        } else if (reportDesc->items) {
                            reportDesc->items->addItem(newCollection);
                        } else {
                            reportDesc->items = newCollection;
                        }
                        currentCollection = newCollection;
                    }

                    // End collection
                    else if (bTag == ETagM_EndCollection) {
                        if (currentCollection) {
                            currentCollection = static_cast<HIDCollection *>(currentCollection->parent);
                        }
                    }

                    // Input/output/feature
                    else if (bTag == ETagM_Input || bTag == ETagM_Output || bTag == ETagM_Feature) {
                        uint32_t      attributes = value;
                        HIDItem::Type type;

                        ReportInfo *reportInfo = nullptr;
                        if (bTag == ETagM_Input) {
                            reportInfo = &inputReportInfo;
                            type       = HIDItem::TInputField;
                        } else if (bTag == ETagM_Output) {
                            reportInfo = &outputReportInfo;
                            type       = HIDItem::TOutputField;
                        } else { // (bTag == ETagM_Feature)
                            reportInfo = &featureReportInfo;
                            type       = HIDItem::TFeatureField;
                        }

                        // Skip constant fields
                        if ((attributes & (1 << 0)) == 0) {
                            uint8_t reportID = global.getReportID();

                            uint32_t bitOffset = 0;
                            if (!reportInfo->getBitOffset(reportID, bitOffset)) {
                                printf("Error in reportID\n");
                            }

                            LocalItems::Usage *usages = local.usages;
                            if (usages) {
                                // Variable
                                if (attributes & (1 << 1)) {
                                    uint32_t numUsages = 0;
                                    uint32_t curUsage  = 0;

                                    for (unsigned i = 0; i < global.reportCount; i++) {
                                        if (numUsages == 0) {
                                            numUsages = (usages->usageMax - usages->usageMin) + 1;
                                            curUsage  = usages->usageMin;
                                        } else {
                                            curUsage++;
                                        }

                                        uint16_t usagePage = global.usagePage;
                                        if (curUsage & 0xFFFF0000) {
                                            usagePage = curUsage >> 16;
                                            curUsage &= 0xFFFF;
                                        }

                                        // Ignore vendor specific usage pages
                                        if (usagePage < 0xFF00) {
                                            auto newField = new HIDField(
                                                type,
                                                global.getReportID(),
                                                bitOffset + i * global.reportSize, global.reportSize, 1,
                                                usagePage, curUsage, curUsage,
                                                global.logicalMin, global.logicalMax,
                                                global.physicalMin, global.physicalMax,
                                                global.unitExponent, global.unit,
                                                attributes);

                                            if (currentCollection) {
                                                currentCollection->addSubItem(newField);
                                            } else if (reportDesc->items) {
                                                reportDesc->items->addItem(newField);
                                            } else {
                                                reportDesc->items = newField;
                                            }
                                        }

                                        numUsages--;
                                        if (numUsages == 0) {
                                            if (usages->next) {
                                                usages = usages->next;
                                            }
                                        }
                                    }

                                }

                                // Array
                                else {
                                    uint16_t usagePage = global.usagePage;
                                    uint32_t usageMin  = usages->usageMin;
                                    uint32_t usageMax  = usages->usageMax;

                                    if (usageMin & 0xFFFF0000) {
                                        usagePage = usageMin >> 16;
                                        usageMin &= 0xFFFF;
                                        usageMax &= 0xFFFF;
                                    }

                                    // Ignore vendor specific usage pages
                                    if (usagePage < 0xFF00) {
                                        HIDField *newField = new HIDField(
                                            type,
                                            global.getReportID(),
                                            bitOffset, global.reportSize, global.reportCount,
                                            usagePage, usageMin, usageMax,
                                            global.logicalMin, global.logicalMax,
                                            global.physicalMin, global.physicalMax,
                                            global.unitExponent, global.unit,
                                            attributes);

                                        if (currentCollection) {
                                            currentCollection->addSubItem(newField);
                                        } else if (reportDesc->items) {
                                            reportDesc->items->addItem(newField);
                                        } else {
                                            reportDesc->items = newField;
                                        }
                                    }
                                }
                            }
                        }

                        reportInfo->addBits(global.getReportID(), global.reportSize * global.reportCount);
                    }

                    // Clear local items
                    local.clear();
                    break;
                }

                // Global item
                case 1: {
                    // Sign extend if logical, physical min/max or unit exponent
                    if (bTag == ETagG_LogicalMinimum || bTag == ETagG_LogicalMaximum ||
                        bTag == ETagG_PhysicalMinimum || bTag == ETagG_PhysicalMaximum ||
                        bTag == ETagG_UnitExponent) {

                        switch (bSize) {
                            case 0: value = 0; break;
                            case 1: value = (int8_t)value; break;
                            case 2: value = (int16_t)value; break;
                            case 3: value = (int32_t)value; break;
                        }
                    }

                    switch (bTag) {
                        case ETagG_UsagePage: global.usagePage = value; break;
                        case ETagG_LogicalMinimum: global.logicalMin = value; break;
                        case ETagG_LogicalMaximum: global.logicalMax = value; break;
                        case ETagG_PhysicalMinimum: global.physicalMin = value; break;
                        case ETagG_PhysicalMaximum: global.physicalMax = value; break;
                        case ETagG_UnitExponent: global.unitExponent = value; break;
                        case ETagG_Unit: global.unit = value; break;
                        case ETagG_ReportSize: global.reportSize = value; break;
                        case ETagG_ReportID: global.reportID = value; break;
                        case ETagG_ReportCount: global.reportCount = value; break;
                        case ETagG_Push: global.push(); break;
                        case ETagG_Pop: global.pop(); break;
                        default: break;
                    }
                    if (bTag <= ETagG_ReportCount)
                        global.itemUsed |= (1 << bTag);
                    break;
                }

                // Local item
                case 2: {
                    switch (bTag) {
                        case ETagL_Usage: local.addUsage(value, value); break;
                        case ETagL_UsageMinimum:
                            local.usageMinimum = value;
                            if (local.itemUsed & (1 << 2)) {
                                local.addUsage(local.usageMinimum, local.usageMaximum);
                            }
                            break;
                        case ETagL_UsageMaximum:
                            local.usageMaximum = value;
                            if (local.itemUsed & (1 << 1)) {
                                local.addUsage(local.usageMinimum, local.usageMaximum);
                            }
                            break;
                        case ETagL_DesignatorIndex: local.designatorIndex = value; break;
                        case ETagL_DesignatorMinimum: local.designatorMinimum = value; break;
                        case ETagL_DesignatorMaximum: local.designatorMaximum = value; break;
                        case ETagL_StringIndex: local.stringIndex = value; break;
                        case ETagL_StringMinimum: local.stringMinimum = value; break;
                        case ETagL_StringMaximum: local.stringMaximum = value; break;
                        case ETagL_Delimiter: break;
                        default: break;
                    }
                    if (bTag <= ETagL_Delimiter && bTag != 6) {
                        local.itemUsed |= (1 << bTag);
                    }
                    break;
                }

                default: break;
            }
        }
    }
    return reportDesc;
}

void HIDReportDescriptor::dumpItems(const HIDItem *item, int depth) {
    if (!item) {
        if (depth > 0) {
            return;
        } else {
            item = items;
        }
    }

    while (item) {
        if (item->type == HIDItem::TCollection) {
            const HIDCollection *coll = static_cast<const HIDCollection *>(item);

            const char *typeStr;
            switch (coll->type) {
                case HIDCollection::CTPhysical: typeStr = "CP"; break;
                case HIDCollection::CTApplication: typeStr = "CA"; break;
                case HIDCollection::CTLogical: typeStr = "CL"; break;
                case HIDCollection::CTReport: typeStr = "Report"; break;
                case HIDCollection::CTNamedArray: typeStr = "NARy"; break;
                case HIDCollection::CTUsageSwitch: typeStr = "US"; break;
                case HIDCollection::CTUsageModifier: typeStr = "UM"; break;
                default: typeStr = "Unknown"; break;
            }

            printf("%*s Collection %s - usage: %04X:%04X\n", depth * 2, "-", typeStr, coll->usagePage, coll->usage);

            dumpItems(coll->subItems, depth + 1);

        } else {
            const HIDField *field = static_cast<const HIDField *>(item);

            const char *typeStr;
            switch (field->type) {
                case HIDItem::TInputField: typeStr = "Input"; break;
                case HIDItem::TOutputField: typeStr = "Output"; break;
                case HIDItem::TFeatureField: typeStr = "Feature"; break;
                default: typeStr = "Unknown"; break;
            }

            if (field->arraySize == 1) {
                printf(
                    "%*s %s (reportId: %d) bit %d:%d - usage: %X:%X (log. range: %d..%d  phys. range: %d..%d  unit: 0x%02X  unitExponent: %d) attributes: %X\n",
                    depth * 2, "-", typeStr,
                    field->reportID,
                    field->bitIdx, field->bitSize,
                    field->usagePage, field->usageMin,
                    (int)field->logicalMin, (int)field->logicalMax,
                    (int)field->physicalMin, (int)field->physicalMax,
                    (unsigned)field->unit, (int)field->unitExponent,
                    (unsigned)field->attributes);

            } else {
                printf(
                    "%*s %s (reportId: %d) bit %d:%d (%ux) - usage: %X:%X..%X (log. range: %d..%d  phys. range: %d..%d  unit: 0x%02X  unitExponent: %d) attributes: %X\n",
                    depth * 2, "-", typeStr,
                    field->reportID,
                    field->bitIdx, field->bitSize, (unsigned)field->arraySize,
                    field->usagePage, field->usageMin, field->usageMax,
                    (int)field->logicalMin, (int)field->logicalMax,
                    (int)field->physicalMin, (int)field->physicalMax,
                    (unsigned)field->unit, (int)field->unitExponent,
                    (unsigned)field->attributes);
            }
        }

        item = item->next;
    }
}
