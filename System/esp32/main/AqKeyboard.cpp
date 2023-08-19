// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#include "AqKeyboard.h"
#include "AqKeyboardDefs.h"
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

void AqKeyboard::_keyUp(int key) {
    keybMatrix[key / 8] |= (1 << (key % 8));
}
void AqKeyboard::_keyDown(int key) {
    keybMatrix[key / 8] &= ~(1 << (key % 8));
}
void AqKeyboard::_keyDown(int key, bool shift) {
    _keyDown(key);
    if (shift) {
        _keyDown(KEY_SHIFT);
    } else {
        _keyUp(KEY_SHIFT);
    }
}

void AqKeyboard::handleScancode(unsigned scanCode, bool keyDown) {
    // printf("Scancode: %u %s\n", scanCode, keyDown ? "Down" : "Up");

#ifndef EMULATOR
    RecursiveMutexLock lock(mutex);
#endif
    uint8_t ledStatusNext = ledStatus == 0xFF ? 0 : ledStatus;

    // Hand controller emulation
    handController(scanCode, keyDown);

    if (keyDown && scanCode == SCANCODE_CAPSLOCK) {
        ledStatusNext ^= CAPS_LOCK;
    }
    if (keyDown && scanCode == SCANCODE_NUMLOCKCLEAR) {
        ledStatusNext ^= NUM_LOCK;
    }
    if (keyDown && scanCode == SCANCODE_SCROLLLOCK) {
        ledStatusNext ^= SCROLL_LOCK;
    }

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
    if (scanCode == SCANCODE_RGUI)
        modifiers = (modifiers & ~KEYMOD_RGUI) | (keyDown ? KEYMOD_RGUI : 0);

    bool ctrlPressed  = (modifiers & (KEYMOD_LCTRL | KEYMOD_RCTRL)) != 0;
    bool altPressed   = (modifiers & (KEYMOD_LALT | KEYMOD_RALT)) != 0;
    bool shiftPressed = (modifiers & (KEYMOD_LSHIFT | KEYMOD_RSHIFT)) != 0;
    bool guiPressed   = (modifiers & (KEYMOD_LGUI | KEYMOD_RGUI)) != 0;

    // Handle caps lock
    if ((ledStatus & CAPS_LOCK) && !ctrlPressed && !altPressed && !guiPressed && scanCode >= SCANCODE_A && scanCode <= SCANCODE_Z) {
        shiftPressed = !shiftPressed;
    }

    // Handle keypad
    if (!ctrlPressed && !altPressed && !shiftPressed && !guiPressed) {
        switch (scanCode) {
            case SCANCODE_KP_DIVIDE: scanCode = SCANCODE_SLASH; break;
            case SCANCODE_KP_MULTIPLY:
                scanCode     = SCANCODE_8;
                shiftPressed = true;
                break;
            case SCANCODE_KP_MINUS: scanCode = SCANCODE_MINUS; break;
            case SCANCODE_KP_PLUS:
                scanCode     = SCANCODE_EQUALS;
                shiftPressed = true;
                break;
            case SCANCODE_KP_ENTER: scanCode = SCANCODE_RETURN; break;
        }

        // Handle num lock
        if (ledStatus & NUM_LOCK) {
            switch (scanCode) {
                case SCANCODE_KP_1: scanCode = SCANCODE_1; break;
                case SCANCODE_KP_2: scanCode = SCANCODE_2; break;
                case SCANCODE_KP_3: scanCode = SCANCODE_3; break;
                case SCANCODE_KP_4: scanCode = SCANCODE_4; break;
                case SCANCODE_KP_5: scanCode = SCANCODE_5; break;
                case SCANCODE_KP_6: scanCode = SCANCODE_6; break;
                case SCANCODE_KP_7: scanCode = SCANCODE_7; break;
                case SCANCODE_KP_8: scanCode = SCANCODE_8; break;
                case SCANCODE_KP_9: scanCode = SCANCODE_9; break;
                case SCANCODE_KP_0: scanCode = SCANCODE_0; break;
                case SCANCODE_KP_PERIOD: scanCode = SCANCODE_PERIOD; break;
            }
        }
    }

    // Keep track of pressed keys
    if (scanCode < 128) {
        if (keyDown) {
            pressedKeys[scanCode / 8] |= 1 << (scanCode & 7);
        } else {
            pressedKeys[scanCode / 8] &= ~(1 << (scanCode & 7));
        }
    }

    // Check if any (non-modifier) keys are currently pressed
    bool anyPressed = false;
    for (unsigned i = 0; i < sizeof(pressedKeys); i++) {
        if (pressedKeys[i]) {
            anyPressed = true;
            break;
        }
    }
    if (!anyPressed)
        waitAllReleased = false;

#ifndef EMULATOR
    if (!waitAllReleased && keyDown) {
        if (guiPressed && scanCode == SCANCODE_F12) {
            MemDump::dumpCartridge();
            waitAllReleased = true;
        } else if (scanCode == SCANCODE_PRINTSCREEN) {
            MemDump::dumpScreen();
            waitAllReleased = true;
        }
    }
#endif

    // Don't allow shift state being changed while another key is being pressed
    if (prevShiftPressed != shiftPressed && anyPressed) {
        waitAllReleased = true;
    }
    prevShiftPressed = shiftPressed;

    // Clear keyboard state
    for (int i = 0; i < 8; i++) {
        keybMatrix[i] = 0xFF;
    }

    // Set keyboard state based on currently pressed keys
    if (ctrlPressed)
        _keyDown(KEY_CTRL);
    if (shiftPressed)
        _keyDown(KEY_SHIFT);
    if (altPressed)
        _keyDown(KEY_ALT);
    if (guiPressed)
        _keyDown(KEY_GUI);

    for (int i = 0; i < 128; i++) {
        if (pressedKeys[i / 8] & (1 << (i & 7))) {
            switch (i) {
                case SCANCODE_ESCAPE:
#ifndef EMULATOR
                    if (ctrlPressed && shiftPressed) {
                        // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                        esp_restart();
                    }
#endif

                    if (waitAllReleased)
                        break;

                    if (ctrlPressed) {
                        // CTRL-ESCAPE -> reset
                        waitAllReleased = true;
#ifdef EMULATOR
                        emuState.reset();
#else
                        FPGA::instance().aqpReset();
#endif
                    } else {
                        // ESCAPE -> CTRL-C
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_C);
                    }
                    break;

                case SCANCODE_RETURN:
                    _keyDown(KEY_RETURN, shiftPressed);
                    break;

                case SCANCODE_1: _keyDown(KEY_1, shiftPressed); break;
                case SCANCODE_2:
                    if (!shiftPressed)
                        _keyDown(KEY_2, false);
                    else
                        _keyDown(KEY_SEMICOLON, true);
                    break;
                case SCANCODE_3: _keyDown(KEY_3, shiftPressed); break;
                case SCANCODE_4: _keyDown(KEY_4, shiftPressed); break;
                case SCANCODE_5: _keyDown(KEY_5, shiftPressed); break;
                case SCANCODE_6:
                    if (!shiftPressed)
                        _keyDown(KEY_6, false);
                    else
                        _keyDown(KEY_SLASH, true);
                    break;
                case SCANCODE_7:
                    if (!shiftPressed)
                        _keyDown(KEY_7, false);
                    else
                        _keyDown(KEY_6, true);
                    break;
                case SCANCODE_8:
                    if (!shiftPressed)
                        _keyDown(KEY_8, false);
                    else
                        _keyDown(KEY_COLON, true);
                    break;
                case SCANCODE_9:
                    if (!shiftPressed)
                        _keyDown(KEY_9, false);
                    else
                        _keyDown(KEY_8, true);
                    break;
                case SCANCODE_0:
                    if (!shiftPressed)
                        _keyDown(KEY_0, false);
                    else
                        _keyDown(KEY_9, true);
                    break;
                case SCANCODE_MINUS: _keyDown(KEY_MINUS, shiftPressed); break;
                case SCANCODE_EQUALS: _keyDown(KEY_EQUALS, shiftPressed); break;
                case SCANCODE_BACKSPACE: _keyDown(KEY_BACKSPACE, false); break;

                case SCANCODE_Q: _keyDown(KEY_Q, shiftPressed); break;
                case SCANCODE_W: _keyDown(KEY_W, shiftPressed); break;
                case SCANCODE_E: _keyDown(KEY_E, shiftPressed); break;
                case SCANCODE_R: _keyDown(KEY_R, shiftPressed); break;
                case SCANCODE_T: _keyDown(KEY_T, shiftPressed); break;
                case SCANCODE_Y: _keyDown(KEY_Y, shiftPressed); break;
                case SCANCODE_U: _keyDown(KEY_U, shiftPressed); break;
                case SCANCODE_I: _keyDown(KEY_I, shiftPressed); break;
                case SCANCODE_O: _keyDown(KEY_O, shiftPressed); break;
                case SCANCODE_P: _keyDown(KEY_P, shiftPressed); break;

                case SCANCODE_A: _keyDown(KEY_A, shiftPressed); break;
                case SCANCODE_S: _keyDown(KEY_S, shiftPressed); break;
                case SCANCODE_D: _keyDown(KEY_D, shiftPressed); break;
                case SCANCODE_F: _keyDown(KEY_F, shiftPressed); break;
                case SCANCODE_G: _keyDown(KEY_G, shiftPressed); break;
                case SCANCODE_H: _keyDown(KEY_H, shiftPressed); break;
                case SCANCODE_J: _keyDown(KEY_J, shiftPressed); break;
                case SCANCODE_K: _keyDown(KEY_K, shiftPressed); break;
                case SCANCODE_L: _keyDown(KEY_L, shiftPressed); break;
                case SCANCODE_SEMICOLON:
                    if (!shiftPressed)
                        _keyDown(KEY_SEMICOLON, false);
                    else
                        _keyDown(KEY_COLON, false);
                    break;
                case SCANCODE_APOSTROPHE:
                    if (!shiftPressed)
                        _keyDown(KEY_7, true);
                    else
                        _keyDown(KEY_2, true);
                    break;

                case SCANCODE_NONUSBACKSLASH:
                case SCANCODE_NONUSHASH:
                case SCANCODE_BACKSLASH:
                    if (!shiftPressed) {
                        _keyDown(KEY_BACKSPACE, true);
                    } else {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_1, false);
                    }
                    break;

                case SCANCODE_Z: _keyDown(KEY_Z, shiftPressed); break;
                case SCANCODE_X: _keyDown(KEY_X, shiftPressed); break;
                case SCANCODE_C: _keyDown(KEY_C, shiftPressed); break;
                case SCANCODE_V: _keyDown(KEY_V, shiftPressed); break;
                case SCANCODE_B: _keyDown(KEY_B, shiftPressed); break;
                case SCANCODE_N: _keyDown(KEY_N, shiftPressed); break;
                case SCANCODE_M: _keyDown(KEY_M, shiftPressed); break;
                case SCANCODE_COMMA: _keyDown(KEY_COMMA, shiftPressed); break;
                case SCANCODE_PERIOD: _keyDown(KEY_PERIOD, shiftPressed); break;
                case SCANCODE_SLASH:
                    if (!shiftPressed)
                        _keyDown(KEY_SLASH, false);
                    else
                        _keyDown(KEY_0, true);
                    break;

                case SCANCODE_SPACE:
                    _keyDown(KEY_SPACE, shiftPressed);
                    break;

                case SCANCODE_LEFTBRACKET:
                    if (!shiftPressed) {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_8, false);
                    } else {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_COMMA, false);
                    }
                    break;

                case SCANCODE_RIGHTBRACKET:
                    if (!shiftPressed) {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_9, false);
                    } else {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_PERIOD, false);
                    }
                    break;

                case SCANCODE_GRAVE:
                    if (!shiftPressed) {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_7, false);
                    } else {
                        _keyDown(KEY_CTRL);
                        _keyDown(KEY_2, false);
                    }
                    break;

                case SCANCODE_INSERT: _keyDown(KEY_INSERT); break;
                case SCANCODE_DELETE: _keyDown(KEY_DELETE); break;
                case SCANCODE_UP: _keyDown(KEY_UP); break;
                case SCANCODE_RIGHT: _keyDown(KEY_RIGHT); break;
                case SCANCODE_LEFT: _keyDown(KEY_LEFT); break;
                case SCANCODE_DOWN: _keyDown(KEY_DOWN); break;
                case SCANCODE_HOME: _keyDown(KEY_HOME); break;
                case SCANCODE_END: _keyDown(KEY_END); break;
                case SCANCODE_PAGEUP: _keyDown(KEY_PGUP); break;
                case SCANCODE_PAGEDOWN: _keyDown(KEY_PGDN); break;
                case SCANCODE_PAUSE: _keyDown(KEY_PAUSE); break;
                case SCANCODE_PRINTSCREEN: _keyDown(KEY_PRTSCR); break;
                case SCANCODE_APPLICATION: _keyDown(KEY_MENU); break;
                case SCANCODE_TAB: _keyDown(KEY_TAB); break;

                case SCANCODE_F1:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_1);
                    break;
                case SCANCODE_F2:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_2);
                    break;
                case SCANCODE_F3:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_3);
                    break;
                case SCANCODE_F4:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_4);
                    break;
                case SCANCODE_F5:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_5);
                    break;
                case SCANCODE_F6:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_6);
                    break;
                case SCANCODE_F7:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_7);
                    break;
                case SCANCODE_F8:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_8);
                    break;
                case SCANCODE_F9:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_9);
                    break;
                case SCANCODE_F10:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_0);
                    break;
                case SCANCODE_F11:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_MINUS);
                    break;
                case SCANCODE_F12:
                    _keyDown(KEY_ALT);
                    _keyDown(KEY_EQUALS);
                    break;
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

void AqKeyboard::handController(unsigned scanCode, bool keyDown) {
    handCtrl1 = 0xFF;
    if ((ledStatus & SCROLL_LOCK) == 0) {
        handCtrl1Pressed = 0;
        return;
    }

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
}

void AqKeyboard::updateMatrix() {
#ifndef EMULATOR
    RecursiveMutexLock lock(mutex);
#endif
    if (!waitAllReleased) {
        if (memcmp(prevMatrix, keybMatrix, 8) != 0) {
#ifdef EMULATOR
            memcpy(emuState.keybMatrix, keybMatrix, 8);
#else
            FPGA::instance().aqpUpdateKeybMatrix(keybMatrix);
#endif
            memcpy(prevMatrix, keybMatrix, 8);
        }
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
