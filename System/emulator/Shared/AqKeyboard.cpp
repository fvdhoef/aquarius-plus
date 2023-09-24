#include "AqKeyboard.h"
#include "AqKeyboardDefs.h"
#include "KeyMaps.h"

#ifdef EMULATOR
#    include <SDL.h>
#    include "EmuState.h"
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

    uint8_t ledStatusNext = ledStatus == 0xFF ? 0 : ledStatus;
    if (keyDown && scanCode == SCANCODE_CAPSLOCK)
        ledStatusNext ^= CAPS_LOCK;
    if (keyDown && scanCode == SCANCODE_NUMLOCKCLEAR)
        ledStatusNext ^= NUM_LOCK;
    if (keyDown && scanCode == SCANCODE_SCROLLLOCK)
        ledStatusNext ^= SCROLL_LOCK;

    // Keep track of pressed modifier keys
    if (scanCode == SCANCODE_LSHIFT)
        modifiers = (modifiers & ~KEYMOD_LSHIFT) | (keyDown ? KEYMOD_LSHIFT : 0);
    if (scanCode == SCANCODE_RSHIFT)
        modifiers = (modifiers & ~KEYMOD_RSHIFT) | (keyDown ? KEYMOD_RSHIFT : 0);
    if (scanCode == SCANCODE_LCTRL)
        modifiers = (modifiers & ~KEYMOD_LCTRL) | (keyDown ? KEYMOD_LCTRL : 0);
    if (scanCode == SCANCODE_RCTRL)
        modifiers = (modifiers & ~KEYMOD_RCTRL) | (keyDown ? KEYMOD_RCTRL : 0);
    if (scanCode == SCANCODE_LALT)
        modifiers = (modifiers & ~KEYMOD_LALT) | (keyDown ? KEYMOD_LALT : 0);
    if (scanCode == SCANCODE_RALT)
        modifiers = (modifiers & ~KEYMOD_RALT) | (keyDown ? KEYMOD_RALT : 0);
    if (scanCode == SCANCODE_LGUI)
        modifiers = (modifiers & ~KEYMOD_LGUI) | (keyDown ? KEYMOD_LGUI : 0);

    uint64_t modMask =
        ((modifiers & (KEYMOD_LCTRL | KEYMOD_RCTRL)) ? (1ULL << KEY_CTRL) : 0) |
        ((modifiers & (KEYMOD_LALT | KEYMOD_RALT)) ? (1ULL << KEY_ALT) : 0) |
        ((modifiers & (KEYMOD_LSHIFT | KEYMOD_RSHIFT)) ? (1ULL << KEY_SHIFT) : 0) |
        ((modifiers & (KEYMOD_LGUI | KEYMOD_RGUI)) ? (1ULL << KEY_GUI) : 0);

    if (scanCode == SCANCODE_RGUI)
        modifiers = (modifiers & ~KEYMOD_RGUI) | (keyDown ? KEYMOD_RGUI : 0);

    bool ctrlPressed  = (modifiers & (KEYMOD_LCTRL | KEYMOD_RCTRL)) != 0;
    bool altPressed   = (modifiers & (KEYMOD_LALT | KEYMOD_RALT)) != 0;
    bool shiftPressed = (modifiers & (KEYMOD_LSHIFT | KEYMOD_RSHIFT)) != 0;
    bool guiPressed   = (modifiers & (KEYMOD_LGUI | KEYMOD_RGUI)) != 0;
    bool onlyShift    = (modifiers & KEYMOD_SHIFT) != 0 && (modifiers & ~KEYMOD_SHIFT) == 0;
    bool doCapsToggle = (ledStatus & CAPS_LOCK) && (onlyShift || modifiers == 0);

    // printf("modifiers: %016llx\n", modMask);

    // Handle RESET key-combos
    if (scanCode == SCANCODE_ESCAPE) {
        if (keyDown) {
            if (ctrlPressed && shiftPressed) {
                dontSend = true;
#ifdef EMULATOR
                emuState.reset();
#else
                // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                FPGA::instance().aqpAqcuireBus();
                FPGA::instance().aqpReset();
                esp_restart();
#endif
            } else if (ctrlPressed) {
                dontSend = true;

#ifdef EMULATOR
                emuState.reset();
#else
                FPGA::instance().aqpReset();
#endif
            }
        } else {
            dontSend = false;
        }
    }

    const keymap_t *keymap = getKeyMap();

    // Lookup key
    uint64_t keyMask = 0;
    if (keyDown) {
        uint16_t code = 0xFFFF;
        if (scanCode >= 4 && scanCode <= 101) {
            if (onlyShift) {
                code = (*keymap)[scanCode - 4][1];
                if (doCapsToggle && (code & CAPS)) {
                    code = (*keymap)[scanCode - 4][0];
                }
            } else {
                code = (*keymap)[scanCode - 4][0];
                if (doCapsToggle && (code & CAPS)) {
                    code = (*keymap)[scanCode - 4][1];
                }

                if (ctrlPressed)
                    code |= CTRL;
                if (altPressed)
                    code |= ALT;
                if (guiPressed)
                    code |= GUI;
            }

            if (code & CTRL)
                keyMask |= 1ULL << KEY_CTRL;
            if (code & ALT)
                keyMask |= 1ULL << KEY_ALT;
            if (code & SHFT)
                keyMask |= 1ULL << KEY_SHIFT;
            if (code & GUI)
                keyMask |= 1ULL << KEY_GUI;
            keyMask |= 1ULL << (code & 63);
        }

        // Store the key mask for use on key up
        keyMaskMap.insert(std::make_pair(scanCode, keyMask));

    } else {
        // Use the key mask that was used on key down
        auto it = keyMaskMap.find(scanCode);
        if (it != keyMaskMap.end()) {
            keyMask = it->second;
        }
    }

    // printf("keyMask   : %016llx\n", keyMask);

    // Check if new key is compatible with already pressed keys
    if (keyMask == 0 && (keybMatrix & KEY_MASK) == 0) {
        keybMatrix = modMask;
    } else {
        if (keyDown) {
            if ((keyMask & KEY_MOD_MASK) == (keybMatrix & KEY_MOD_MASK)) {
                // Modifiers are the same, so add key to already pressed keys
                keybMatrix |= keyMask;
            } else {
                // Modifiers are different, replace keyboard matrix
                keybMatrix = keyMask;
            }
        } else {
            if ((keyMask & KEY_MOD_MASK) == (keybMatrix & KEY_MOD_MASK)) {
                // Modifiers are the same, so remove key
                keybMatrix &= ~(keyMask & KEY_MASK);
            } else {
                // Modifiers are different, clear all keys
                keybMatrix = 0;
            }
        }
    }

    if (ledStatus != ledStatusNext) {
        ledStatus = ledStatusNext;
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

#ifdef EMULATOR
void AqKeyboard::pressKey(unsigned char ch, bool keyDown) {
    if (ch > '~')
        return;

    uint8_t val = scanCodeLut[ch];
    if (val == 0)
        return;

    if (val & FLAG_SHFT)
        handleScancode(SCANCODE_LSHIFT, keyDown);
    if (val & FLAG_CTRL)
        handleScancode(SCANCODE_LCTRL, keyDown);
    handleScancode(val & 0x3F, keyDown);
    AqKeyboard::instance().updateMatrix();
}
#else
void AqKeyboard::pressKey(unsigned ch) {
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
    uint8_t val = scanCodeLut[ch];
    if (val == 0)
        return;

    if (val & FLAG_SHFT)
        handleScancode(SCANCODE_LSHIFT, true);
    if (val & FLAG_CTRL)
        handleScancode(SCANCODE_LCTRL, true);
    handleScancode(val & 0x3F, true);
    updateMatrix();
    vTaskDelay(pdMS_TO_TICKS(20));

    if (val & FLAG_SHFT)
        handleScancode(SCANCODE_LSHIFT, false);
    if (val & FLAG_CTRL)
        handleScancode(SCANCODE_LCTRL, false);
    handleScancode(val & 0x3F, false);
    updateMatrix();
    vTaskDelay(pdMS_TO_TICKS(20));

    // Delay a little longer on reset
    if (ch == 0x1E)
        vTaskDelay(pdMS_TO_TICKS(500));
}
#endif

const uint8_t AqKeyboard::scanCodeLut[] = {
    0,                                       //  0 CTRL-@
    0,                                       //  1 CTRL-A
    0,                                       //  2 CTRL-B
    FLAG_CTRL | SCANCODE_C,                  //  3 CTRL-C
    0,                                       //  4 CTRL-D
    0,                                       //  5 CTRL-E
    0,                                       //  6 CTRL-F
    FLAG_CTRL | SCANCODE_G,                  //  7 CTRL-G
    0,                                       //  8 CTRL-H
    0,                                       //  9 CTRL-I
    SCANCODE_RETURN,                         // 10 CTRL-J \n
    0,                                       // 11 CTRL-K
    0,                                       // 12 CTRL-L
    0,                                       // 13 CTRL-M \r
    0,                                       // 14 CTRL-N
    0,                                       // 15 CTRL-O
    0,                                       // 16 CTRL-P
    0,                                       // 17 CTRL-Q
    0,                                       // 18 CTRL-R
    0,                                       // 19 CTRL-S
    0,                                       // 20 CTRL-T
    0,                                       // 21 CTRL-U
    0,                                       // 22 CTRL-V
    0,                                       // 23 CTRL-W
    0,                                       // 24 CTRL-X
    0,                                       // 25 CTRL-Y
    0,                                       // 26 CTRL-Z
    0,                                       // 27 CTRL-[
    0,                                       // \x1C 28 CTRL-backslash
    0,                                       // \x1D 29 CTRL-]
    FLAG_CTRL | SCANCODE_ESCAPE,             // \x1E 30 CTRL-^
    FLAG_CTRL | FLAG_SHFT | SCANCODE_ESCAPE, // \x1F 31 CTRL-_
    SCANCODE_SPACE,                          // Space
    FLAG_SHFT | SCANCODE_1,                  // !
    FLAG_SHFT | SCANCODE_APOSTROPHE,         // "
    FLAG_SHFT | SCANCODE_3,                  // #
    FLAG_SHFT | SCANCODE_4,                  // $
    FLAG_SHFT | SCANCODE_5,                  // %
    FLAG_SHFT | SCANCODE_7,                  // &
    SCANCODE_APOSTROPHE,                     // '
    FLAG_SHFT | SCANCODE_9,                  // (
    FLAG_SHFT | SCANCODE_0,                  // )
    FLAG_SHFT | SCANCODE_8,                  // *
    FLAG_SHFT | SCANCODE_EQUALS,             // +
    SCANCODE_COMMA,                          // ,
    SCANCODE_MINUS,                          // -
    SCANCODE_PERIOD,                         // .
    SCANCODE_SLASH,                          // /
    SCANCODE_0,                              // 0
    SCANCODE_1,                              // 1
    SCANCODE_2,                              // 2
    SCANCODE_3,                              // 3
    SCANCODE_4,                              // 4
    SCANCODE_5,                              // 5
    SCANCODE_6,                              // 6
    SCANCODE_7,                              // 7
    SCANCODE_8,                              // 8
    SCANCODE_9,                              // 9
    FLAG_SHFT | SCANCODE_SEMICOLON,          // :
    SCANCODE_SEMICOLON,                      // ;
    FLAG_SHFT | SCANCODE_COMMA,              // <
    SCANCODE_EQUALS,                         // =
    FLAG_SHFT | SCANCODE_PERIOD,             // >
    FLAG_SHFT | SCANCODE_SLASH,              // ?
    FLAG_SHFT | SCANCODE_2,                  // @
    FLAG_SHFT | SCANCODE_A,                  // A
    FLAG_SHFT | SCANCODE_B,                  // B
    FLAG_SHFT | SCANCODE_C,                  // C
    FLAG_SHFT | SCANCODE_D,                  // D
    FLAG_SHFT | SCANCODE_E,                  // E
    FLAG_SHFT | SCANCODE_F,                  // F
    FLAG_SHFT | SCANCODE_G,                  // G
    FLAG_SHFT | SCANCODE_H,                  // H
    FLAG_SHFT | SCANCODE_I,                  // I
    FLAG_SHFT | SCANCODE_J,                  // J
    FLAG_SHFT | SCANCODE_K,                  // K
    FLAG_SHFT | SCANCODE_L,                  // L
    FLAG_SHFT | SCANCODE_M,                  // M
    FLAG_SHFT | SCANCODE_N,                  // N
    FLAG_SHFT | SCANCODE_O,                  // O
    FLAG_SHFT | SCANCODE_P,                  // P
    FLAG_SHFT | SCANCODE_Q,                  // Q
    FLAG_SHFT | SCANCODE_R,                  // R
    FLAG_SHFT | SCANCODE_S,                  // S
    FLAG_SHFT | SCANCODE_T,                  // T
    FLAG_SHFT | SCANCODE_U,                  // U
    FLAG_SHFT | SCANCODE_V,                  // V
    FLAG_SHFT | SCANCODE_W,                  // W
    FLAG_SHFT | SCANCODE_X,                  // X
    FLAG_SHFT | SCANCODE_Y,                  // Y
    FLAG_SHFT | SCANCODE_Z,                  // Z
    SCANCODE_LEFTBRACKET,                    // [
    SCANCODE_BACKSLASH,                      // backslash
    SCANCODE_RIGHTBRACKET,                   // ]
    FLAG_SHFT | SCANCODE_6,                  // ^
    FLAG_SHFT | SCANCODE_MINUS,              // _
    SCANCODE_GRAVE,                          // `
    SCANCODE_A,                              // a
    SCANCODE_B,                              // b
    SCANCODE_C,                              // c
    SCANCODE_D,                              // d
    SCANCODE_E,                              // e
    SCANCODE_F,                              // f
    SCANCODE_G,                              // g
    SCANCODE_H,                              // h
    SCANCODE_I,                              // i
    SCANCODE_J,                              // j
    SCANCODE_K,                              // k
    SCANCODE_L,                              // l
    SCANCODE_M,                              // m
    SCANCODE_N,                              // n
    SCANCODE_O,                              // o
    SCANCODE_P,                              // p
    SCANCODE_Q,                              // q
    SCANCODE_R,                              // r
    SCANCODE_S,                              // s
    SCANCODE_T,                              // t
    SCANCODE_U,                              // u
    SCANCODE_V,                              // v
    SCANCODE_W,                              // w
    SCANCODE_X,                              // x
    SCANCODE_Y,                              // y
    SCANCODE_Z,                              // z
    FLAG_SHFT | SCANCODE_LEFTBRACKET,        // {
    FLAG_SHFT | SCANCODE_BACKSLASH,          // |
    FLAG_SHFT | SCANCODE_RIGHTBRACKET,       // }
    FLAG_SHFT | SCANCODE_GRAVE,              // ~
};
