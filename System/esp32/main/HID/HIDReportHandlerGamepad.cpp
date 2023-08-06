#include "HIDReportHandlerGamepad.h"
#include "FPGA.h"
#include <math.h>

static const char *TAG = "HIDReportHandlerGamepad";

// XBOX button order:
// 1:A
// 2:B
// 3:-
// 4:X
// 5:Y
// 6:-
// 7:LB
// 8:RB
// 9:-
// 10:-
// 11:view
// 12:menu
// 13:xbox
// 14:L3
// 15:R3

HIDReportHandlerGamepad::HIDReportHandlerGamepad()
    : HIDReportHandler(TGamepad) {
}

HIDReportHandlerGamepad::~HIDReportHandlerGamepad() {
}

void HIDReportHandlerGamepad::addInputField(const HIDReportDescriptor::HIDField &field) {
    if (field.arraySize == 1) {
        switch (field.usagePage) {
            case 1: {
                switch (field.usageMin) {
                    case 0x30:
                        leftX_Idx  = field.bitIdx;
                        leftX_Size = field.bitSize;
                        break;
                    case 0x31:
                        leftY_Idx  = field.bitIdx;
                        leftY_Size = field.bitSize;
                        break;
                    case 0x39:
                        hat_Idx  = field.bitIdx;
                        hat_Size = field.bitSize;
                        break;
                    default: break;
                }
                break;
            }

            case 9: {
                if (field.bitSize == 1) {
                    switch (field.usageMin) {
                        case 1: btnA_Idx = field.bitIdx; break;
                        case 2: btnB_Idx = field.bitIdx; break;
                        case 4: btnX_Idx = field.bitIdx; break;
                        case 5: btnY_Idx = field.bitIdx; break;
                        case 7: btnLB_Idx = field.bitIdx; break;
                        case 8: btnRB_Idx = field.bitIdx; break;
                    }
                }
                break;
            }
            default: break;
        }
    }
}

void HIDReportHandlerGamepad::inputReport(const uint8_t *buf, size_t length) {
    uint8_t handctrl = 0xFF;

    if (btnA_Idx >= 0 && readBits(buf, length, btnA_Idx, 1, 0))
        handctrl &= ~(1 << 6);
    if (btnB_Idx >= 0 && readBits(buf, length, btnB_Idx, 1, 0))
        handctrl &= ~((1 << 7) | (1 << 2));
    if (btnX_Idx >= 0 && readBits(buf, length, btnX_Idx, 1, 0))
        handctrl &= ~((1 << 7) | (1 << 5));
    if (btnY_Idx >= 0 && readBits(buf, length, btnY_Idx, 1, 0))
        handctrl &= ~((1 << 5));
    if (btnLB_Idx >= 0 && readBits(buf, length, btnLB_Idx, 1, 0))
        handctrl &= ~((1 << 7) | (1 << 1));
    if (btnRB_Idx >= 0 && readBits(buf, length, btnRB_Idx, 1, 0))
        handctrl &= ~((1 << 7) | (1 << 0));

    unsigned p = 0;
    if (hat_Idx >= 0) {
        int hat = readBits(buf, length, hat_Idx, hat_Size, false);
        switch (hat) {
            case 1: p = 13; break; // UP
            case 2: p = 15; break; // UP+RIGHT
            case 3: p = 1; break;  // RIGHT
            case 4: p = 3; break;  // DOWN+RIGHT
            case 5: p = 5; break;  // DOWN
            case 6: p = 7; break;  // DOWN+LEFT
            case 7: p = 9; break;  // LEFT
            case 8: p = 11; break; // UP+LEFT
            default: break;
        }
    }

    {
        float x = 0;
        float y = 0;

        if (leftX_Idx >= 0) {
            int lx = readBits(buf, length, leftX_Idx, leftX_Size, false);
            x      = (lx - 0x8000) / 32768.0f;
        }
        if (leftY_Idx >= 0) {
            int lx = readBits(buf, length, leftY_Idx, leftY_Size, false);
            y      = (lx - 0x8000) / 32768.0f;
        }

        float len   = sqrtf(x * x + y * y);
        float angle = 0;
        if (len > 0.4f) {
            angle = atan2f(y, x) / M_PI * 180.0f + 180.0f;
            p     = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
        }
    }

    switch (p) {
        case 1: handctrl &= ~((1 << 1)); break;
        case 2: handctrl &= ~((1 << 4) | (1 << 1)); break;
        case 3: handctrl &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case 4: handctrl &= ~((1 << 1) | (1 << 0)); break;
        case 5: handctrl &= ~((1 << 0)); break;
        case 6: handctrl &= ~((1 << 4) | (1 << 0)); break;
        case 7: handctrl &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        case 8: handctrl &= ~((1 << 3) | (1 << 0)); break;
        case 9: handctrl &= ~((1 << 3)); break;
        case 10: handctrl &= ~((1 << 4) | (1 << 3)); break;
        case 11: handctrl &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case 12: handctrl &= ~((1 << 3) | (1 << 2)); break;
        case 13: handctrl &= ~((1 << 2)); break;
        case 14: handctrl &= ~((1 << 4) | (1 << 2)); break;
        case 15: handctrl &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case 16: handctrl &= ~((1 << 2) | (1 << 1)); break;
        default: break;
    }

    if (oldHandctrl != handctrl) {
        FPGA::instance().aqpUpdateHandCtrl(handctrl, 0xFF, pdMS_TO_TICKS(100));
        oldHandctrl = handctrl;
    }
}
