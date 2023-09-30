#include "AqKeyboard.h"
#include "AqKeyboardDefs.h"

#ifdef EMULATOR
#    include <SDL.h>
#    include "EmuState.h"
#    include "Config.h"
#else
#    include "FPGA.h"
#    include <esp_system.h>
#    include "USBHost.h"
#    include "MemDump.h"
#endif

#ifndef EMULATOR
static const char *TAG = "keyboard";
#endif

#define FLAG_SHFT (1 << 7)
#define FLAG_CTRL (1 << 6)

struct ComposeCombo {
    const char *combo;
    uint8_t     result;
};

// From https://math.dartmouth.edu/~sarunas/Linux_Compose_Key_Sequences.html
static ComposeCombo composeCombos[] = {
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
    {"oA", 0xA5},
    {"!^", 0xA6},
    {"os", 0xA7},
    // how to type 0xA8?
    {"oc", 0xA9},
    {"oC", 0xA9},
    {"Oc", 0xA9},
    {"OC", 0xA9},
    // how to type 0xAA?
    {"<<", 0xAB},
    {"-,", 0xAC},
    // how to type 0xAD?
    {"or", 0xAE},
    {"oR", 0xAE},
    {"Or", 0xAE},
    {"OR", 0xAE},
    // how to type 0xAF?

    {"oo", 0xB0},
    {"+-", 0xB1},
    {"^2", 0xB2},
    {"^3", 0xB3},
    // how to type 0xB4?
    {"mu", 0xB5},
    {"p!", 0xB6},
    {"P!", 0xB6},
    {"PP", 0xB6},
    {"..", 0xB7},
    {", ", 0xB8},
    {"^1", 0xB9},
    // how to type 0xBA?
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

static KeyLayout curLayout = KeyLayout::US;

void setKeyLayout(KeyLayout layout) {
    curLayout = layout;
}

KeyLayout getKeyLayout() {
    return curLayout;
}

std::string getKeyLayoutName(KeyLayout layout) {
    switch (layout) {
        default: return "Unknown";
        case KeyLayout::US: return "US";
        case KeyLayout::UK: return "UK";
    }
}

void KeyboardLayout::processScancode(unsigned scanCode, bool keyDown) {
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
        // printf("%d\n", scanCode);

        switch (getKeyLayout()) {
            default:
            case KeyLayout::US: ch = layoutUS(scanCode); break;
            case KeyLayout::UK: ch = layoutUK(scanCode); break;
        }

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
#ifdef EMULATOR
            emuState.kbBufWrite(ch);
#else
            FPGA::instance().aqpWriteKeybBuffer(ch);
#endif
            // printf("'%c' (%02x)\n", (ch >= ' ' && ch <= '~') ? ch : '.', ch);
        }
    }
}

uint8_t KeyboardLayout::compose(uint8_t first, uint8_t second) {
    // printf("Compose '%c' and '%c'\n", first, second);
    for (auto &combo : composeCombos) {
        if ((combo.combo[0] == first && combo.combo[1] == second) ||
            (combo.combo[0] == second && combo.combo[1] == first))
            return combo.result;
    }
    return second;
}

uint8_t KeyboardLayout::layoutUS(unsigned scanCode) {
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

uint8_t KeyboardLayout::layoutUK(unsigned scanCode) {
    uint8_t ch = 0;
    if (scanCode >= SCANCODE_A && scanCode <= SCANCODE_SLASH) {
        // clang-format off
        static const uint8_t lut1[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\r',0x03,'\b','\t', ' ', '-', '=', '[', ']', '~', '~', ';','\'', '`', ',', '.', '/'};
        static const uint8_t lut2[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '"',0xA3, '$', '%', '^', '&', '*', '(', ')', '\r',0x03,'\b',0x8C, ' ', '_', '+', '{', '}', '#', '#', ':', '@',0xAC, '<', '>', '?'};
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

AqKeyboard::AqKeyboard() {
}

AqKeyboard &AqKeyboard::instance() {
    static AqKeyboard obj;
    return obj;
}

void AqKeyboard::init() {
#ifndef EMULATOR
    mutex = xSemaphoreCreateRecursiveMutex();
#endif
}

void AqKeyboard::handleScancode(unsigned scanCode, bool keyDown) {
    // printf("%3d: %s\n", scanCode, keyDown ? "DOWN" : "UP");
#ifndef EMULATOR
    RecursiveMutexLock lock(mutex);
#endif
    // Hand controller emulation
    if (handController(scanCode, keyDown))
        return;

    // Keyboard layout handling
    kbLayout.processScancode(scanCode, keyDown);

    // Keyboard matrix emulation
    {
        int key = -1;
        switch (scanCode) {
            case SCANCODE_EQUALS: key = KEY_EQUALS; break;
            case SCANCODE_BACKSPACE: key = KEY_BACKSPACE; break;
            case SCANCODE_APOSTROPHE: key = KEY_COLON; break;
            case SCANCODE_RETURN: key = KEY_RETURN; break;
            case SCANCODE_SEMICOLON: key = KEY_SEMICOLON; break;
            case SCANCODE_PERIOD: key = KEY_PERIOD; break;
            case SCANCODE_INSERT: key = KEY_INSERT; break;
            case SCANCODE_DELETE: key = KEY_DELETE; break;
            case SCANCODE_MINUS: key = KEY_MINUS; break;
            case SCANCODE_SLASH: key = KEY_SLASH; break;
            case SCANCODE_0: key = KEY_0; break;
            case SCANCODE_P: key = KEY_P; break;
            case SCANCODE_L: key = KEY_L; break;
            case SCANCODE_COMMA: key = KEY_COMMA; break;
            case SCANCODE_UP: key = KEY_UP; break;
            case SCANCODE_RIGHT: key = KEY_RIGHT; break;
            case SCANCODE_9: key = KEY_9; break;
            case SCANCODE_O: key = KEY_O; break;
            case SCANCODE_K: key = KEY_K; break;
            case SCANCODE_M: key = KEY_M; break;
            case SCANCODE_N: key = KEY_N; break;
            case SCANCODE_J: key = KEY_J; break;
            case SCANCODE_LEFT: key = KEY_LEFT; break;
            case SCANCODE_DOWN: key = KEY_DOWN; break;
            case SCANCODE_8: key = KEY_8; break;
            case SCANCODE_I: key = KEY_I; break;
            case SCANCODE_7: key = KEY_7; break;
            case SCANCODE_U: key = KEY_U; break;
            case SCANCODE_H: key = KEY_H; break;
            case SCANCODE_B: key = KEY_B; break;
            case SCANCODE_HOME: key = KEY_HOME; break;
            case SCANCODE_END: key = KEY_END; break;
            case SCANCODE_6: key = KEY_6; break;
            case SCANCODE_Y: key = KEY_Y; break;
            case SCANCODE_G: key = KEY_G; break;
            case SCANCODE_V: key = KEY_V; break;
            case SCANCODE_C: key = KEY_C; break;
            case SCANCODE_F: key = KEY_F; break;
            case SCANCODE_PAGEUP: key = KEY_PGUP; break;
            case SCANCODE_PAGEDOWN: key = KEY_PGDN; break;
            case SCANCODE_5: key = KEY_5; break;
            case SCANCODE_T: key = KEY_T; break;
            case SCANCODE_4: key = KEY_4; break;
            case SCANCODE_R: key = KEY_R; break;
            case SCANCODE_D: key = KEY_D; break;
            case SCANCODE_X: key = KEY_X; break;
            case SCANCODE_PAUSE: key = KEY_PAUSE; break;
            case SCANCODE_PRINTSCREEN: key = KEY_PRTSCR; break;
            case SCANCODE_3: key = KEY_3; break;
            case SCANCODE_E: key = KEY_E; break;
            case SCANCODE_S: key = KEY_S; break;
            case SCANCODE_Z: key = KEY_Z; break;
            case SCANCODE_SPACE: key = KEY_SPACE; break;
            case SCANCODE_A: key = KEY_A; break;
            case SCANCODE_APPLICATION: key = KEY_MENU; break;
            case SCANCODE_TAB: key = KEY_TAB; break;
            case SCANCODE_2: key = KEY_2; break;
            case SCANCODE_W: key = KEY_W; break;
            case SCANCODE_1: key = KEY_1; break;
            case SCANCODE_Q: key = KEY_Q; break;
            default: break;
        }
        if (key >= 0) {
            if (keyDown)
                keybMatrix |= (1ULL << key);
            else
                keybMatrix &= ~(1ULL << key);
        }

        if (kbLayout.modifiers & (KeyboardLayout::ModLShift | KeyboardLayout::ModRShift))
            keybMatrix |= (1ULL << KEY_SHIFT);
        else
            keybMatrix &= ~(1ULL << KEY_SHIFT);

        if (kbLayout.modifiers & (KeyboardLayout::ModLAlt | KeyboardLayout::ModRAlt))
            keybMatrix |= (1ULL << KEY_ALT);
        else
            keybMatrix &= ~(1ULL << KEY_ALT);

        if (kbLayout.modifiers & (KeyboardLayout::ModLCtrl | KeyboardLayout::ModRCtrl))
            keybMatrix |= (1ULL << KEY_CTRL);
        else
            keybMatrix &= ~(1ULL << KEY_CTRL);

        if (kbLayout.modifiers & (KeyboardLayout::ModLGui | KeyboardLayout::ModRGui))
            keybMatrix |= (1ULL << KEY_GUI);
        else
            keybMatrix &= ~(1ULL << KEY_GUI);
    }

    // Special keys
    {
        uint8_t combinedModifiers = (kbLayout.modifiers & 0xF) | (kbLayout.modifiers >> 4);
        if (scanCode == SCANCODE_ESCAPE && keyDown) {
            if (combinedModifiers == KeyboardLayout::ModLCtrl) {
#ifdef EMULATOR
                emuState.reset();
#else
                FPGA::instance().aqpReset();
#endif
            } else if (combinedModifiers == (KeyboardLayout::ModLShift | KeyboardLayout::ModLCtrl)) {
#ifdef EMULATOR
                emuState.reset();
#else
                // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                FPGA::instance().aqpAqcuireBus();
                FPGA::instance().aqpReset();
                esp_restart();
#endif
            }
        }

#ifndef EMULATOR
        else if (combinedModifiers == KeyboardLayout::ModLGui && keyDown && scanCode == SCANCODE_F12) {
            MemDump::dumpCartridge();
        } else if (keyDown && scanCode == SCANCODE_PRINTSCREEN) {
            MemDump::dumpScreen();
        }
#endif
    }

    if (ledStatus != kbLayout.leds) {
        ledStatus = kbLayout.leds;
#ifndef EMULATOR
        USBHost::instance().keyboardSetLeds(ledStatus);
#endif
    }
}

bool AqKeyboard::handController(unsigned scanCode, bool keyDown) {
    handCtrl1 = 0xFF;
    if ((ledStatus & SCROLL_LOCK) == 0) {
        handCtrl1Pressed = 0;
        return false;
    }

    bool result = true;

    enum {
        UP    = (1 << 0),
        DOWN  = (1 << 1),
        LEFT  = (1 << 2),
        RIGHT = (1 << 3),
        K1    = (1 << 4),
        K2    = (1 << 5),
        K3    = (1 << 6),
        K4    = (1 << 7),
        K5    = (1 << 8),
        K6    = (1 << 9),
    };

    switch (scanCode) {
        case SCANCODE_UP: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | UP) : (handCtrl1Pressed & ~UP); break;
        case SCANCODE_DOWN: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | DOWN) : (handCtrl1Pressed & ~DOWN); break;
        case SCANCODE_LEFT: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | LEFT) : (handCtrl1Pressed & ~LEFT); break;
        case SCANCODE_RIGHT: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | RIGHT) : (handCtrl1Pressed & ~RIGHT); break;
        case SCANCODE_F1: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | K1) : (handCtrl1Pressed & ~K1); break;
        case SCANCODE_F2: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | K2) : (handCtrl1Pressed & ~K2); break;
        case SCANCODE_F3: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | K3) : (handCtrl1Pressed & ~K3); break;
        case SCANCODE_F4: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | K4) : (handCtrl1Pressed & ~K4); break;
        case SCANCODE_F5: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | K5) : (handCtrl1Pressed & ~K5); break;
        case SCANCODE_F6: handCtrl1Pressed = (keyDown) ? (handCtrl1Pressed | K6) : (handCtrl1Pressed & ~K6); break;
        default: result = false;
    }

    switch (handCtrl1Pressed & 0xF) {
        case LEFT: handCtrl1 &= ~(1 << 3); break;
        case UP | LEFT: handCtrl1 &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case UP: handCtrl1 &= ~(1 << 2); break;
        case UP | RIGHT: handCtrl1 &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case RIGHT: handCtrl1 &= ~(1 << 1); break;
        case DOWN | RIGHT: handCtrl1 &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case DOWN: handCtrl1 &= ~(1 << 0); break;
        case DOWN | LEFT: handCtrl1 &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        default: break;
    }
    if (handCtrl1Pressed & K1)
        handCtrl1 &= ~(1 << 6);
    if (handCtrl1Pressed & K2)
        handCtrl1 &= ~((1 << 7) | (1 << 2));
    if (handCtrl1Pressed & K3)
        handCtrl1 &= ~((1 << 7) | (1 << 5));
    if (handCtrl1Pressed & K4)
        handCtrl1 &= ~(1 << 5);
    if (handCtrl1Pressed & K5)
        handCtrl1 &= ~((1 << 7) | (1 << 1));
    if (handCtrl1Pressed & K6)
        handCtrl1 &= ~((1 << 7) | (1 << 0));

    return result;
}

void AqKeyboard::updateMatrix() {
#ifndef EMULATOR
    RecursiveMutexLock lock(mutex);
#endif
    if (prevMatrix != keybMatrix) {
        // printf("keybMatrix: %016llx\n", keybMatrix);

        uint64_t tmpMatrix = ~keybMatrix;

        if (!dontSend) {
#ifdef EMULATOR
            memcpy(emuState.keybMatrix, &tmpMatrix, 8);
#else
            FPGA::instance().aqpUpdateKeybMatrix(tmpMatrix);
#endif
        }
        prevMatrix = keybMatrix;
    }

    if (prevHandCtrl1 != handCtrl1 || prevHandCtrl2 != handCtrl2) {
#ifdef EMULATOR
        emuState.handCtrl1 = handCtrl1;
        emuState.handCtrl2 = handCtrl2;
#else
        FPGA::instance().aqpUpdateHandCtrl(handCtrl1, handCtrl2);
#endif
        prevHandCtrl1 = handCtrl1;
        prevHandCtrl2 = handCtrl2;
    }
}

void AqKeyboard::pressKey(unsigned char ch) {
    if (ch == '\n') {
        ch = '\r';
    }

#ifdef EMULATOR
    emuState.kbBufWrite(ch);
#else
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
    if (ch > '~')
        return;
    FPGA::instance().aqpWriteKeybBuffer(ch);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Delay a little longer on reset
    if (ch == 0x1E)
        vTaskDelay(pdMS_TO_TICKS(500));
#endif
}
