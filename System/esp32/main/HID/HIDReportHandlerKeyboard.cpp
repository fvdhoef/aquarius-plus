#include "HIDReportHandlerKeyboard.h"
#include "Keyboard.h"

static const char *TAG = "HIDReportHandlerKeyboard";

HIDReportHandlerKeyboard::HIDReportHandlerKeyboard()
    : HIDReportHandler(TKeyboard) {

    for (int i = 0; i < maxButtons; i++) {
        buttons[i] = -1;
    }
    for (unsigned i = 0; i < maxKeyData; i++) {
        prevKeyData[i] = 0;
    }
}

HIDReportHandlerKeyboard::~HIDReportHandlerKeyboard() {
}

void HIDReportHandlerKeyboard::_addInputField(const HIDReportDescriptor::HIDField &field) {
    if (field.attributes & (1 << 1)) {
        if (field.bitSize == 1 && field.usageMin >= 0xE0 && field.usageMin <= 0xE7) {
            buttons[field.usageMin - 0xE0] = field.bitIdx;
        }
    } else {
        if (field.arraySize <= maxKeyData) {
            keyArrayIdx      = field.bitIdx;
            keyArrayItemSize = field.bitSize;
            keyArrayItems    = field.arraySize;
        }
    }
}

void HIDReportHandlerKeyboard::_addOutputField(const HIDReportDescriptor::HIDField &field) {
    if (field.bitSize == 1 && field.arraySize == 1 && field.usagePage == 8) {
        switch (field.usageMin) {
            case 1: ledNumLockIdx = field.bitIdx; break;
            case 2: ledCapsLockIdx = field.bitIdx; break;
            case 3: ledScrollLockIdx = field.bitIdx; break;
            default: break;
        }
    }
}

void HIDReportHandlerKeyboard::_inputReport(uint8_t reportId, const uint8_t *buf, size_t length) {
    //	printf("HIDReportHandlerKeyboard::inputReport\n");
    //	HexDump(buf, length);
    // auto &aqkb = AqKeyboard::instance();

    if (keyArrayIdx < 0) {
        return;
    }

    uint8_t modifiers = 0;
    if (buttons[LCtrl] >= 0) {
        modifiers |= readBits(buf, length, buttons[LCtrl], 1, false) << 0;
    }
    if (buttons[LShift] >= 0) {
        modifiers |= readBits(buf, length, buttons[LShift], 1, false) << 1;
    }
    if (buttons[LAlt] >= 0) {
        modifiers |= readBits(buf, length, buttons[LAlt], 1, false) << 2;
    }
    if (buttons[LGui] >= 0) {
        modifiers |= readBits(buf, length, buttons[LGui], 1, false) << 3;
    }
    if (buttons[RCtrl] >= 0) {
        modifiers |= readBits(buf, length, buttons[RCtrl], 1, false) << 4;
    }
    if (buttons[RShift] >= 0) {
        modifiers |= readBits(buf, length, buttons[RShift], 1, false) << 5;
    }
    if (buttons[RAlt] >= 0) {
        modifiers |= readBits(buf, length, buttons[RAlt], 1, false) << 6;
    }
    if (buttons[RGui] >= 0) {
        modifiers |= readBits(buf, length, buttons[RGui], 1, false) << 7;
    }

    uint8_t releasedModifiers = ~modifiers & prevModifiers;
    uint8_t pressedModifiers  = modifiers & ~prevModifiers;
    prevModifiers             = modifiers;

    uint8_t keyData[maxKeyData];
    for (int i = 0; i < keyArrayItems; i++) {
        keyData[i] = readBits(buf, length, keyArrayIdx + i * keyArrayItemSize, keyArrayItemSize, false);
    }

    // Don't process during rollover
    if (keyData[0] == 1) {
        return;
    }

    // Process modifier key changes
    for (int i = 0; i < 8; i++) {
        if (releasedModifiers & (1 << i)) {
            getKeyboard()->handleScancode(0xE0 + i, false);
        }
        if (pressedModifiers & (1 << i)) {
            getKeyboard()->handleScancode(0xE0 + i, true);
        }
    }

    uint8_t prev, cur;

    // Check for key releases
    for (int j = 0; j < keyArrayItems; j++) {
        if ((prev = prevKeyData[j]) == 0)
            break;

        // Check if this key is in current report
        bool keyReleased = true;
        for (int i = 0; i < keyArrayItems; i++) {
            if ((cur = keyData[i]) == 0)
                break;

            if (prev == cur) {
                keyReleased = false;
                break;
            }
        }

        if (keyReleased) {
            getKeyboard()->handleScancode(prev, false);
        }
    }

    // Check for key presses
    for (int j = 0; j < keyArrayItems; j++) {
        if ((cur = keyData[j]) == 0)
            break;

        // Check if this key is in previous report
        bool keyPressed = true;
        for (int i = 0; i < keyArrayItems; i++) {
            if ((prev = prevKeyData[i]) == 0)
                break;

            if (prev == cur) {
                keyPressed = false;
                break;
            }
        }

        if (keyPressed) {
            getKeyboard()->handleScancode(cur, true);
        }
    }

    // Copy current key data to prev key data
    for (int i = 0; i < keyArrayItems; i++) {
        prevKeyData[i] = keyData[i];
    }
    // aqkb.updateMatrix();
}

uint8_t HIDReportHandlerKeyboard::outputReport(uint8_t leds) const {
    uint8_t result = 0;
    if (ledNumLockIdx >= 0 && (leds & NUM_LOCK) != 0)
        result |= (1 << ledNumLockIdx);
    if (ledCapsLockIdx >= 0 && (leds & CAPS_LOCK) != 0)
        result |= (1 << ledCapsLockIdx);
    if (ledScrollLockIdx >= 0 && (leds & SCROLL_LOCK) != 0)
        result |= (1 << ledScrollLockIdx);
    return result;
}
