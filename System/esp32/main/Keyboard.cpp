#include "Keyboard.h"
#include "FPGA.h"
#include "FpgaCore.h"
#include "DisplayOverlay/DisplayOverlay.h"
#include "AqKeyboardDefs.h"
#include "USBHost.h"

static const char *TAG = "Keyboard";

enum {
    LedNumLock    = (1 << 0),
    LedCapsLock   = (1 << 1),
    LedScrollLock = (1 << 2),
};

struct ComposeCombo {
    const char *combo;
    uint8_t     result;
};

// From https://math.dartmouth.edu/~sarunas/Linux_Compose_Key_Sequences.html
static ComposeCombo composeCombos[] = {
    {"\xA8 ", 0xA8}, // Diacritic
    {"\xB4 ", 0xB4}, // Acute accent
    {"' ", '\''},
    {"\" ", '"'},
    {"` ", '`'},
    {"~ ", '~'},
    {"^ ", '^'},

    {"=C", 0x80},
    {"=c", 0x80},
    {"=E", 0x80},
    {":)", 0x81},
    {"<3", 0x84},

    {"  ", 0xA0},
    {"!!", 0xA1},
    {"|c", 0xA2},
    {"/c", 0xA2},
    {"-L", 0xA3},
    {"ox", 0xA4},
    {"Y=", 0xA5},
    {"!^", 0xA6},
    {"os", 0xA7},
    // how to type 0xA8?
    {"oc", 0xA9},
    {"oC", 0xA9},
    {"Oc", 0xA9},
    {"OC", 0xA9},
    {"aa", 0xAA},
    {"<<", 0xAB},
    {"-,", 0xAC},
    {"--", 0xAD},
    {"or", 0xAE},
    {"oR", 0xAE},
    {"Or", 0xAE},
    {"OR", 0xAE},
    {"^-", 0xAF},

    {"oo", 0xB0},
    {"+-", 0xB1},
    {"^2", 0xB2},
    {"^3", 0xB3},
    {"''", 0xB4},
    {"mu", 0xB5},
    {"p!", 0xB6},
    {"P!", 0xB6},
    {"PP", 0xB6},
    {"..", 0xB7},
    {", ", 0xB8},
    {"^1", 0xB9},
    {"^0", 0xBA},
    {">>", 0xBB},
    {"14", 0xBC},
    {"12", 0xBD},
    {"34", 0xBE},
    {"??", 0xBF},

    {"`A", 0xC0},
    {"'A", 0xC1},
    {"^A", 0xC2},
    {"~A", 0xC3},
    {"\"A", 0xC4},
    {"oA", 0xC5},
    {"AE", 0xC6},
    {",C", 0xC7},
    {"`E", 0xC8},
    {"'E", 0xC9},
    {"^E", 0xCA},
    {"\"E", 0xCB},
    {"`I", 0xCC},
    {"'I", 0xCD},
    {"^I", 0xCE},
    {"\"I", 0xCF},

    {"DH", 0xD0},
    {"~N", 0xD1},
    {"`O", 0xD2},
    {"'O", 0xD3},
    {"^O", 0xD4},
    {"~O", 0xD5},
    {"\"O", 0xD6},
    {"xx", 0xD7},
    {"/O", 0xD8},
    {"`U", 0xD9},
    {"'U", 0xDA},
    {"^U", 0xDB},
    {"\"U", 0xDC},
    {"'Y", 0xDD},
    {"TH", 0xDE},
    {"ss", 0xDF},

    {"`a", 0xE0},
    {"'a", 0xE1},
    {"^a", 0xE2},
    {"~a", 0xE3},
    {"\"a", 0xE4},
    {"oa", 0xE5},
    {"ae", 0xE6},
    {",c", 0xE7},
    {"`e", 0xE8},
    {"'e", 0xE9},
    {"^e", 0xEA},
    {"\"e", 0xEB},
    {"`i", 0xEC},
    {"'i", 0xED},
    {"^i", 0xEE},
    {"\"i", 0xEF},

    {"dh", 0xF0},
    {"~n", 0xF1},
    {"`o", 0xF2},
    {"'o", 0xF3},
    {"^o", 0xF4},
    {"~o", 0xF5},
    {"\"o", 0xF6},
    {"-:", 0xF7},
    {"/o", 0xF8},
    {"`u", 0xF9},
    {"'u", 0xFA},
    {"^u", 0xFB},
    {"\"u", 0xFC},
    {"'y", 0xFD},
    {"th", 0xFE},
    {"\"y", 0xFF},
};

class KeyboardInt : public Keyboard {
public:
    uint8_t           prevKeys[14] = {0};
    uint8_t           modifiers    = 0;
    QueueHandle_t     keyQueue;
    uint8_t           repeat       = 0;
    unsigned          pressCounter = 0;
    uint8_t           keyMode      = 3;
    SemaphoreHandle_t mutex;
    KeyLayout         curLayout    = KeyLayout::US;
    uint8_t           leds         = 0;
    uint8_t           composeFirst = 0;

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

    uint8_t compose(uint8_t first, uint8_t second) {
        // printf("Compose '%c' and '%c'\n", first, second);
        for (auto &combo : composeCombos) {
            if (((uint8_t)combo.combo[0] == first && (uint8_t)combo.combo[1] == second) ||
                ((uint8_t)combo.combo[0] == second && (uint8_t)combo.combo[1] == first))
                return combo.result;
        }

        if (first == 0xA8) {
            return compose('"', second);
        } else if (first == 0xB4) {
            return compose('\'', second);
        }
        return second;
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

    uint8_t layoutUK(unsigned scanCode) {
        uint8_t ch = 0;
        if (scanCode >= SCANCODE_A && scanCode <= SCANCODE_SLASH) {
            // clang-format off
        static const uint8_t lut2[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '"',0xA3, '$', '%', '^', '&', '*', '(', ')', '\r',0x03,'\b',0x8C, ' ', '_', '+', '{', '}', '~', '~', ':', '@',0xAC, '<', '>', '?'};
        static const uint8_t lut1[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\r',0x03,'\b','\t', ' ', '-', '=', '[', ']', '#', '#', ';','\'', '`', ',', '.', '/'};
        static const uint8_t lut3[] = {0xE1,   0,   0,   0,0xE9,   0,   0,   0,0xED,   0,   0,   0,   0,   0,0xF3,   0,   0,   0,   0,   0,0xFA,   0,   0,   0,   0,   0,   0,   0,   0,0x80,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,'\\','\\',   0,   0,0xA6,   0,   0,   0};
        static const uint8_t lut4[] = {0xC1,   0,   0,   0,0xC9,   0,   0,   0,0xCD,   0,   0,   0,   0,   0,0xD3,   0,   0,   0,   0,   0,0xDA,   0,   0,   0,   0,   0,   0,   0,   0,0x80,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0, '|', '|',   0,   0,0xA6,   0,   0,   0};
            // clang-format on

            if (modifiers & ModRAlt) {
                ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut4[scanCode - SCANCODE_A] : lut3[scanCode - SCANCODE_A];
            } else {
                ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut2[scanCode - SCANCODE_A] : lut1[scanCode - SCANCODE_A];
            }

        } else if (scanCode == SCANCODE_NONUSBACKSLASH) {
            ch = (modifiers & (ModLShift | ModRShift)) ? '|' : '\\';
        }
        return ch;
    }

    uint8_t layoutFR(unsigned scanCode) {
        uint8_t ch = 0;
        if (scanCode >= SCANCODE_A && scanCode <= SCANCODE_SLASH) {
            // clang-format off
        static const uint8_t lut2[] = { 'Q', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', '?', 'N', 'O', 'P', 'A', 'R', 'S', 'T', 'U', 'V', 'Z', 'X', 'Y', 'W', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\r',0x03,'\b',0x8C, ' ',0xB0, '+',0xA8,0xA3,0xB5,0xB5, 'M', '%',0xB3, '.', '/',0xA7};
        static const uint8_t lut1[] = { 'q', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', ',', 'n', 'o', 'p', 'a', 'r', 's', 't', 'u', 'v', 'z', 'x', 'y', 'w', '&',0xE9, '"','\'', '(', '-',0xE8, '_',0xE7,0xE0, '\r',0x03,'\b','\t', ' ', ')', '=', '^', '$', '*', '*', 'm',0xF9,0xB2, ';', ':', '!'};
        static const uint8_t lut3[] = {   0,   0,   0,   0,0x80,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '~', '#', '{', '[', '|', '`','\\', '^', '@',    0,   0,   0,   0,   0, ']', '}',   0,0xA4,   0,   0,   0,   0,   0,   0,   0,   0};
        static const uint8_t lut4[] = {   0,   0,   0,   0,0x80,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '~', '#', '{', '[', '|', '`','\\', '^', '@',    0,   0,   0,   0,   0, ']', '}',   0,0xA4,   0,   0,   0,   0,   0,   0,   0,   0};
            // clang-format on

            if (modifiers & ModRAlt) {
                ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut4[scanCode - SCANCODE_A] : lut3[scanCode - SCANCODE_A];
            } else {
                ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut2[scanCode - SCANCODE_A] : lut1[scanCode - SCANCODE_A];
            }

        } else if (scanCode == SCANCODE_NONUSBACKSLASH) {
            if (modifiers & ModRAlt)
                ch = '\\';
            else
                ch = (modifiers & (ModLShift | ModRShift)) ? '>' : '<';
        }

        // Handle dead-keys
        if ((scanCode == SCANCODE_LEFTBRACKET && ch == '^') || ch == 0xA8) {
            composeFirst = ch;
            ch           = 0;
        }
        return ch;
    }

    uint8_t layoutDE(unsigned scanCode) {
        uint8_t ch = 0;
        if (scanCode >= SCANCODE_A && scanCode <= SCANCODE_SLASH) {
            // clang-format off
        static const uint8_t lut2[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Z', 'Y', '!', '"',0xA7, '$', '%', '&', '/', '(', ')', '=', '\r',0x03,'\b',0x8C, ' ', '?', '`',0xDC, '*','\'','\'',0xD6,0xC4,0xB0, ';', ':', '_'};
        static const uint8_t lut1[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'z', 'y', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\r',0x03,'\b','\t', ' ',0xDF,0xB4,0xFC, '+', '#', '#',0xF6,0xE4, '^', ',', '.', '-'};
        static const uint8_t lut3[] = {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0xB5,   0,   0,   0, '@',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0xB2,0xB3,   0,   0,   0, '{', '[', ']', '}',    0,   0,   0,   0,   0,'\\',   0,   0, '~',   0,   0,   0,   0,   0,   0,   0,   0};
        static const uint8_t lut4[] = {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0xB5,   0,   0,   0, '@',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0xB2,0xB3,   0,   0,   0, '{', '[', ']', '}',    0,   0,   0,   0,   0,'\\',   0,   0, '~',   0,   0,   0,   0,   0,   0,   0,   0};
            // clang-format on

            if (modifiers & ModRAlt) {
                ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut4[scanCode - SCANCODE_A] : lut3[scanCode - SCANCODE_A];
            } else {
                ch = (modifiers & (ModLShift | ModRShift)) != 0 ? lut2[scanCode - SCANCODE_A] : lut1[scanCode - SCANCODE_A];
            }

        } else if (scanCode == SCANCODE_NONUSBACKSLASH) {
            if (modifiers & ModRAlt)
                ch = '|';
            else
                ch = (modifiers & (ModLShift | ModRShift)) ? '>' : '<';
        }

        // Handle dead-keys
        if (ch == '^' || ch == '`' || ch == 0xB4) {
            composeFirst = ch;
            ch           = 0;
        }

        return ch;
    }

    void processScancode(unsigned scanCode, bool keyDown) {
        // ESP_LOGI(TAG, "Key %3d %s", scanCode, keyDown ? "pressed" : "released");
        uint8_t prevLeds = leds;


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
        if (scanCode == SCANCODE_NUMLOCK && keyDown)
            leds ^= LedNumLock;
        if (scanCode == SCANCODE_CAPSLOCK && keyDown)
            leds ^= LedCapsLock;
        if (scanCode == SCANCODE_SCROLLLOCK && keyDown)
            leds ^= LedScrollLock;

        if (keyDown) {
            uint8_t ch = 0;
            switch (curLayout) {
                default:
                case KeyLayout::US: ch = layoutUS(scanCode); break;
                case KeyLayout::UK: ch = layoutUK(scanCode); break;
                case KeyLayout::FR: ch = layoutFR(scanCode); break;
                case KeyLayout::DE: ch = layoutDE(scanCode); break;
            }
            // printf("%d\n", scanCode);

            if (scanCode == SCANCODE_ESP_MENU || (scanCode == SCANCODE_TAB && (modifiers & ModLCtrl)))
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

            if (leds & LedCapsLock) {
                if (ch >= 'a' && ch <= 'z') {
                    ch = (ch - 'a') + 'A';
                } else if (ch >= 'A' && ch <= 'Z') {
                    ch = (ch - 'A') + 'a';
                }
            }

            if (ch > 0) {
                if (composeFirst > 0) {
                    ch           = compose(composeFirst, ch);
                    composeFirst = 0;

                } else if (modifiers & ModLAlt) {
                    composeFirst = ch;
                    ch           = 0;
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

        if (prevLeds != leds) {
            USBHost::instance().keyboardSetLeds(leds);
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
