#include "HIDReportDescriptor.h"

HIDReportDescriptor *HIDReportDescriptor::parseReportDescriptor(const void *descriptor, size_t descriptorLength) {
    HIDReportDescriptor *reportDesc        = new HIDReportDescriptor();
    HIDCollection       *currentCollection = NULL;

    struct GlobalItems {
        GlobalItems()
            : itemUsed(0), usagePage(0), logicalMinimum(0), logicalMaximum(0),
              physicalMinimum(0), physicalMaximum(0), unitExponent(0), unit(0),
              reportSize(0), reportID(0), reportCount(0), next(NULL) {
        }

        ~GlobalItems() {
            if (next) {
                delete next;
            }
        }

        uint32_t itemUsed;

        uint32_t usagePage;
        uint32_t logicalMinimum;
        uint32_t logicalMaximum;
        uint32_t physicalMinimum;
        uint32_t physicalMaximum;
        uint32_t unitExponent;
        uint32_t unit;
        uint32_t reportSize;
        uint8_t  reportID;
        uint32_t reportCount;

        uint8_t getReportID() {
            if (itemUsed & (1 << 8)) {
                return reportID;
            }
            return 0;
        }

        GlobalItems *next;

        void Push() {
            GlobalItems *newStackItem     = new GlobalItems();
            newStackItem->itemUsed        = itemUsed;
            newStackItem->usagePage       = usagePage;
            newStackItem->logicalMinimum  = logicalMinimum;
            newStackItem->logicalMaximum  = logicalMaximum;
            newStackItem->physicalMinimum = physicalMinimum;
            newStackItem->physicalMaximum = physicalMaximum;
            newStackItem->unitExponent    = unitExponent;
            newStackItem->unit            = unit;
            newStackItem->reportSize      = reportSize;
            newStackItem->reportID        = reportID;
            newStackItem->reportCount     = reportCount;
            newStackItem->next            = next;
            next                          = newStackItem;
        }

        void Pop() {
            if (!next) {
                return;
            }

            GlobalItems *stackItem = next;
            next                   = next->next;

            itemUsed        = stackItem->itemUsed;
            usagePage       = stackItem->usagePage;
            logicalMinimum  = stackItem->logicalMinimum;
            logicalMaximum  = stackItem->logicalMaximum;
            physicalMinimum = stackItem->physicalMinimum;
            physicalMaximum = stackItem->physicalMaximum;
            unitExponent    = stackItem->unitExponent;
            unit            = stackItem->unit;
            reportSize      = stackItem->reportSize;
            reportID        = stackItem->reportID;
            reportCount     = stackItem->reportCount;

            delete stackItem;
        }
    };

    struct LocalItems {
        LocalItems()
            : itemUsed(0), usageMinimum(0), usageMaximum(0),
              designatorIndex(0), designatorMinimum(0), designatorMaximum(0),
              stringIndex(0), stringMinimum(0), stringMaximum(0), delimiter(0),
              usages(NULL) {
        }

        ~LocalItems() {
            if (usages) {
                delete usages;
            }
        }

        uint32_t itemUsed;
        uint32_t usageMinimum;
        uint32_t usageMaximum;
        uint32_t designatorIndex;
        uint32_t designatorMinimum;
        uint32_t designatorMaximum;
        uint32_t stringIndex;
        uint32_t stringMinimum;
        uint32_t stringMaximum;
        uint32_t delimiter;

        struct Usage {
            Usage(uint32_t usageMin, uint32_t usageMax)
                : usageMin(usageMin), usageMax(usageMax), next(NULL) {
            }
            ~Usage() {
                if (next) {
                    delete next;
                }
            }

            uint32_t usageMin;
            uint32_t usageMax;
            Usage   *next;
        };

        Usage *usages;

        void Clear() {
            itemUsed = 0;
            if (usages) {
                delete usages;
                usages = NULL;
            }
        }

        void AddUsage(uint32_t minUsage, uint32_t maxUsage) {
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

            itemUsed &= ~((1 << 1) | (1 << 2));
        }
    };

    GlobalItems global;
    LocalItems  local;

    size_t         len  = 0;
    const uint8_t *data = (const uint8_t *)descriptor;

    while (len < descriptorLength) {
        if (data[0] == 0xFE) {
            // Skip long item
            int size = data[1];
            data += 3 + size;
            len += 3 + size;
        } else {
            // Read short item
            int      bSize = (data[0] >> 0) & 0x3;
            int      bType = (data[0] >> 2) & 0x3;
            int      bTag  = (data[0] >> 4) & 0xF;
            uint32_t value;

            switch (bSize) {
                default:
                case 0: value = 0; break;
                case 1: value = data[1]; break;
                case 2: value = (data[2] << 8) | data[1]; break;
                case 3: value = (data[4] << 24) | (data[3] << 16) | (data[2] << 8) | data[1]; break;
            }

            {
                // Determine item length
                size_t itemLength = 1;
                if (bSize == 3) {
                    itemLength += 4;
                } else {
                    itemLength += bSize;
                }
                data += itemLength;
                len += itemLength;
            }

            switch (bType) {
                // Main item
                case 0: {
                    if (bTag == 10) { // Collection
                        uint16_t usagePage = 0, usage = 0;

                        if (global.itemUsed & (1 << 0)) {
                            usagePage = global.usagePage;
                        }

                        if (local.usages) {
                            if ((local.usages->usageMin & 0xFFFF0000) != 0) {
                                usagePage = local.usages->usageMin >> 16;
                            }
                            usage = local.usages->usageMin & 0xFFFF;
                        }

                        HIDCollection *newCollection = new HIDCollection((HIDCollection::CollectionType)value, usagePage, usage);
                        if (currentCollection) {
                            currentCollection->addSubItem(newCollection);
                        } else if (reportDesc->items) {
                            reportDesc->items->addItem(newCollection);
                        } else {
                            reportDesc->items = newCollection;
                        }

                        currentCollection = newCollection;

                    } else if (bTag == 12) { // End collection
                        if (currentCollection) {
                            currentCollection = static_cast<HIDCollection *>(currentCollection->parent);
                        }
                    } else if (bTag == 8 || bTag == 9 || bTag == 11) { // Input/output/feature
                        uint32_t      attributes = value;
                        HIDItem::Type type;

                        HIDReportDescriptor::ReportInfo *reportInfo = NULL;
                        if (bTag == 8) {
                            reportInfo = &reportDesc->inputReportInfo;
                            type       = HIDItem::TInputField;
                        } else if (bTag == 9) {
                            reportInfo = &reportDesc->outputReportInfo;
                            type       = HIDItem::TOutputField;
                        } else { // (bTag == 11)
                            reportInfo = &reportDesc->featureReportInfo;
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
                                if (attributes & (1 << 1)) {
                                    uint32_t numUsages = 0;
                                    uint32_t curUsage  = 0;

                                    for (unsigned int i = 0; i < global.reportCount; i++) {
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

                                        HIDField *newField = new HIDField(
                                            type,
                                            global.getReportID(),
                                            bitOffset + i * global.reportSize, global.reportSize, 1,
                                            usagePage, curUsage, curUsage,
                                            global.logicalMinimum, global.logicalMaximum,
                                            attributes);

                                        if (currentCollection) {
                                            currentCollection->addSubItem(newField);
                                        } else if (reportDesc->items) {
                                            reportDesc->items->addItem(newField);
                                        } else {
                                            reportDesc->items = newField;
                                        }

                                        numUsages--;
                                        if (numUsages == 0) {
                                            if (usages->next) {
                                                usages = usages->next;
                                            }
                                        }
                                    }

                                } else {
                                    uint16_t usagePage = global.usagePage;
                                    uint32_t usageMin  = usages->usageMin;
                                    uint32_t usageMax  = usages->usageMax;

                                    if (usageMin & 0xFFFF0000) {
                                        usagePage = usageMin >> 16;
                                        usageMin &= 0xFFFF;
                                        usageMax &= 0xFFFF;
                                    }

                                    HIDField *newField = new HIDField(
                                        type,
                                        global.getReportID(),
                                        bitOffset, global.reportSize, global.reportCount,
                                        usagePage, usageMin, usageMax,
                                        global.logicalMinimum, global.logicalMaximum,
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

                        reportInfo->addBits(global.getReportID(), global.reportSize * global.reportCount);
                    }

                    // Clear local items
                    local.Clear();
                    break;
                }

                // Global item
                case 1: {
                    // Sign extend if logical or physical min/max
                    if (bTag == 1 || bTag == 2 || bTag == 3 || bTag == 4) {
                        switch (bSize) {
                            case 0: value = 0; break;
                            case 1: value = (int8_t)value; break;
                            case 2: value = (int16_t)value; break;
                            case 3: value = (int32_t)value; break;
                        }
                    }

                    switch (bTag) {
                        case 0: global.usagePage = value; break;
                        case 1: global.logicalMinimum = value; break;
                        case 2: global.logicalMaximum = value; break;
                        case 3: global.physicalMinimum = value; break;
                        case 4: global.physicalMaximum = value; break;
                        case 5: global.unitExponent = value; break;
                        case 6: global.unit = value; break;
                        case 7: global.reportSize = value; break;
                        case 8: global.reportID = value; break;
                        case 9: global.reportCount = value; break;
                        case 10: global.Push(); break;
                        case 11: global.Pop(); break;
                        default: break;
                    }
                    if (bTag <= 9) {
                        global.itemUsed |= (1 << bTag);
                    }
                    break;
                }

                // Local item
                case 2: {
                    switch (bTag) {
                        case 0: local.AddUsage(value, value); break;
                        case 1:
                            local.usageMinimum = value;
                            if (local.itemUsed & (1 << 2)) {
                                local.AddUsage(local.usageMinimum, local.usageMaximum);
                            }
                            break;
                        case 2:
                            local.usageMaximum = value;
                            if (local.itemUsed & (1 << 1)) {
                                local.AddUsage(local.usageMinimum, local.usageMaximum);
                            }
                            break;
                        case 3: local.designatorIndex = value; break;
                        case 4: local.designatorMinimum = value; break;
                        case 5: local.designatorMaximum = value; break;
                        case 7: local.stringIndex = value; break;
                        case 8: local.stringMinimum = value; break;
                        case 9: local.stringMaximum = value; break;
                        case 10: break; // delimiter
                        default: break;
                    }
                    if (bTag <= 10 && bTag != 6) {
                        local.itemUsed |= (1 << bTag);
                    }
                    break;
                }

                default: {
                    break;
                }
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
        for (int i = 0; i < depth; i++) {
            printf(". ");
        }

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

            printf("Collection %s - usage: %04X:%04X\n", typeStr, coll->usagePage, coll->usage);

            dumpItems(coll->subItems, depth + 1);

        } else {
            const HIDField *field = static_cast<const HIDField *>(item);

            switch (field->type) {
                case HIDItem::TInputField: printf("Input"); break;
                case HIDItem::TOutputField: printf("Output"); break;
                case HIDItem::TFeatureField: printf("Feature"); break;
                default: printf("Unknown"); break;
            }

            printf(
                " (reportId: %d) bit %d:%d (%ux) - usage: %X:%X..%X (valid range: %d..%d) attributes: %X\n",
                field->reportID,
                field->bitIdx, field->bitSize,
                (unsigned)field->arraySize,
                field->usagePage,
                field->usageMin, field->usageMax,
                (int)field->minVal, (int)field->maxVal,
                (unsigned)field->attributes);
        }

        item = item->next;
    }
}
