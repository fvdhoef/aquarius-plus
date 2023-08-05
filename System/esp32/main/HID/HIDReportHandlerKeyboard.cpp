#include "HIDReportHandlerKeyboard.h"

static void reverseInsertionSort(uint8_t *buf, size_t numElements) {
    for (size_t i = 1; i < numElements; i++) {
        uint8_t tmp = buf[i];

        size_t j;
        for (j = i; j >= 1 && tmp > buf[j - 1]; j--) {
            buf[j] = buf[j - 1];
        }
        buf[j] = tmp;
    }
}

HIDReportHandlerKeyboard::HIDReportHandlerKeyboard()
    : HIDReportHandler(TKeyboard) {

    for (int i = 0; i < maxButtons; i++) {
        buttons[i] = -1;
    }

    keyArrayIdx      = -1;
    keyArrayItemSize = -1;
    keyArrayItems    = -1;

    for (unsigned i = 0; i < maxKeyData; i++) {
        prevKeyData[i] = 0;
    }
    prevModifiers = 0;
}

HIDReportHandlerKeyboard::~HIDReportHandlerKeyboard() {
}

void HIDReportHandlerKeyboard::addInputField(const HIDReportDescriptor::HIDField &field) {
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

void HIDReportHandlerKeyboard::inputReport(const uint8_t *buf, size_t length) {
    //	printf("HIDReportHandlerKeyboard::inputReport\n");
    //	HexDump(buf, length);

    if (keyArrayIdx < 0) {
        return;
    }

    uint8_t pressed[maxKeyData + 8], released[maxKeyData + 8];
    int     numPressed = 0, numReleased = 0;

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

    uint8_t pressedModifiers  = modifiers & ~prevModifiers;
    uint8_t releasedModifiers = ~modifiers & prevModifiers;
    prevModifiers             = modifiers;

    for (int i = 0; i < 8; i++) {
        if (pressedModifiers & (1 << i)) {
            pressed[numPressed++] = 0xE0 + i;
        }
        if (releasedModifiers & (1 << i)) {
            released[numReleased++] = 0xE0 + i;
        }
    }

    uint8_t keyData[maxKeyData];
    for (int i = 0; i < keyArrayItems; i++) {
        keyData[i] = readBits(buf, length, keyArrayIdx + i * keyArrayItemSize, keyArrayItemSize, false);
    }

    reverseInsertionSort(keyData, keyArrayItems);

    // Roll over?
    if (keyData[0] != 1) {
        // Determine pressed / released / unchanged keys
        {
            int j = 0;
            int k = 0;

            while (j < keyArrayItems && k < keyArrayItems) {
                uint8_t key     = keyData[k];
                uint8_t prevKey = prevKeyData[j];
                if (key == 0 || prevKey == 0) {
                    break;
                }

                if (key < prevKey) {
                    released[numReleased++] = prevKey;
                    j++;
                } else if (key > prevKey) {
                    pressed[numPressed++] = key;
                    k++;
                } else { // key == prevKey
                    k++;
                    j++;
                }
            }

            while (j < keyArrayItems) {
                uint8_t prevKey = prevKeyData[j++];
                if (prevKey == 0) {
                    break;
                }
                released[numReleased++] = prevKey;
            }
            while (k < keyArrayItems) {
                uint8_t key = keyData[k++];
                if (key == 0) {
                    break;
                }
                pressed[numPressed++] = key;
            }
        }

        // Copy current key data to prev key data
        for (int i = 0; i < keyArrayItems; i++) {
            prevKeyData[i] = keyData[i];
        }
    }

    if (numPressed || numReleased) {
        if (numPressed) {
            for (int i = 0; i < numPressed; i++) {
                printf("P%02X ", pressed[i]);
            }
        }
        if (numReleased) {
            for (int i = 0; i < numReleased; i++) {
                printf("R%02X ", released[i]);
            }
        }
        printf("\n");
    }
}
