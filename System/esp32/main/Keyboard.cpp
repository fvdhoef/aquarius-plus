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

#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
static const uint8_t morphBookKeyLut[54] = {
    SCANCODE_ESCAPE,      //  0: Esc
    SCANCODE_1,           //  1: 1
    SCANCODE_TAB,         //  2: Tab
    SCANCODE_Q,           //  3: Q
    SCANCODE_LALT,        //  4: Modifier: Left Alt
    SCANCODE_A,           //  5: A
    SCANCODE_LSHIFT,      //  6: Modifier: Left Shift
    0,                    //  7:
    SCANCODE_LCTRL,       //  8: Modifier: Left Ctrl
    SCANCODE_2,           //  9: 2
    SCANCODE_3,           // 10: 3
    SCANCODE_W,           // 11: W
    SCANCODE_E,           // 12: E
    SCANCODE_S,           // 13: S
    SCANCODE_D,           // 14: D
    SCANCODE_Z,           // 15: Z
    SCANCODE_X,           // 16: X
    0,                    // 17: Left Fn
    SCANCODE_4,           // 18: 4
    SCANCODE_5,           // 19: 5
    SCANCODE_R,           // 20: R
    SCANCODE_T,           // 21: T
    SCANCODE_F,           // 22: F
    SCANCODE_G,           // 23: G
    SCANCODE_C,           // 24: C
    SCANCODE_V,           // 25: V
    SCANCODE_SPACE,       // 26: Space
    SCANCODE_6,           // 27: 6
    SCANCODE_7,           // 28: 7
    SCANCODE_Y,           // 29: Y
    SCANCODE_U,           // 30: U
    SCANCODE_H,           // 31: H
    SCANCODE_J,           // 32: J
    SCANCODE_B,           // 33: B
    SCANCODE_N,           // 34: N
    SCANCODE_RIGHT,       // 35: Right
    SCANCODE_8,           // 36: 8
    SCANCODE_9,           // 37: 9
    SCANCODE_I,           // 38: I
    SCANCODE_O,           // 39: O
    SCANCODE_K,           // 40: K
    SCANCODE_L,           // 41: L
    SCANCODE_M,           // 42: M
    SCANCODE_PERIOD,      // 43: .
    SCANCODE_LEFT,        // 44: Left
    SCANCODE_0,           // 45: 0
    SCANCODE_BACKSPACE,   // 46: Bksp
    SCANCODE_P,           // 47: P
    SCANCODE_LEFTBRACKET, // 48: [
    SCANCODE_RETURN,      // 49: Enter
    0,                    // 50:
    SCANCODE_UP,          // 51: Up
    0,                    // 52: Right Fn
    SCANCODE_DOWN,        // 53: Down
};
#endif

class KeyboardInt : public Keyboard {
public:
#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
    uint64_t prevKeys = 0;
#endif
    uint8_t           modifiers = 0;
    QueueHandle_t     keyQueue;
    QueueHandle_t     scanCodeQueue;
    uint8_t           repeat       = 0;
    unsigned          pressCounter = 0;
    uint8_t           keyMode      = 3;
    SemaphoreHandle_t mutex;
    KeyLayout         curLayout    = KeyLayout::US;
    uint8_t           leds         = 0;
    uint8_t           composeFirst = 0;

    KeyboardInt() {
        mutex         = xSemaphoreCreateRecursiveMutex();
        keyQueue      = xQueueCreate(128, 1);
        scanCodeQueue = xQueueCreate(1, 1);

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

#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
    int translateKeyToScanCode(unsigned scanCode, bool leftFn, bool rightFn) {
        int result = morphBookKeyLut[scanCode];

        // Handle Fn keys
        // clang-format off
        // #pragma GCC diagnostic ignored "-Wmisleading-indentation"
        switch (result) {
            case SCANCODE_ESCAPE:      if      (leftFn || rightFn) result = SCANCODE_GRAVE;        break;
            case SCANCODE_1:           if      (leftFn && rightFn) result = SCANCODE_F11;          
                                       else if (leftFn || rightFn) result = SCANCODE_F1;           
                                       break;
            case SCANCODE_2:           if      (leftFn && rightFn) result = SCANCODE_F12;          
                                       else if (leftFn || rightFn) result = SCANCODE_F2;           
                                       break;
            case SCANCODE_3:           if      (leftFn || rightFn) result = SCANCODE_F3;           break;
            case SCANCODE_4:           if      (leftFn || rightFn) result = SCANCODE_F4;           break;
            case SCANCODE_5:           if      (leftFn || rightFn) result = SCANCODE_F5;           break;
            case SCANCODE_6:           if      (leftFn || rightFn) result = SCANCODE_F6;           break;
            case SCANCODE_7:           if      (leftFn || rightFn) result = SCANCODE_F7;           break;
            case SCANCODE_8:           if      (leftFn || rightFn) result = SCANCODE_F8;           break;
            case SCANCODE_9:           if      (leftFn && rightFn) result = SCANCODE_F9;           
                                       else if (leftFn || rightFn) result = SCANCODE_MINUS;        
                                       break;
            case SCANCODE_0:           if      (leftFn && rightFn) result = SCANCODE_F10;          
                                       else if (leftFn || rightFn) result = SCANCODE_EQUALS;       
                                       break;
            case SCANCODE_BACKSPACE:   if      (leftFn || rightFn) result = SCANCODE_DELETE;       break;
            case SCANCODE_LEFTBRACKET: if      (leftFn || rightFn) result = SCANCODE_RIGHTBRACKET; break;
            case SCANCODE_J:           if      (leftFn || rightFn) result = SCANCODE_SEMICOLON;    break;
            case SCANCODE_K:           if      (leftFn || rightFn) result = SCANCODE_APOSTROPHE;   break;
            case SCANCODE_L:           if      (leftFn || rightFn) result = SCANCODE_BACKSLASH;    break;
            case SCANCODE_M:           if      (leftFn || rightFn) result = SCANCODE_COMMA;        break;
            case SCANCODE_PERIOD:      if      (leftFn || rightFn) result = SCANCODE_SLASH;        break;
            case SCANCODE_UP:          if      (leftFn || rightFn) result = SCANCODE_PAGEUP;       break;
            case SCANCODE_LEFT:        if      (leftFn || rightFn) result = SCANCODE_HOME;         break;
            case SCANCODE_DOWN:        if      (leftFn || rightFn) result = SCANCODE_PAGEDOWN;     break;
            case SCANCODE_RIGHT:       if      (leftFn || rightFn) result = SCANCODE_END;          break;
            default: break;
        }
        // clang-format on

        return result;
    }

    void updateKeys(uint64_t keys) override {
        RecursiveMutexLock lock(mutex);

        auto releasedKeys = prevKeys & ~keys;
        auto pressedKeys  = ~prevKeys & keys;
        prevKeys          = keys;

        bool leftFn  = (keys & (1ULL << 17)) != 0;
        bool rightFn = (keys & (1ULL << 52)) != 0;

        if (releasedKeys) {
            for (unsigned i = 0; i < 54; i++) {
                if (releasedKeys & (1ULL << i)) {
                    // ESP_LOGI(TAG, "key %2d released", i);
                    auto scanCode = translateKeyToScanCode(i, leftFn, rightFn);
                    if (scanCode >= 0) {
                        handleScancode(scanCode, false);
                    }
                }
            }
        }

        if (pressedKeys) {
            for (unsigned i = 0; i < 54; i++) {
                if (pressedKeys & (1ULL << i)) {
                    // ESP_LOGI(TAG, "key %2d pressed", i);
                    auto scanCode = translateKeyToScanCode(i, leftFn, rightFn);
                    if (scanCode >= 0) {
                        handleScancode(scanCode, true);
                    }
                }
            }
        }
    }
#endif

    void handleScancode(unsigned scanCode, bool keyDown) override {
        RecursiveMutexLock lock(mutex);
        repeat       = 0;
        pressCounter = 0;
        if (keyDown) {
            uint8_t val = scanCode;
            xQueueSend(scanCodeQueue, &val, 0);
        }

        // Keyboard layout handling
        int ch = processScancode(scanCode, keyDown);

        bool stopProcessing = false;
        auto core           = getFpgaCore();
        if (core && (!getDisplayOverlay()->isVisible() || !keyDown)) {
            stopProcessing = core->keyScancode(modifiers, scanCode, keyDown);
        }

        if (!stopProcessing && ch > 0) {
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

    int processScancode(unsigned scanCode, bool keyDown) {
        int result = -1;

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

            if (scanCode == SCANCODE_TAB && (modifiers & ModLCtrl))
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

            if (ch > 0)
                result = ch;

            // printf("%c\n", ch);
        }

        if (prevLeds != leds) {
            getUSBHost()->keyboardSetLeds(leds);
        }
        return result;
    }

    int getKey(TickType_t ticksToWait) override {
        uint8_t result;
        if (!xQueueReceive(keyQueue, &result, ticksToWait))
            return -1;
        return result;
    }

    int waitScanCode() override {
        uint8_t result;
        xQueueReceive(scanCodeQueue, &result, 0);
        if (!xQueueReceive(scanCodeQueue, &result, portMAX_DELAY))
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

    void pressKey(uint8_t ch) {
        auto core = getFpgaCore();
        if (!core)
            return;

        if (ch == '\n') {
            ch = '\r';
        }

        if (ch == 0x1C) {
            // Delay for 100ms
            vTaskDelay(pdMS_TO_TICKS(100));
            return;
        }
        if (ch == 0x1D) {
            // Delay for 500ms
            vTaskDelay(pdMS_TO_TICKS(500));
            return;
        }
        if (ch == 0x1E) {
            // Reset
            core->resetCore();
            vTaskDelay(pdMS_TO_TICKS(500));
            return;
        }
        if (ch > '~')
            return;

        core->keyChar(ch, false);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
};

Keyboard *Keyboard::instance() {
    static KeyboardInt obj;
    return &obj;
}

static const char *scanCodeNames[] = {
    "A",         // 4
    "B",         // 5
    "C",         // 6
    "D",         // 7
    "E",         // 8
    "F",         // 9
    "G",         // 10
    "H",         // 11
    "I",         // 12
    "J",         // 13
    "K",         // 14
    "L",         // 15
    "M",         // 16
    "N",         // 17
    "O",         // 18
    "P",         // 19
    "Q",         // 20
    "R",         // 21
    "S",         // 22
    "T",         // 23
    "U",         // 24
    "V",         // 25
    "W",         // 26
    "X",         // 27
    "Y",         // 28
    "Z",         // 29
    "1",         // 30
    "2",         // 31
    "3",         // 32
    "4",         // 33
    "5",         // 34
    "6",         // 35
    "7",         // 36
    "8",         // 37
    "9",         // 38
    "0",         // 39
    "Enter",     // 40
    "Escape",    // 41
    "Backspace", // 42
    "Tab",       // 43
    "Space",     // 44
    "-",         // 45
    "=",         // 46
    "[",         // 47
    "]",         // 48
    "\\",        // 49
    "#",         // 50
    ";",         // 51
    "'",         // 52
    "`",         // 53
    ",",         // 54
    ".",         // 55
    "/",         // 56
    "CapsLock",  // 57
    "F1",        // 58
    "F2",        // 59
    "F3",        // 60
    "F4",        // 61
    "F5",        // 62
    "F6",        // 63
    "F7",        // 64
    "F8",        // 65
    "F9",        // 66
    "F10",       // 67
    "F11",       // 68
    "F12",       // 69
    "PrtScr",    // 70
    "ScrollLk",  // 71
    "Pause",     // 72
    "Insert",    // 73
    "Home",      // 74
    "Page Up",   // 75
    "Delete",    // 76
    "End",       // 77
    "Page Down", // 78
    "Right",     // 79
    "Left",      // 80
    "Down",      // 81
    "Up",        // 82
    "NumLock",   // 83
    "KP /",      // 84
    "KP *",      // 85
    "KP -",      // 86
    "KP +",      // 87
    "KP Enter",  // 88
    "KP 1",      // 89
    "KP 2",      // 90
    "KP 3",      // 91
    "KP 4",      // 92
    "KP 5",      // 93
    "KP 6",      // 94
    "KP 7",      // 95
    "KP 8",      // 96
    "KP 9",      // 97
    "KP 0",      // 98
    "KP .",      // 99
};

static const char *scanCodeNames2[] = {
    "Left Ctrl",   // 224
    "Left Shift",  // 225
    "Left Alt",    // 226
    "Left GUI",    // 227
    "Right Ctrl",  // 228
    "Right Shift", // 229
    "Right Alt",   // 230
    "Right GUI",   // 231
};

const char *getScanCodeName(uint8_t scanCode) {
    if (scanCode >= 4 && scanCode <= 99)
        return scanCodeNames[scanCode - 4];
    if (scanCode >= 224 && scanCode <= 231)
        return scanCodeNames2[scanCode - 224];
    return nullptr;
}
