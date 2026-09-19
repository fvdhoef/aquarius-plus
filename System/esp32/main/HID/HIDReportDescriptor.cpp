#include "HIDReportDescriptor.h"
#include <optional>

void HIDItem::dump(bool recurseChildren, int depth) {
    if (itemType == HIDItem::ItemType::Collection) {
        auto collection = static_cast<const HIDCollection *>(this);

        const char *typeStr;
        switch (collection->collectionType) {
            case HIDCollection::CollectionType::Physical: typeStr = "CP"; break;
            case HIDCollection::CollectionType::Application: typeStr = "CA"; break;
            case HIDCollection::CollectionType::Logical: typeStr = "CL"; break;
            case HIDCollection::CollectionType::Report: typeStr = "Report"; break;
            case HIDCollection::CollectionType::NamedArray: typeStr = "NARy"; break;
            case HIDCollection::CollectionType::UsageSwitch: typeStr = "US"; break;
            case HIDCollection::CollectionType::UsageModifier: typeStr = "UM"; break;
            default: typeStr = "Unknown"; break;
        }
        printf("%*s Collection %s - usage: %04X:%04X\n", depth * 2, "-", typeStr, collection->usagePage, collection->usage);

    } else {
        const HIDField *field = static_cast<const HIDField *>(this);

        const char *typeStr;
        switch (field->itemType) {
            case HIDItem::ItemType::InputField: typeStr = "Input"; break;
            case HIDItem::ItemType::OutputField: typeStr = "Output"; break;
            case HIDItem::ItemType::FeatureField: typeStr = "Feature"; break;
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

    if (recurseChildren)
        for (auto subItem : subItems)
            subItem->dump(recurseChildren, depth + 1);
}

class HIDReportDescriptorParser {
public:
    enum class ItemType {
        MainItem   = 0,
        GlobalItem = 1,
        LocalItem  = 2,
    };

    enum class MainItemTag {
        Input         = 8,
        Output        = 9,
        Feature       = 11,
        Collection    = 10,
        EndCollection = 12,
    };

    enum class GlobalItemTag {
        UsagePage       = 0,
        LogicalMinimum  = 1,
        LogicalMaximum  = 2,
        PhysicalMinimum = 3,
        PhysicalMaximum = 4,
        UnitExponent    = 5,
        Unit            = 6,
        ReportSize      = 7,
        ReportID        = 8,
        ReportCount     = 9,
        Push            = 10,
        Pop             = 11,
    };

    enum class LocalItemTag {
        Usage             = 0,
        UsageMinimum      = 1,
        UsageMaximum      = 2,
        DesignatorIndex   = 3,
        DesignatorMinimum = 4,
        DesignatorMaximum = 5,
        StringIndex       = 7,
        StringMinimum     = 8,
        StringMaximum     = 9,
        Delimiter         = 10,
    };

    struct Global {
        std::optional<uint32_t> usagePage;
        std::optional<uint32_t> logicalMin;
        std::optional<uint32_t> logicalMax;
        std::optional<uint32_t> physicalMin;
        std::optional<uint32_t> physicalMax;
        std::optional<int32_t>  unitExponent;
        std::optional<uint32_t> unit;
        std::optional<uint32_t> reportSize;
        std::optional<uint8_t>  reportID;
        std::optional<uint32_t> reportCount;
    };

    struct Local {
        struct Usage {
            Usage(uint32_t _usageMin, uint32_t _usageMax)
                : usageMin(_usageMin), usageMax(_usageMax) {
            }
            uint32_t usageMin;
            uint32_t usageMax;
        };

        std::vector<Usage> usages;

        std::optional<uint32_t> usageMinimum;      // tag 1
        std::optional<uint32_t> usageMaximum;      // tag 2
        std::optional<uint32_t> designatorIndex;   // tag 3
        std::optional<uint32_t> designatorMinimum; // tag 4
        std::optional<uint32_t> designatorMaximum; // tag 5
        std::optional<uint32_t> stringIndex;       // tag 7
        std::optional<uint32_t> stringMinimum;     // tag 8
        std::optional<uint32_t> stringMaximum;     // tag 9
        std::optional<uint32_t> delimiter;         // tag 10

        void clear() {
            usages.clear();
            usageMinimum.reset();
            usageMaximum.reset();
            designatorIndex.reset();
            designatorMinimum.reset();
            designatorMaximum.reset();
            stringIndex.reset();
            stringMinimum.reset();
            stringMaximum.reset();
            delimiter.reset();
        }

        void addUsage(uint32_t minUsage, uint32_t maxUsage) {
            usages.emplace_back(minUsage, maxUsage);
            usageMinimum.reset();
            usageMaximum.reset();
        }
    };

    struct ReportInfo {
        struct ReportID {
            ReportID(uint8_t _id) : id(_id) {}
            uint8_t  id;
            uint32_t size = 0;
        };

        std::map<uint8_t, ReportID> reportIDs;
        bool                        noReportIDs = false;

        ReportID *getReportID(uint8_t id) {
            // Check if already used
            auto it = reportIDs.find(id);
            if (it != reportIDs.end()) {
                return &it->second;
            }
            if (noReportIDs)
                return nullptr;

            if (id == 0)
                noReportIDs = true;

            auto result = reportIDs.emplace(id, id);
            return &(result.first)->second;
        }

        bool addBits(uint8_t id, uint32_t numBits) {
            auto reportID = getReportID(id);
            if (reportID) {
                reportID->size += numBits;
                return true;
            }
            return false;
        }

        bool getBitOffset(uint8_t id, uint32_t &bitOffset) {
            auto reportID = getReportID(id);
            if (reportID) {
                bitOffset = reportID->size;
                return true;
            }
            return false;
        }
    };

    Global                                      global;
    std::vector<Global>                         globalStack;
    Local                                       local;
    ReportInfo                                  inputReportInfo;
    ReportInfo                                  outputReportInfo;
    ReportInfo                                  featureReportInfo;
    std::vector<std::shared_ptr<HIDCollection>> collections;
    std::vector<std::shared_ptr<HIDCollection>> collectionStack;

    void handleMainItem(MainItemTag tag, unsigned value) {
        // printf("handleMainItem(%u) value=%u\n", (unsigned)tag, value);

        // Collection
        if (tag == MainItemTag::Collection) {
            uint16_t usagePage = global.usagePage.value_or(0);
            uint16_t usage     = 0;

            if (!local.usages.empty()) {
                if ((local.usages[0].usageMin & 0xFFFF0000) != 0) {
                    usagePage = local.usages[0].usageMin >> 16;
                }
                usage = local.usages[0].usageMin & 0xFFFF;
            }

            // Create new collection
            auto collection = std::make_shared<HIDCollection>((HIDCollection::CollectionType)value, usagePage, usage);
            if (collectionStack.empty()) {
                // This is a top-level collection
                collections.push_back(collection);
            } else {
                collectionStack.back()->subItems.push_back(collection);
            }
            collectionStack.push_back(collection);
        }

        // End collection
        else if (tag == MainItemTag::EndCollection) {
            if (!collectionStack.empty())
                collectionStack.pop_back();
        }

        // Input/output/feature
        else if (tag == MainItemTag::Input || tag == MainItemTag::Output || tag == MainItemTag::Feature) {
            uint32_t          attributes = value;
            HIDItem::ItemType type;

            ReportInfo *reportInfo = nullptr;
            if (tag == MainItemTag::Input) {
                reportInfo = &inputReportInfo;
                type       = HIDItem::ItemType::InputField;
            } else if (tag == MainItemTag::Output) {
                reportInfo = &outputReportInfo;
                type       = HIDItem::ItemType::OutputField;
            } else { // (tag == MainItemTag::Feature)
                reportInfo = &featureReportInfo;
                type       = HIDItem::ItemType::FeatureField;
            }

            // Skip constant fields
            if ((attributes & (1 << 0)) == 0) {
                uint8_t reportID = global.reportID.value_or(0);

                uint32_t bitOffset = 0;
                if (!reportInfo->getBitOffset(reportID, bitOffset)) {
                    printf("Error in reportID\n");
                }

                int usagesIdx = 0;
                if (!local.usages.empty()) {
                    // Variable
                    if (attributes & (1 << 1)) {
                        uint32_t numUsages = 0;
                        uint32_t curUsage  = 0;

                        for (unsigned i = 0; i < global.reportCount; i++) {
                            if (numUsages == 0) {
                                numUsages = (local.usages[usagesIdx].usageMax - local.usages[usagesIdx].usageMin) + 1;
                                curUsage  = local.usages[usagesIdx].usageMin;
                            } else {
                                curUsage++;
                            }

                            uint16_t usagePage = global.usagePage.value_or(0);
                            if (curUsage & 0xFFFF0000) {
                                usagePage = curUsage >> 16;
                                curUsage &= 0xFFFF;
                            }

                            // Ignore vendor specific usage pages
                            if (usagePage < 0xFF00 && global.reportSize.has_value()) {
                                auto newField = std::make_shared<HIDField>(
                                    type,
                                    global.reportID.value_or(0),
                                    bitOffset + i * global.reportSize.value(), global.reportSize.value(), 1,
                                    usagePage, curUsage, curUsage,
                                    global.logicalMin.value_or(0), global.logicalMax.value_or(0),
                                    global.physicalMin.value_or(0), global.physicalMax.value_or(0),
                                    global.unitExponent.value_or(0), global.unit.value_or(0),
                                    attributes);

                                if (!collectionStack.empty()) {
                                    collectionStack.back()->subItems.push_back(newField);
                                }
                            }

                            numUsages--;
                            if (numUsages == 0) {
                                if (++usagesIdx == (int)local.usages.size())
                                    break;
                            }
                        }
                    }

                    // Array
                    else {
                        uint16_t usagePage = global.usagePage.value_or(0);
                        uint32_t usageMin  = local.usages[0].usageMin;
                        uint32_t usageMax  = local.usages[0].usageMax;

                        if (usageMin & 0xFFFF0000) {
                            usagePage = usageMin >> 16;
                            usageMin &= 0xFFFF;
                            usageMax &= 0xFFFF;
                        }

                        // Ignore vendor specific usage pages
                        if (usagePage < 0xFF00 && global.reportSize.has_value() && global.reportCount.has_value()) {
                            auto newField = std::make_shared<HIDField>(
                                type,
                                global.reportID.value_or(0),
                                bitOffset, global.reportSize.value(), global.reportCount.value(),
                                usagePage, usageMin, usageMax,
                                global.logicalMin.value_or(0), global.logicalMax.value_or(0),
                                global.physicalMin.value_or(0), global.physicalMax.value_or(0),
                                global.unitExponent.value_or(0), global.unit.value_or(0),
                                attributes);

                            if (!collectionStack.empty()) {
                                collectionStack.back()->subItems.push_back(newField);
                            }
                        }
                    }
                }
            }

            if (global.reportSize.has_value() && global.reportCount.has_value())
                reportInfo->addBits(global.reportID.value_or(0), global.reportSize.value() * global.reportCount.value());
        }

        // Clear local items
        local.clear();
    }

    void handleGlobalItem(GlobalItemTag tag, unsigned size, uint32_t value) {
        // Sign extend if logical, physical min/max
        if (tag == GlobalItemTag::LogicalMinimum || tag == GlobalItemTag::LogicalMaximum ||
            tag == GlobalItemTag::PhysicalMinimum || tag == GlobalItemTag::PhysicalMaximum) {

            switch (size) {
                case 1: value = (int8_t)value; break;
                case 2: value = (int16_t)value; break;
                case 3: value = (int32_t)value; break;
                default: break;
            }
        }

        switch (tag) {
            case GlobalItemTag::UsagePage: global.usagePage = value; break;
            case GlobalItemTag::LogicalMinimum: global.logicalMin = value; break;
            case GlobalItemTag::LogicalMaximum: global.logicalMax = value; break;
            case GlobalItemTag::PhysicalMinimum: global.physicalMin = value; break;
            case GlobalItemTag::PhysicalMaximum: global.physicalMax = value; break;
            case GlobalItemTag::UnitExponent: global.unitExponent = value; break;
            case GlobalItemTag::Unit: global.unit = value; break;
            case GlobalItemTag::ReportSize: global.reportSize = value; break;
            case GlobalItemTag::ReportID: global.reportID = value; break;
            case GlobalItemTag::ReportCount: global.reportCount = value; break;
            case GlobalItemTag::Push: globalStack.push_back(global); break;
            case GlobalItemTag::Pop:
                global = globalStack.back();
                globalStack.pop_back();
                break;
            default: break;
        }
    }

    void handleLocalItem(LocalItemTag tag, unsigned value) {
        switch (tag) {
            case LocalItemTag::Usage: local.addUsage(value, value); break;
            case LocalItemTag::UsageMinimum:
                local.usageMinimum = value;
                if (local.usageMaximum.has_value())
                    local.addUsage(local.usageMinimum.value(), local.usageMaximum.value());
                break;
            case LocalItemTag::UsageMaximum:
                local.usageMaximum = value;
                if (local.usageMinimum.has_value())
                    local.addUsage(local.usageMinimum.value(), local.usageMaximum.value());
                break;
            case LocalItemTag::DesignatorIndex: local.designatorIndex = value; break;
            case LocalItemTag::DesignatorMinimum: local.designatorMinimum = value; break;
            case LocalItemTag::DesignatorMaximum: local.designatorMaximum = value; break;
            case LocalItemTag::StringIndex: local.stringIndex = value; break;
            case LocalItemTag::StringMinimum: local.stringMinimum = value; break;
            case LocalItemTag::StringMaximum: local.stringMaximum = value; break;
            case LocalItemTag::Delimiter: break;
            default: break;
        }
    }

    std::vector<std::shared_ptr<HIDCollection>> parseReportDescriptor(const void *descriptor, size_t descriptorLength) {
        const uint8_t *data = (const uint8_t *)descriptor;
        size_t         len  = 0;

        while (len < descriptorLength) {
            if (data[0] == 0xFE) {
                // Skip long item (not specified in HID specification)
                int size = data[1];
                data += 3 + size;
                len += 3 + size;
            } else {
                // Read short item
                unsigned bSize    = (data[0] >> 0) & 0x3;
                ItemType itemType = (ItemType)((data[0] >> 2) & 0x3);
                unsigned tag      = (data[0] >> 4) & 0xF;

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

                // Handle item
                switch (itemType) {
                    case ItemType::MainItem: handleMainItem((MainItemTag)tag, value); break;
                    case ItemType::GlobalItem: handleGlobalItem((GlobalItemTag)tag, bSize, value); break;
                    case ItemType::LocalItem: handleLocalItem((LocalItemTag)tag, value); break;
                    default: printf("Invalid item type: %u\n", (unsigned)itemType); break;
                }
            }
        }

        return collections;
    }
};

std::vector<std::shared_ptr<HIDCollection>> parseReportDescriptor(const void *descriptor, size_t descriptorLength) {
    HIDReportDescriptorParser parser;
    return parser.parseReportDescriptor(descriptor, descriptorLength);
}
