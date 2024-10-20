#include "HIDReportHandlerMouse.h"
#include "FpgaCore.h"

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

HIDReportHandlerMouse::~HIDReportHandlerMouse() {
}

void HIDReportHandlerMouse::_addInputField(const HIDField &field) {
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

void HIDReportHandlerMouse::_inputReport(uint8_t reportId, const uint8_t *buf, size_t length) {
    // printf("HIDReportHandlerMouse::inputReport:");
    // for (unsigned i = 0; i < length; i++) {
    //     printf(" %02X", ((const uint8_t *)buf)[i]);
    // }
    // printf("   ");

    //	HexDump(buf, length);

    int     dx         = 0;
    int     dy         = 0;
    int     dWheel     = 0;
    uint8_t buttonMask = 0;

    if (xIdx >= 0)
        dx = (int)readBits(buf, length, xIdx, xSize, true);
    if (yIdx >= 0)
        dy = (int)readBits(buf, length, yIdx, ySize, true);
    if (wheelIdx >= 0)
        dWheel = (int)readBits(buf, length, wheelIdx, wheelSize, true);

    for (int i = 0; i < 3; i++) {
        if (buttons[i] >= 0 && (int)readBits(buf, length, buttons[i], 1, false))
            buttonMask |= (1 << i);
    }

    auto core = getFpgaCore();
    if (core) {
        core->mouseReport(dx, dy, buttonMask, dWheel);
    }
}
