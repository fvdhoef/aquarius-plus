#include "HIDReportHandlerGamepad.h"
#include "FPGA.h"
#include <math.h>
#include "AqUartProtocol.h"

static const char *TAG = "HIDReportHandlerGamepad";

#define AQP_EN
// #define PRINT_INPUT

HIDReportHandlerGamepad::HIDReportHandlerGamepad()
    : HIDReportHandler(TGamepad) {

    for (int i = 0; i < 16; i++) {
        idxBtns[i] = -1;
    }

#ifdef AQP_EN
    auto              &aqp = AqUartProtocol::instance();
    RecursiveMutexLock lk(aqp.mutexGameCtrlData);
    aqp.gameCtrlReset(true);
#endif
}

HIDReportHandlerGamepad::~HIDReportHandlerGamepad() {
#ifdef AQP_EN
    auto              &aqp = AqUartProtocol::instance();
    RecursiveMutexLock lk(aqp.mutexGameCtrlData);
    aqp.gameCtrlReset(false);
#endif
}

void HIDReportHandlerGamepad::_addInputField(const HIDReportDescriptor::HIDField &field) {
    if (field.arraySize == 1) {
        // printf("usage: %02X:%02X\n", field.usagePage, field.usageMin);

        switch (field.usagePage) {
            case 1: {
                switch (field.usageMin) {
                    case 0x30: // X (left stick X)
                        idxLSX  = field.bitIdx;
                        sizeLSX = field.bitSize;
                        minLSX  = field.logicalMin;
                        maxLSX  = field.logicalMax;
                        break;
                    case 0x31: // Y (left stick Y)
                        idxLSY  = field.bitIdx;
                        sizeLSY = field.bitSize;
                        minLSY  = field.logicalMin;
                        maxLSY  = field.logicalMax;
                        break;
                    case 0x32: // Z
                        idxRSX  = field.bitIdx;
                        sizeRSX = field.bitSize;
                        minRSX  = field.logicalMin;
                        maxRSX  = field.logicalMax;
                        break;
                    case 0x35: // Rz
                        idxRSY  = field.bitIdx;
                        sizeRSY = field.bitSize;
                        minRSY  = field.logicalMin;
                        maxRSY  = field.logicalMax;
                        break;
                    case 0x39: // Hat switch (D-pad)
                        idxHat  = field.bitIdx;
                        sizeHat = field.bitSize;
                        minHat  = field.logicalMin;
                        maxHat  = field.logicalMax;
                        break;
                    default: break;
                }
                break;
            }

            case 2: {
                switch (field.usageMin) {
                    case 0xC4: // Accelerator
                        idxRT  = field.bitIdx;
                        sizeRT = field.bitSize;
                        minRT  = field.logicalMin;
                        maxRT  = field.logicalMax;
                        break;
                    case 0xC5: // Brake
                        idxLT  = field.bitIdx;
                        sizeLT = field.bitSize;
                        minLT  = field.logicalMin;
                        maxLT  = field.logicalMax;
                        break;
                }
                break;
            }

            case 9: {
                if (field.bitSize == 1) {
                    switch (field.usageMin) {
                        case 1: idxBtns[GCB_A_IDX] = field.bitIdx; break;
                        case 2: idxBtns[GCB_B_IDX] = field.bitIdx; break;
                        case 4: idxBtns[GCB_X_IDX] = field.bitIdx; break;
                        case 5: idxBtns[GCB_Y_IDX] = field.bitIdx; break;
                        case 7: idxBtns[GCB_LB_IDX] = field.bitIdx; break;
                        case 8: idxBtns[GCB_RB_IDX] = field.bitIdx; break;
                        case 11: idxBtns[GCB_VIEW_IDX] = field.bitIdx; break;
                        case 12: idxBtns[GCB_MENU_IDX] = field.bitIdx; break;
                        case 13: idxBtns[GCB_GUIDE_IDX] = field.bitIdx; break;
                        case 14: idxBtns[GCB_LS_IDX] = field.bitIdx; break;
                        case 15: idxBtns[GCB_RS_IDX] = field.bitIdx; break;
                        default: break;
                    }
                }
                break;
            }

            case 0x0C: {
                if (field.usageMin == 0xB2) { // Phone key 2
                    idxBtns[GCB_SHARE_IDX] = field.bitIdx;
                }
                break;
            }
            default: break;
        }
    }
}

static inline float remap(float val, float in_min, float in_max, float out_min, float out_max, bool clip = true) {
    float result = ((val - in_min) / (in_max - in_min)) * (out_max - out_min) + out_min;
    if (clip)
        result = std::max(out_min, std::min(result, out_max));
    return result;
}

int8_t HIDReportHandlerGamepad::getInt8(int idx, int size, int minVal, int maxVal, const uint8_t *buf, size_t length) {
    if (idx < 0)
        return 0;

    float val = remap(readBits(buf, length, idx, size, false), minVal, maxVal, -1, 1, false);

    // Apply dead zone
    const float deadZone = 0.1f;
    if (val <= -deadZone) {
        return (int8_t)remap(val, -1, -deadZone, -127, 0);
    } else if (val >= deadZone) {
        return (int8_t)remap(val, deadZone, 1, 0, 127);
    }
    return 0;
}

uint8_t HIDReportHandlerGamepad::getUInt8(int idx, int size, int minVal, int maxVal, const uint8_t *buf, size_t length) {
    if (idx < 0)
        return 0;

    float val = remap(readBits(buf, length, idx, size, false), minVal, maxVal, 0, 1, false);

    // Apply dead zone
    const float deadZone = 0.05f;
    if (val >= deadZone) {
        return (int8_t)remap(val, deadZone, 1, 0, 255);
    }
    return 0;
}

void HIDReportHandlerGamepad::_inputReport(uint8_t reportId, const uint8_t *buf, size_t length) {
    int8_t   ctrlLX      = getInt8(idxLSX, sizeLSX, minLSX, maxLSX, buf, length);
    int8_t   ctrlLY      = getInt8(idxLSY, sizeLSY, minLSY, maxLSY, buf, length);
    int8_t   ctrlRX      = getInt8(idxRSX, sizeRSX, minRSX, maxRSX, buf, length);
    int8_t   ctrlRY      = getInt8(idxRSY, sizeRSY, minRSY, maxRSY, buf, length);
    uint8_t  ctrlLT      = getUInt8(idxLT, sizeLT, minLT, maxLT, buf, length);
    uint8_t  ctrlRT      = getUInt8(idxRT, sizeRT, minRT, maxRT, buf, length);
    uint16_t ctrlButtons = 0;

    for (int i = 0; i < 16; i++) {
        // printf("%d:%d ", idxBtns[i]);

        if (idxBtns[i] >= 0 && readBits(buf, length, idxBtns[i], 1, 0)) {
            ctrlButtons |= (1 << i);
        }
    }
    // printf("\n");

    if (idxHat >= 0) {
        int hat = readBits(buf, length, idxHat, sizeHat, false);
        switch (hat - minHat) {
            case 0: ctrlButtons |= GCB_DPAD_UP; break;                    // UP
            case 1: ctrlButtons |= GCB_DPAD_UP | GCB_DPAD_RIGHT; break;   // UP+RIGHT
            case 2: ctrlButtons |= GCB_DPAD_RIGHT; break;                 // RIGHT
            case 3: ctrlButtons |= GCB_DPAD_DOWN | GCB_DPAD_RIGHT; break; // DOWN+RIGHT
            case 4: ctrlButtons |= GCB_DPAD_DOWN; break;                  // DOWN
            case 5: ctrlButtons |= GCB_DPAD_DOWN | GCB_DPAD_LEFT; break;  // DOWN+LEFT
            case 6: ctrlButtons |= GCB_DPAD_LEFT; break;                  // LEFT
            case 7: ctrlButtons |= GCB_DPAD_UP | GCB_DPAD_LEFT; break;    // UP+LEFT
            default: break;
        }
    }

#ifdef PRINT_INPUT
    printf(
        "- LX:%4d LY:%4d RX:%4d RY:%4d LT:%3u RT:%3u Buttons:",
        ctrlLX,
        ctrlLY,
        ctrlRX,
        ctrlRY,
        ctrlLT,
        ctrlRT);

    if (ctrlButtons & GCB_A)
        printf("[A]");
    if (ctrlButtons & GCB_B)
        printf("[B]");
    if (ctrlButtons & GCB_X)
        printf("[X]");
    if (ctrlButtons & GCB_Y)
        printf("[Y]");
    if (ctrlButtons & GCB_VIEW)
        printf("[VIEW]");
    if (ctrlButtons & GCB_GUIDE)
        printf("[GUIDE]");
    if (ctrlButtons & GCB_MENU)
        printf("[MENU]");
    if (ctrlButtons & GCB_LS)
        printf("[LS]");
    if (ctrlButtons & GCB_RS)
        printf("[RS]");
    if (ctrlButtons & GCB_LB)
        printf("[LB]");
    if (ctrlButtons & GCB_RB)
        printf("[RB]");
    if (ctrlButtons & GCB_DPAD_UP)
        printf("[UP]");
    if (ctrlButtons & GCB_DPAD_DOWN)
        printf("[DOWN]");
    if (ctrlButtons & GCB_DPAD_LEFT)
        printf("[LEFT]");
    if (ctrlButtons & GCB_DPAD_RIGHT)
        printf("[RIGHT]");
    if (ctrlButtons & GCB_SHARE)
        printf("[SHARE]");

    printf("\n");
#endif

#ifdef AQP_EN
    {
        auto              &aqp = AqUartProtocol::instance();
        RecursiveMutexLock lk(aqp.mutexGameCtrlData);

        aqp.gameCtrlPresent = true;
        aqp.gameCtrlLX      = ctrlLX;
        aqp.gameCtrlLY      = ctrlLY;
        aqp.gameCtrlRX      = ctrlRX;
        aqp.gameCtrlRY      = ctrlRY;
        aqp.gameCtrlLT      = ctrlLT;
        aqp.gameCtrlRT      = ctrlRT;
        aqp.gameCtrlButtons = ctrlButtons;
        aqp.gameCtrlUpdated();
    }
#endif
}
