#include "AqKeyboard.h"
#include "AqKeyboardDefs.h"
#include "FPGA.h"
#include <esp_system.h>
#include "usbhost.h"

static const char *TAG = "keyboard";

enum {
    NUM_LOCK    = (1 << 0),
    CAPS_LOCK   = (1 << 1),
    SCROLL_LOCK = (1 << 2),
};

#define FLAG_SHFT (1 << 7)
#define FLAG_CTRL (1 << 6)

AqKeyboard::AqKeyboard() {
}

AqKeyboard &AqKeyboard::instance() {
    static AqKeyboard obj;
    return obj;
}

void AqKeyboard::init() {
    mutex = xSemaphoreCreateRecursiveMutex();
}

void AqKeyboard::keyUp(int key) {
    keybMatrix[key / 6] |= (1 << (key % 6));
}

void AqKeyboard::keyDown(int key) {
    keybMatrix[key / 6] &= ~(1 << (key % 6));
}

void AqKeyboard::keyDown(int key, bool shift) {
    keyDown(key);
    if (shift) {
        keyDown(KEY_SHIFT);
    } else {
        keyUp(KEY_SHIFT);
    }
}

void AqKeyboard::handleScancode(unsigned scancode, bool keydown) {
    RecursiveMutexLock lock(mutex);

    uint8_t ledStatusNext = ledStatus;

    // if (keydown) {
    //     ESP_LOGI(TAG, "Key pressed:  %02X", scancode);
    // } else {
    //     ESP_LOGI(TAG, "Key released: %02X", scancode);
    // }

    // Hand controller emulation
    handController(scancode, keydown);

    if (keydown && scancode == SDL_SCANCODE_CAPSLOCK) {
        ledStatusNext ^= CAPS_LOCK;
    }
    if (keydown && scancode == SDL_SCANCODE_NUMLOCKCLEAR) {
        ledStatusNext ^= NUM_LOCK;
    }
    if (keydown && scancode == SDL_SCANCODE_SCROLLLOCK) {
        ledStatusNext ^= SCROLL_LOCK;
    }

    // Keep track of pressed modifier keys
    static uint16_t modifiers = 0;
    if (scancode == SDL_SCANCODE_LSHIFT)
        modifiers = (modifiers & ~KMOD_LSHIFT) | (keydown ? KMOD_LSHIFT : 0);
    if (scancode == SDL_SCANCODE_RSHIFT)
        modifiers = (modifiers & ~KMOD_RSHIFT) | (keydown ? KMOD_RSHIFT : 0);
    if (scancode == SDL_SCANCODE_LCTRL)
        modifiers = (modifiers & ~KMOD_LCTRL) | (keydown ? KMOD_LCTRL : 0);
    if (scancode == SDL_SCANCODE_RCTRL)
        modifiers = (modifiers & ~KMOD_RCTRL) | (keydown ? KMOD_RCTRL : 0);
    if (scancode == SDL_SCANCODE_LALT)
        modifiers = (modifiers & ~KMOD_LALT) | (keydown ? KMOD_LALT : 0);
    if (scancode == SDL_SCANCODE_RALT)
        modifiers = (modifiers & ~KMOD_RALT) | (keydown ? KMOD_RALT : 0);
    if (scancode == SDL_SCANCODE_LGUI)
        modifiers = (modifiers & ~KMOD_LGUI) | (keydown ? KMOD_LGUI : 0);
    if (scancode == SDL_SCANCODE_RGUI)
        modifiers = (modifiers & ~KMOD_RGUI) | (keydown ? KMOD_RGUI : 0);

    bool ctrlPressed  = (modifiers & (KMOD_LCTRL | KMOD_RCTRL)) != 0;
    bool altPressed   = (modifiers & (KMOD_LALT | KMOD_RALT)) != 0;
    bool shiftPressed = (modifiers & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
    bool guiPressed   = (modifiers & (KMOD_LGUI | KMOD_RGUI)) != 0;

    // Handle caps lock
    if ((ledStatus & CAPS_LOCK) && !ctrlPressed && !altPressed && !guiPressed && scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
        shiftPressed = !shiftPressed;
    }

    // Handle keypad
    if (!ctrlPressed && !altPressed && !shiftPressed && !guiPressed) {
        switch (scancode) {
            case SDL_SCANCODE_KP_DIVIDE: scancode = SDL_SCANCODE_SLASH; break;
            case SDL_SCANCODE_KP_MULTIPLY:
                scancode     = SDL_SCANCODE_8;
                shiftPressed = true;
                break;
            case SDL_SCANCODE_KP_MINUS: scancode = SDL_SCANCODE_MINUS; break;
            case SDL_SCANCODE_KP_PLUS:
                scancode     = SDL_SCANCODE_EQUALS;
                shiftPressed = true;
                break;
            case SDL_SCANCODE_KP_ENTER: scancode = SDL_SCANCODE_RETURN; break;
        }

        // Handle num lock
        if (ledStatus & NUM_LOCK) {
            switch (scancode) {
                case SDL_SCANCODE_KP_1: scancode = SDL_SCANCODE_1; break;
                case SDL_SCANCODE_KP_2: scancode = SDL_SCANCODE_2; break;
                case SDL_SCANCODE_KP_3: scancode = SDL_SCANCODE_3; break;
                case SDL_SCANCODE_KP_4: scancode = SDL_SCANCODE_4; break;
                case SDL_SCANCODE_KP_5: scancode = SDL_SCANCODE_5; break;
                case SDL_SCANCODE_KP_6: scancode = SDL_SCANCODE_6; break;
                case SDL_SCANCODE_KP_7: scancode = SDL_SCANCODE_7; break;
                case SDL_SCANCODE_KP_8: scancode = SDL_SCANCODE_8; break;
                case SDL_SCANCODE_KP_9: scancode = SDL_SCANCODE_9; break;
                case SDL_SCANCODE_KP_0: scancode = SDL_SCANCODE_0; break;
                case SDL_SCANCODE_KP_PERIOD: scancode = SDL_SCANCODE_PERIOD; break;
            }
        }
    }

    // Keep track of pressed keys
    static uint8_t pressedKeys[8] = {0};
    if (scancode < 64) {
        if (keydown) {
            pressedKeys[scancode / 8] |= 1 << (scancode & 7);
        } else {
            pressedKeys[scancode / 8] &= ~(1 << (scancode & 7));
        }
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

    // Clear keyboard state
    for (int i = 0; i < 8; i++) {
        keybMatrix[i] = 0xFF;
    }

    // Set keyboard state based on currently pressed keys
    if (ctrlPressed)
        keyDown(KEY_CTRL);
    if (shiftPressed)
        keyDown(KEY_SHIFT);

    for (int i = 0; i < 64; i++) {
        if (pressedKeys[i / 8] & (1 << (i & 7))) {
            switch (i) {
                case SDL_SCANCODE_ESCAPE:
                    if (ctrlPressed && shiftPressed) {
                        // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                        esp_restart();
                    } else if (ctrlPressed) {
                        // CTRL-ESCAPE -> reset
                        FPGA::instance().aqpReset();
                    } else {
                        // ESCAPE -> CTRL-C
                        keyDown(KEY_CTRL);
                        keyDown(KEY_C);
                    }
                    break;

                case SDL_SCANCODE_RETURN:
                    keyDown(KEY_RETURN, shiftPressed);
                    break;

                case SDL_SCANCODE_1: keyDown(KEY_1, shiftPressed); break;
                case SDL_SCANCODE_2:
                    if (!shiftPressed)
                        keyDown(KEY_2, false);
                    else
                        keyDown(KEY_SEMICOLON, true);
                    break;
                case SDL_SCANCODE_3: keyDown(KEY_3, shiftPressed); break;
                case SDL_SCANCODE_4: keyDown(KEY_4, shiftPressed); break;
                case SDL_SCANCODE_5: keyDown(KEY_5, shiftPressed); break;
                case SDL_SCANCODE_6:
                    if (!shiftPressed)
                        keyDown(KEY_6, false);
                    else
                        keyDown(KEY_SLASH, true);
                    break;
                case SDL_SCANCODE_7:
                    if (!shiftPressed)
                        keyDown(KEY_7, false);
                    else
                        keyDown(KEY_6, true);
                    break;
                case SDL_SCANCODE_8:
                    if (!shiftPressed)
                        keyDown(KEY_8, false);
                    else
                        keyDown(KEY_COLON, true);
                    break;
                case SDL_SCANCODE_9:
                    if (!shiftPressed)
                        keyDown(KEY_9, false);
                    else
                        keyDown(KEY_8, true);
                    break;
                case SDL_SCANCODE_0:
                    if (!shiftPressed)
                        keyDown(KEY_0, false);
                    else
                        keyDown(KEY_9, true);
                    break;
                case SDL_SCANCODE_MINUS: keyDown(KEY_MINUS, shiftPressed); break;
                case SDL_SCANCODE_EQUALS: keyDown(KEY_EQUALS, shiftPressed); break;
                case SDL_SCANCODE_BACKSPACE: keyDown(KEY_BACKSPACE, false); break;

                case SDL_SCANCODE_Q: keyDown(KEY_Q, shiftPressed); break;
                case SDL_SCANCODE_W: keyDown(KEY_W, shiftPressed); break;
                case SDL_SCANCODE_E: keyDown(KEY_E, shiftPressed); break;
                case SDL_SCANCODE_R: keyDown(KEY_R, shiftPressed); break;
                case SDL_SCANCODE_T: keyDown(KEY_T, shiftPressed); break;
                case SDL_SCANCODE_Y: keyDown(KEY_Y, shiftPressed); break;
                case SDL_SCANCODE_U: keyDown(KEY_U, shiftPressed); break;
                case SDL_SCANCODE_I: keyDown(KEY_I, shiftPressed); break;
                case SDL_SCANCODE_O: keyDown(KEY_O, shiftPressed); break;
                case SDL_SCANCODE_P: keyDown(KEY_P, shiftPressed); break;

                case SDL_SCANCODE_A: keyDown(KEY_A, shiftPressed); break;
                case SDL_SCANCODE_S: keyDown(KEY_S, shiftPressed); break;
                case SDL_SCANCODE_D: keyDown(KEY_D, shiftPressed); break;
                case SDL_SCANCODE_F: keyDown(KEY_F, shiftPressed); break;
                case SDL_SCANCODE_G: keyDown(KEY_G, shiftPressed); break;
                case SDL_SCANCODE_H: keyDown(KEY_H, shiftPressed); break;
                case SDL_SCANCODE_J: keyDown(KEY_J, shiftPressed); break;
                case SDL_SCANCODE_K: keyDown(KEY_K, shiftPressed); break;
                case SDL_SCANCODE_L: keyDown(KEY_L, shiftPressed); break;
                case SDL_SCANCODE_SEMICOLON:
                    if (!shiftPressed)
                        keyDown(KEY_SEMICOLON, false);
                    else
                        keyDown(KEY_COLON, false);
                    break;
                case SDL_SCANCODE_APOSTROPHE:
                    if (!shiftPressed)
                        keyDown(KEY_7, true);
                    else
                        keyDown(KEY_2, true);
                    break;
                case SDL_SCANCODE_BACKSLASH:
                    if (!shiftPressed)
                        keyDown(KEY_BACKSPACE, true);
                    break;

                case SDL_SCANCODE_Z: keyDown(KEY_Z, shiftPressed); break;
                case SDL_SCANCODE_X: keyDown(KEY_X, shiftPressed); break;
                case SDL_SCANCODE_C: keyDown(KEY_C, shiftPressed); break;
                case SDL_SCANCODE_V: keyDown(KEY_V, shiftPressed); break;
                case SDL_SCANCODE_B: keyDown(KEY_B, shiftPressed); break;
                case SDL_SCANCODE_N: keyDown(KEY_N, shiftPressed); break;
                case SDL_SCANCODE_M: keyDown(KEY_M, shiftPressed); break;
                case SDL_SCANCODE_COMMA: keyDown(KEY_COMMA, shiftPressed); break;
                case SDL_SCANCODE_PERIOD: keyDown(KEY_PERIOD, shiftPressed); break;
                case SDL_SCANCODE_SLASH:
                    if (!shiftPressed)
                        keyDown(KEY_SLASH, false);
                    else
                        keyDown(KEY_0, true);
                    break;

                case SDL_SCANCODE_SPACE:
                    keyDown(KEY_SPACE, shiftPressed);
                    break;
            }
        }
    }

    if (ledStatus != ledStatusNext) {
        ledStatus = ledStatusNext;
        keyboard_set_leds(ledStatus);
    }
}

void AqKeyboard::handController(unsigned scancode, bool keydown) {
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

    static int handCtrlPressed = 0;

    switch (scancode) {
        case SDL_SCANCODE_UP: handCtrlPressed = (keydown) ? (handCtrlPressed | UP) : (handCtrlPressed & ~UP); break;
        case SDL_SCANCODE_DOWN: handCtrlPressed = (keydown) ? (handCtrlPressed | DOWN) : (handCtrlPressed & ~DOWN); break;
        case SDL_SCANCODE_LEFT: handCtrlPressed = (keydown) ? (handCtrlPressed | LEFT) : (handCtrlPressed & ~LEFT); break;
        case SDL_SCANCODE_RIGHT: handCtrlPressed = (keydown) ? (handCtrlPressed | RIGHT) : (handCtrlPressed & ~RIGHT); break;
        case SDL_SCANCODE_F1: handCtrlPressed = (keydown) ? (handCtrlPressed | K1) : (handCtrlPressed & ~K1); break;
        case SDL_SCANCODE_F2: handCtrlPressed = (keydown) ? (handCtrlPressed | K2) : (handCtrlPressed & ~K2); break;
        case SDL_SCANCODE_F3: handCtrlPressed = (keydown) ? (handCtrlPressed | K3) : (handCtrlPressed & ~K3); break;
        case SDL_SCANCODE_F4: handCtrlPressed = (keydown) ? (handCtrlPressed | K4) : (handCtrlPressed & ~K4); break;
        case SDL_SCANCODE_F5: handCtrlPressed = (keydown) ? (handCtrlPressed | K5) : (handCtrlPressed & ~K5); break;
        case SDL_SCANCODE_F6: handCtrlPressed = (keydown) ? (handCtrlPressed | K6) : (handCtrlPressed & ~K6); break;
    }

    handCtrl1 = 0xFF;
    switch (handCtrlPressed & 0xF) {
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
    if (handCtrlPressed & K1)
        handCtrl1 &= ~(1 << 6);
    if (handCtrlPressed & K2)
        handCtrl1 &= ~((1 << 7) | (1 << 2));
    if (handCtrlPressed & K3)
        handCtrl1 &= ~((1 << 7) | (1 << 5));
    if (handCtrlPressed & K4)
        handCtrl1 &= ~(1 << 5);
    if (handCtrlPressed & K5)
        handCtrl1 &= ~((1 << 7) | (1 << 1));
    if (handCtrlPressed & K6)
        handCtrl1 &= ~((1 << 7) | (1 << 0));
}

void AqKeyboard::updateMatrix() {
    RecursiveMutexLock lock(mutex);

    static uint8_t prev_matrix[8];
    if (memcmp(prev_matrix, keybMatrix, 8) != 0) {
        FPGA::instance().aqpUpdateKeybMatrix(keybMatrix);
        memcpy(prev_matrix, keybMatrix, 8);
    }

    static uint8_t prev_handctrl1;
    static uint8_t prev_handctrl2;
    if (prev_handctrl1 != handCtrl1 || prev_handctrl2 != handCtrl2) {
        FPGA::instance().aqpUpdateHandCtrl(handCtrl1, handCtrl2);
        prev_handctrl1 = handCtrl1;
        prev_handctrl2 = handCtrl2;
    }
}

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
        handleScancode(SDL_SCANCODE_LSHIFT, true);
    if (val & FLAG_CTRL)
        handleScancode(SDL_SCANCODE_LCTRL, true);
    handleScancode(val & 0x3F, true);
    updateMatrix();
    vTaskDelay(pdMS_TO_TICKS(20));

    if (val & FLAG_SHFT)
        handleScancode(SDL_SCANCODE_LSHIFT, false);
    if (val & FLAG_CTRL)
        handleScancode(SDL_SCANCODE_LCTRL, false);
    handleScancode(val & 0x3F, false);
    updateMatrix();
    vTaskDelay(pdMS_TO_TICKS(20));

    // Delay a little longer on reset
    if (ch == 0x1E)
        vTaskDelay(pdMS_TO_TICKS(500));
}

const uint8_t AqKeyboard::scanCodeLut[] = {
    0,                                           //  0 CTRL-@
    0,                                           //  1 CTRL-A
    0,                                           //  2 CTRL-B
    FLAG_CTRL | SDL_SCANCODE_C,                  //  3 CTRL-C
    0,                                           //  4 CTRL-D
    0,                                           //  5 CTRL-E
    0,                                           //  6 CTRL-F
    FLAG_CTRL | SDL_SCANCODE_G,                  //  7 CTRL-G
    0,                                           //  8 CTRL-H
    0,                                           //  9 CTRL-I
    SDL_SCANCODE_RETURN,                         // 10 CTRL-J \n
    0,                                           // 11 CTRL-K
    0,                                           // 12 CTRL-L
    0,                                           // 13 CTRL-M \r
    0,                                           // 14 CTRL-N
    0,                                           // 15 CTRL-O
    0,                                           // 16 CTRL-P
    0,                                           // 17 CTRL-Q
    0,                                           // 18 CTRL-R
    0,                                           // 19 CTRL-S
    0,                                           // 20 CTRL-T
    0,                                           // 21 CTRL-U
    0,                                           // 22 CTRL-V
    0,                                           // 23 CTRL-W
    0,                                           // 24 CTRL-X
    0,                                           // 25 CTRL-Y
    0,                                           // 26 CTRL-Z
    0,                                           // 27 CTRL-[
    0,                                           // \x1C 28 CTRL-backslash
    0,                                           // \x1D 29 CTRL-]
    FLAG_CTRL | SDL_SCANCODE_ESCAPE,             // \x1E 30 CTRL-^
    FLAG_CTRL | FLAG_SHFT | SDL_SCANCODE_ESCAPE, // \x1F 31 CTRL-_
    SDL_SCANCODE_SPACE,                          // Space
    FLAG_SHFT | SDL_SCANCODE_1,                  // !
    FLAG_SHFT | SDL_SCANCODE_APOSTROPHE,         // "
    FLAG_SHFT | SDL_SCANCODE_3,                  // #
    FLAG_SHFT | SDL_SCANCODE_4,                  // $
    FLAG_SHFT | SDL_SCANCODE_5,                  // %
    FLAG_SHFT | SDL_SCANCODE_7,                  // &
    SDL_SCANCODE_APOSTROPHE,                     // '
    FLAG_SHFT | SDL_SCANCODE_9,                  // (
    FLAG_SHFT | SDL_SCANCODE_0,                  // )
    FLAG_SHFT | SDL_SCANCODE_8,                  // *
    FLAG_SHFT | SDL_SCANCODE_EQUALS,             // +
    SDL_SCANCODE_COMMA,                          // ,
    SDL_SCANCODE_MINUS,                          // -
    SDL_SCANCODE_PERIOD,                         // .
    SDL_SCANCODE_SLASH,                          // /
    SDL_SCANCODE_0,                              // 0
    SDL_SCANCODE_1,                              // 1
    SDL_SCANCODE_2,                              // 2
    SDL_SCANCODE_3,                              // 3
    SDL_SCANCODE_4,                              // 4
    SDL_SCANCODE_5,                              // 5
    SDL_SCANCODE_6,                              // 6
    SDL_SCANCODE_7,                              // 7
    SDL_SCANCODE_8,                              // 8
    SDL_SCANCODE_9,                              // 9
    FLAG_SHFT | SDL_SCANCODE_SEMICOLON,          // :
    SDL_SCANCODE_SEMICOLON,                      // ;
    FLAG_SHFT | SDL_SCANCODE_COMMA,              // <
    SDL_SCANCODE_EQUALS,                         // =
    FLAG_SHFT | SDL_SCANCODE_PERIOD,             // >
    FLAG_SHFT | SDL_SCANCODE_SLASH,              // ?
    FLAG_SHFT | SDL_SCANCODE_2,                  // @
    FLAG_SHFT | SDL_SCANCODE_A,                  // A
    FLAG_SHFT | SDL_SCANCODE_B,                  // B
    FLAG_SHFT | SDL_SCANCODE_C,                  // C
    FLAG_SHFT | SDL_SCANCODE_D,                  // D
    FLAG_SHFT | SDL_SCANCODE_E,                  // E
    FLAG_SHFT | SDL_SCANCODE_F,                  // F
    FLAG_SHFT | SDL_SCANCODE_G,                  // G
    FLAG_SHFT | SDL_SCANCODE_H,                  // H
    FLAG_SHFT | SDL_SCANCODE_I,                  // I
    FLAG_SHFT | SDL_SCANCODE_J,                  // J
    FLAG_SHFT | SDL_SCANCODE_K,                  // K
    FLAG_SHFT | SDL_SCANCODE_L,                  // L
    FLAG_SHFT | SDL_SCANCODE_M,                  // M
    FLAG_SHFT | SDL_SCANCODE_N,                  // N
    FLAG_SHFT | SDL_SCANCODE_O,                  // O
    FLAG_SHFT | SDL_SCANCODE_P,                  // P
    FLAG_SHFT | SDL_SCANCODE_Q,                  // Q
    FLAG_SHFT | SDL_SCANCODE_R,                  // R
    FLAG_SHFT | SDL_SCANCODE_S,                  // S
    FLAG_SHFT | SDL_SCANCODE_T,                  // T
    FLAG_SHFT | SDL_SCANCODE_U,                  // U
    FLAG_SHFT | SDL_SCANCODE_V,                  // V
    FLAG_SHFT | SDL_SCANCODE_W,                  // W
    FLAG_SHFT | SDL_SCANCODE_X,                  // X
    FLAG_SHFT | SDL_SCANCODE_Y,                  // Y
    FLAG_SHFT | SDL_SCANCODE_Z,                  // Z
    SDL_SCANCODE_LEFTBRACKET,                    // [
    SDL_SCANCODE_BACKSLASH,                      // backslash
    SDL_SCANCODE_RIGHTBRACKET,                   // ]
    FLAG_SHFT | SDL_SCANCODE_6,                  // ^
    FLAG_SHFT | SDL_SCANCODE_MINUS,              // _
    SDL_SCANCODE_GRAVE,                          // `
    SDL_SCANCODE_A,                              // a
    SDL_SCANCODE_B,                              // b
    SDL_SCANCODE_C,                              // c
    SDL_SCANCODE_D,                              // d
    SDL_SCANCODE_E,                              // e
    SDL_SCANCODE_F,                              // f
    SDL_SCANCODE_G,                              // g
    SDL_SCANCODE_H,                              // h
    SDL_SCANCODE_I,                              // i
    SDL_SCANCODE_J,                              // j
    SDL_SCANCODE_K,                              // k
    SDL_SCANCODE_L,                              // l
    SDL_SCANCODE_M,                              // m
    SDL_SCANCODE_N,                              // n
    SDL_SCANCODE_O,                              // o
    SDL_SCANCODE_P,                              // p
    SDL_SCANCODE_Q,                              // q
    SDL_SCANCODE_R,                              // r
    SDL_SCANCODE_S,                              // s
    SDL_SCANCODE_T,                              // t
    SDL_SCANCODE_U,                              // u
    SDL_SCANCODE_V,                              // v
    SDL_SCANCODE_W,                              // w
    SDL_SCANCODE_X,                              // x
    SDL_SCANCODE_Y,                              // y
    SDL_SCANCODE_Z,                              // z
    FLAG_SHFT | SDL_SCANCODE_LEFTBRACKET,        // {
    FLAG_SHFT | SDL_SCANCODE_BACKSLASH,          // |
    FLAG_SHFT | SDL_SCANCODE_RIGHTBRACKET,       // }
    FLAG_SHFT | SDL_SCANCODE_GRAVE,              // ~
};
