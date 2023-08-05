#include "HIDReportHandlerMouse.h"

HIDReportHandlerMouse::HIDReportHandlerMouse()
    : HIDReportHandler(TMouse) {

    for (int i = 0; i < maxButtons; i++) {
        buttons[i] = -1;
    }
    xIdx      = -1;
    yIdx      = -1;
    wheelIdx  = -1;
    xSize     = -1;
    ySize     = -1;
    wheelSize = -1;
}

void HIDReportHandlerMouse::addInputField(const HIDReportDescriptor::HIDField &field) {
    if (field.usageMin == field.usageMax) {
        switch (field.usagePage) {
            case 1: {
                switch (field.usageMin) {
                    case 0x30:
                        xIdx  = field.bitIdx;
                        xSize = field.bitSize;
                        break;
                    case 0x31:
                        yIdx  = field.bitIdx;
                        ySize = field.bitSize;
                        break;
                    case 0x38:
                        wheelIdx  = field.bitIdx;
                        wheelSize = field.bitSize;
                        break;
                    default: break;
                }
                break;
            }

            case 9: {
                if (field.bitSize == 1 && field.usageMin >= 1 && field.usageMin <= maxButtons) {
                    buttons[field.usageMin - 1] = field.bitIdx;
                }
                break;
            }

            default: break;
        }
    }
}

void HIDReportHandlerMouse::inputReport(const uint8_t *buf, size_t length) {
    //	printf("HIDReportHandlerMouse::inputReport\n");
    //	HexDump(buf, length);

    if (xIdx >= 0) {
        printf("X:%5d ", (int)readBits(buf, length, xIdx, xSize, true));
    }
    if (yIdx >= 0) {
        printf("Y:%5d ", (int)readBits(buf, length, yIdx, ySize, true));
    }
    if (wheelIdx >= 0) {
        printf("W:%5d ", (int)readBits(buf, length, wheelIdx, wheelSize, true));
    }
    if (buttons[0] >= 0) {
        printf("L:%d ", (int)readBits(buf, length, buttons[0], 1, false));
    }
    if (buttons[2] >= 0) {
        printf("M:%d ", (int)readBits(buf, length, buttons[2], 1, false));
    }
    if (buttons[1] >= 0) {
        printf("R:%d ", (int)readBits(buf, length, buttons[1], 1, false));
    }
    printf("\n");
}
