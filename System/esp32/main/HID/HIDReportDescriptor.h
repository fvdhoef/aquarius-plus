#pragma once

#include "Common.h"

class HIDReportDescriptor {
public:
    static HIDReportDescriptor *parseReportDescriptor(const void *descriptor, size_t descriptorLength);

    HIDReportDescriptor()
        : items(NULL) {
    }
    ~HIDReportDescriptor() {
        if (items) {
            delete items;
        }
    }

    struct ReportInfo {
        struct ReportID {
            ReportID(uint8_t id)
                : id(id), size(0), next(NULL) {
            }
            ~ReportID() {
                if (next) {
                    delete next;
                }
            }

            uint8_t  id;
            uint32_t size;

            ReportID *next;
        };

        ReportInfo()
            : reportIDs(NULL), noReportIDs(false) {
        }
        ~ReportInfo() {
            if (reportIDs) {
                delete reportIDs;
            }
        }

        ReportID *reportIDs;
        bool      noReportIDs;

        bool getReportID(uint8_t id, ReportID *&reportID) {
            if (id == 0) {
                noReportIDs = true;

                if (!reportIDs) {
                    reportIDs = new ReportID(0);
                }
                reportID = reportIDs;
                return true;
            }

            if (noReportIDs) {
                return false;
            }

            // Check if already used
            reportID               = reportIDs;
            ReportID *lastReportID = NULL;
            while (reportID) {
                if (reportID->id == id) {
                    return true;
                }

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

    struct HIDItem {
        enum Type { TCollection,
                    TInputField,
                    TOutputField,
                    TFeatureField };

        Type     type;
        uint16_t usagePage;
        HIDItem *next;
        HIDItem *parent;

        HIDItem(Type type, uint16_t usagePage)
            : type(type), usagePage(usagePage), next(NULL), parent(NULL) {
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
        HIDItem       *subItems;

        HIDCollection(CollectionType type, uint16_t usagePage, uint16_t usage)
            : HIDItem(TCollection, usagePage), type(type), usage(usage), subItems(NULL) {
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
        uint16_t usageMin;
        uint16_t usageMax;
        int32_t  minVal;
        int32_t  maxVal;
        uint32_t attributes;

        HIDField(
            Type type, uint8_t reportID, int16_t bitIdx, uint16_t bitSize, uint16_t arraySize,
            uint16_t usagePage, uint16_t usageMin, uint16_t usageMax,
            int32_t minVal, int32_t maxVal, uint32_t attributes)
            : HIDItem(type, usagePage), reportID(reportID),
              bitIdx(bitIdx), bitSize(bitSize), arraySize(arraySize),
              usageMin(usageMin), usageMax(usageMax), minVal(minVal), maxVal(maxVal),
              attributes(attributes) {
        }
    };

    ReportInfo inputReportInfo;
    ReportInfo outputReportInfo;
    ReportInfo featureReportInfo;

    void     dumpItems(const HIDItem *item = NULL, int depth = 0);
    HIDItem *items;
};
