#include "Keyboard.h"
#include "FPGA.h"
#include "FpgaCore.h"
#include "DisplayOverlay/DisplayOverlay.h"
#include "AqKeyboardDefs.h"

static const char *TAG = "Keyboard";

class KeyboardInt : public Keyboard {
public:
    uint8_t           prevKeys[14] = {0};
    uint8_t           modifiers    = 0;
    QueueHandle_t     keyQueue;
    uint8_t           repeat       = 0;
    unsigned          pressCounter = 0;
    uint8_t           keyMode      = 3;
    SemaphoreHandle_t mutex;
    KeyLayout         curLayout = KeyLayout::US;

    KeyboardInt() {
        mutex    = xSemaphoreCreateRecursiveMutex();
        keyQueue = xQueueCreate(128, 1);

        auto timer = xTimerCreate("keyRepeat", pdMS_TO_TICKS(16), pdTRUE, this, _keyRepeatTimer);
        xTimerStart(timer, 0);
    }

    void setKeyMode(uint8_t mode) override {
        RecursiveMutexLock lock(mutex);
        keyMode = mode;
    }

    uint8_t getKeyMode() override {
        RecursiveMutexLock lock(mutex);
        return keyMode;
    }

    static void _keyRepeatTimer(TimerHandle_t xTimer) { static_cast<KeyboardInt *>(pvTimerGetTimerID(xTimer))->keyRepeatTimer(); }
    void        keyRepeatTimer() {
        RecursiveMutexLock lock(mutex);

        if (repeat == 0) {
            pressCounter = 0;
            return;
        }

        pressCounter++;
        if (pressCounter > 30 && pressCounter % 3 == 0) {
            xQueueSend(keyQueue, &repeat, 0);

            if ((keyMode & 4) != 0 && !getDisplayOverlay()->isVisible()) {
                auto core = getFpgaCore();
                if (core)
                    core->keyChar(repeat, true);
            }
        }
    }

    void updateKeys(uint8_t keys[14]) override {
        RecursiveMutexLock lock(mutex);

        for (int j = 0; j < 14; j++) {
            uint8_t released = prevKeys[j] & ~keys[j];

            for (int i = 0; i < 8; i++)
                if (released & (1 << i))
                    processInternalKeyboardScancode(j * 8 + i, false);
        }

        for (int j = 0; j < 14; j++) {
            uint8_t pressed = ~prevKeys[j] & keys[j];

            for (int i = 0; i < 8; i++)
                if (pressed & (1 << i))
                    processInternalKeyboardScancode(j * 8 + i, true);
        }

        for (int i = 0; i < 14; i++)
            prevKeys[i] = keys[i];
    }

    void processInternalKeyboardScancode(unsigned scanCode, bool keyDown) {
        // Remap modifier keys to USB HUT location
        if (scanCode >= 104)
            scanCode = (scanCode - 104) + SCANCODE_LCTRL;

        handleScancode(scanCode, keyDown);
    }

    void handleScancode(unsigned scanCode, bool keyDown) override {
        RecursiveMutexLock lock(mutex);
        repeat       = 0;
        pressCounter = 0;

        // Keyboard layout handling
        processScancode(scanCode, keyDown);

        auto core = getFpgaCore();
        if (core) {
            if (!getDisplayOverlay()->isVisible() || !keyDown)
                core->keyScancode(modifiers, scanCode, keyDown);
        }
    }

    uint8_t layoutUS(unsigned scanCode) {
        uint8_t ch = 0;
        if (scanCode >= SCANCODE_A && scanCode <= SCANCODE_SLASH) {
            static const uint8_t lut1[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\r', 3, '\b', '\t', ' ', '-', '=', '[', ']', '\\', '\\', ';', '\'', '`', ',', '.', '/'};
            static const uint8_t lut2[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '\r', 3, '\b', 0x8C, ' ', '_', '+', '{', '}', '|', '|', ':', '"', '~', '<', '>', '?'};

            ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut2[scanCode - SCANCODE_A] : lut1[scanCode - SCANCODE_A];

        } else if (scanCode == SCANCODE_NONUSBACKSLASH) {
            ch = (modifiers & (ModLShift | ModRShift)) ? '|' : '\\';
        }
        return ch;
    }

    void processScancode(unsigned scanCode, bool keyDown) {
        // ESP_LOGI(TAG, "Key %3d %s", scanCode, keyDown ? "pressed" : "released");

        // Keep track of pressed modifier keys
        if (scanCode == SCANCODE_LCTRL)
            modifiers = (modifiers & ~ModLCtrl) | (keyDown ? ModLCtrl : 0);
        if (scanCode == SCANCODE_LSHIFT)
            modifiers = (modifiers & ~ModLShift) | (keyDown ? ModLShift : 0);
        if (scanCode == SCANCODE_LALT)
            modifiers = (modifiers & ~ModLAlt) | (keyDown ? ModLAlt : 0);
        if (scanCode == SCANCODE_LGUI)
            modifiers = (modifiers & ~ModLGui) | (keyDown ? ModLGui : 0);
        if (scanCode == SCANCODE_RCTRL)
            modifiers = (modifiers & ~ModRCtrl) | (keyDown ? ModRCtrl : 0);
        if (scanCode == SCANCODE_RSHIFT)
            modifiers = (modifiers & ~ModRShift) | (keyDown ? ModRShift : 0);
        if (scanCode == SCANCODE_RALT)
            modifiers = (modifiers & ~ModRAlt) | (keyDown ? ModRAlt : 0);
        if (scanCode == SCANCODE_RGUI)
            modifiers = (modifiers & ~ModRGui) | (keyDown ? ModRGui : 0);

        if (keyDown) {
            uint8_t ch = layoutUS(scanCode);
            // printf("%d\n", scanCode);

            if (scanCode == SCANCODE_ESP_MENU || (scanCode == SCANCODE_TAB && (modifiers & (ModLGui | ModRGui))))
                ch = 0xFF;

            if (
                ch == 0 &&
                scanCode >= SCANCODE_F1 && scanCode <= SCANCODE_KP_PERIOD &&
                scanCode != SCANCODE_SCROLLLOCK &&
                scanCode != SCANCODE_NUMLOCK) {
                static const uint8_t lut[] = {
                    0x80, // SCANCODE_F1
                    0x81, // SCANCODE_F2
                    0x82, // SCANCODE_F3
                    0x83, // SCANCODE_F4
                    0x84, // SCANCODE_F5
                    0x85, // SCANCODE_F6
                    0x86, // SCANCODE_F7
                    0x87, // SCANCODE_F8
                    0x90, // SCANCODE_F9
                    0x91, // SCANCODE_F10
                    0x92, // SCANCODE_F11
                    0x93, // SCANCODE_F12
                    0x88, // SCANCODE_PRINTSCREEN
                    0,    // SCANCODE_SCROLLLOCK
                    0x89, // SCANCODE_PAUSE
                    0x9D, // SCANCODE_INSERT
                    0x9B, // SCANCODE_HOME
                    0x8A, // SCANCODE_PAGEUP
                    0x7F, // SCANCODE_DELETE
                    0x9A, // SCANCODE_END
                    0x8B, // SCANCODE_PAGEDOWN
                    0x8E, // SCANCODE_RIGHT
                    0x9E, // SCANCODE_LEFT
                    0x9F, // SCANCODE_DOWN
                    0x8F, // SCANCODE_UP
                    0,    // SCANCODE_NUMLOCK
                    '/',  // SCANCODE_KP_DIVIDE
                    '*',  // SCANCODE_KP_MULTIPLY
                    '-',  // SCANCODE_KP_MINUS
                    '+',  // SCANCODE_KP_PLUS
                    '\r', // SCANCODE_KP_ENTER
                    '1',  // SCANCODE_KP_1
                    '2',  // SCANCODE_KP_2
                    '3',  // SCANCODE_KP_3
                    '4',  // SCANCODE_KP_4
                    '5',  // SCANCODE_KP_5
                    '6',  // SCANCODE_KP_6
                    '7',  // SCANCODE_KP_7
                    '8',  // SCANCODE_KP_8
                    '9',  // SCANCODE_KP_9
                    '0',  // SCANCODE_KP_0
                    '.',  // SCANCODE_KP_PERIOD
                };
                ch = lut[scanCode - SCANCODE_F1];
            }

            if ((modifiers & (ModLCtrl | ModRCtrl)) != 0) {
                if (ch == '@') {
                    ch = 0x80;
                } else if (ch >= 'a' && ch <= 'z') {
                    ch = ch - 'a' + 1;
                } else if (ch >= 'A' && ch <= '_') {
                    ch = ch - 'A' + 1;
                }
            }

            if (ch > 0) {
                if (ch != 0xFF) {
                    repeat = ch;

                    if (!getDisplayOverlay()->isVisible()) {
                        auto core = getFpgaCore();
                        if (core)
                            core->keyChar(ch, false);
                    }
                }
                xQueueSend(keyQueue, &ch, 0);
            }

            // printf("%c\n", ch);
        }
    }

    int getKey(TickType_t ticksToWait) override {
        uint8_t result;
        if (!xQueueReceive(keyQueue, &result, ticksToWait))
            return -1;
        return result;
    }

    void setKeyLayout(KeyLayout layout) override {
        curLayout = layout;
    }

    KeyLayout getKeyLayout() override {
        return curLayout;
    }

    std::string getKeyLayoutName(KeyLayout layout) override {
        switch (layout) {
            default: return "Unknown";
            case KeyLayout::US: return "US";
            case KeyLayout::UK: return "UK";
            case KeyLayout::FR: return "FR/BE (AZERTY)";
            case KeyLayout::DE: return "DE (QWERTZ)";
        }
    }
};

Keyboard *getKeyboard() {
    static KeyboardInt obj;
    return &obj;
}
