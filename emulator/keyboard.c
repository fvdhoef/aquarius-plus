#include "keyboard.h"
#include "common.h"
#include <SDL.h>
#include "emustate.h"

// Aquarius keys
enum {
    KEY_EQUALS    = 0,  // = +
    KEY_BACKSPACE = 1,  // BS Backslash
    KEY_COLON     = 2,  // : *
    KEY_RETURN    = 3,  // Return
    KEY_SEMICOLON = 4,  // ; @
    KEY_PERIOD    = 5,  // . >
    KEY_MINUS     = 6,  // - _
    KEY_SLASH     = 7,  // / ^
    KEY_0         = 8,  // 0 ?
    KEY_P         = 9,  // P
    KEY_L         = 10, // L
    KEY_COMMA     = 11, // , <
    KEY_9         = 12, // 9 )
    KEY_O         = 13, // O
    KEY_K         = 14, // K
    KEY_M         = 15, // M
    KEY_N         = 16, // N
    KEY_J         = 17, // J
    KEY_8         = 18, // 8 (
    KEY_I         = 19, // I
    KEY_7         = 20, // 7 '
    KEY_U         = 21, // U
    KEY_H         = 22, // H
    KEY_B         = 23, // B
    KEY_6         = 24, // 6 &
    KEY_Y         = 25, // Y
    KEY_G         = 26, // G
    KEY_V         = 27, // V
    KEY_C         = 28, // C
    KEY_F         = 29, // F
    KEY_5         = 30, // 5 %
    KEY_T         = 31, // T
    KEY_4         = 32, // 4 $
    KEY_R         = 33, // R
    KEY_D         = 34, // D
    KEY_X         = 35, // X
    KEY_3         = 36, // 3 #
    KEY_E         = 37, // E
    KEY_S         = 38, // S
    KEY_Z         = 39, // Z
    KEY_SPACE     = 40, // Space
    KEY_A         = 41, // A
    KEY_2         = 42, // 2 "
    KEY_W         = 43, // W
    KEY_1         = 44, // 1 !
    KEY_Q         = 45, // Q
    KEY_SHIFT     = 46, // Shift
    KEY_CTRL      = 47, // Ctrl
};

static void handcontroller(unsigned scancode, bool keydown) {
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

    static int handctrl_pressed = 0;

    switch (scancode) {
        case SDL_SCANCODE_UP: handctrl_pressed = (keydown) ? (handctrl_pressed | UP) : (handctrl_pressed & ~UP); break;
        case SDL_SCANCODE_DOWN: handctrl_pressed = (keydown) ? (handctrl_pressed | DOWN) : (handctrl_pressed & ~DOWN); break;
        case SDL_SCANCODE_LEFT: handctrl_pressed = (keydown) ? (handctrl_pressed | LEFT) : (handctrl_pressed & ~LEFT); break;
        case SDL_SCANCODE_RIGHT: handctrl_pressed = (keydown) ? (handctrl_pressed | RIGHT) : (handctrl_pressed & ~RIGHT); break;
        case SDL_SCANCODE_F1: handctrl_pressed = (keydown) ? (handctrl_pressed | K1) : (handctrl_pressed & ~K1); break;
        case SDL_SCANCODE_F2: handctrl_pressed = (keydown) ? (handctrl_pressed | K2) : (handctrl_pressed & ~K2); break;
        case SDL_SCANCODE_F3: handctrl_pressed = (keydown) ? (handctrl_pressed | K3) : (handctrl_pressed & ~K3); break;
        case SDL_SCANCODE_F4: handctrl_pressed = (keydown) ? (handctrl_pressed | K4) : (handctrl_pressed & ~K4); break;
        case SDL_SCANCODE_F5: handctrl_pressed = (keydown) ? (handctrl_pressed | K5) : (handctrl_pressed & ~K5); break;
        case SDL_SCANCODE_F6: handctrl_pressed = (keydown) ? (handctrl_pressed | K6) : (handctrl_pressed & ~K6); break;
    }

    emustate.handctrl1 = 0xFF;
    switch (handctrl_pressed & 0xF) {
        case LEFT: emustate.handctrl1 &= ~(1 << 3); break;
        case UP | LEFT: emustate.handctrl1 &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case UP: emustate.handctrl1 &= ~(1 << 2); break;
        case UP | RIGHT: emustate.handctrl1 &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case RIGHT: emustate.handctrl1 &= ~(1 << 1); break;
        case DOWN | RIGHT: emustate.handctrl1 &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case DOWN: emustate.handctrl1 &= ~(1 << 0); break;
        case DOWN | LEFT: emustate.handctrl1 &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        default: break;
    }
    if (handctrl_pressed & K1)
        emustate.handctrl1 &= ~(1 << 6);
    if (handctrl_pressed & K2)
        emustate.handctrl1 &= ~((1 << 7) | (1 << 2));
    if (handctrl_pressed & K3)
        emustate.handctrl1 &= ~((1 << 7) | (1 << 5));
    if (handctrl_pressed & K4)
        emustate.handctrl1 &= ~(1 << 5);
    if (handctrl_pressed & K5)
        emustate.handctrl1 &= ~((1 << 7) | (1 << 1));
    if (handctrl_pressed & K6)
        emustate.handctrl1 &= ~((1 << 7) | (1 << 0));
}

static inline void _aqkey_up(int key) {
    emustate.keyb_matrix[key / 6] |= (1 << (key % 6));
}
static inline void _aqkey_down(int key) {
    emustate.keyb_matrix[key / 6] &= ~(1 << (key % 6));
}

static inline void aqkey_down(int key, bool shift) {
    _aqkey_down(key);
    if (shift) {
        _aqkey_down(KEY_SHIFT);
    } else {
        _aqkey_up(KEY_SHIFT);
    }
}

void keyboard_scancode(unsigned scancode, bool keydown) {
    // Hand controller emulation
    handcontroller(scancode, keydown);

    // Reset key
    if (scancode == SDL_SCANCODE_ESCAPE && keydown) {
        reset();
    }

    // Handle modifier keys
    static uint16_t modifiers       = 0;
    static uint8_t  pressed_keys[8] = {0};

    if (scancode == SDL_SCANCODE_LSHIFT) {
        modifiers = (modifiers & ~KMOD_LSHIFT) | (keydown ? KMOD_LSHIFT : 0);
    }
    if (scancode == SDL_SCANCODE_RSHIFT) {
        modifiers = (modifiers & ~KMOD_RSHIFT) | (keydown ? KMOD_RSHIFT : 0);
    }
    if (scancode == SDL_SCANCODE_LCTRL) {
        modifiers = (modifiers & ~KMOD_LCTRL) | (keydown ? KMOD_LCTRL : 0);
    }
    if (scancode == SDL_SCANCODE_RCTRL) {
        modifiers = (modifiers & ~KMOD_RCTRL) | (keydown ? KMOD_RCTRL : 0);
    }

    if (scancode < 64) {
        if (keydown) {
            pressed_keys[scancode / 8] |= 1 << (scancode & 7);
        } else {
            pressed_keys[scancode / 8] &= ~(1 << (scancode & 7));
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
        emustate.keyb_matrix[i] = 0xFF;
    }

    // Set keyboard state based on current pressed keys
    if (modifiers & (KMOD_LCTRL | KMOD_RCTRL)) {
        _aqkey_down(KEY_CTRL);
    }

    bool shift_pressed = (modifiers & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
    if (shift_pressed) {
        _aqkey_down(KEY_SHIFT);
    }

    for (int i = 0; i < 64; i++) {
        if (pressed_keys[i / 8] & (1 << (i & 7))) {
            switch (i) {
                case SDL_SCANCODE_RETURN:
                    aqkey_down(KEY_RETURN, shift_pressed);
                    break;

                case SDL_SCANCODE_1: aqkey_down(KEY_1, shift_pressed); break;
                case SDL_SCANCODE_2:
                    if (!shift_pressed)
                        aqkey_down(KEY_2, false);
                    else
                        aqkey_down(KEY_SEMICOLON, true);
                    break;
                case SDL_SCANCODE_3: aqkey_down(KEY_3, shift_pressed); break;
                case SDL_SCANCODE_4: aqkey_down(KEY_4, shift_pressed); break;
                case SDL_SCANCODE_5: aqkey_down(KEY_5, shift_pressed); break;
                case SDL_SCANCODE_6:
                    if (!shift_pressed)
                        aqkey_down(KEY_6, false);
                    else
                        aqkey_down(KEY_SLASH, true);
                    break;
                case SDL_SCANCODE_7:
                    if (!shift_pressed)
                        aqkey_down(KEY_7, false);
                    else
                        aqkey_down(KEY_6, true);
                    break;
                case SDL_SCANCODE_8:
                    if (!shift_pressed)
                        aqkey_down(KEY_8, false);
                    else
                        aqkey_down(KEY_COLON, true);
                    break;
                case SDL_SCANCODE_9:
                    if (!shift_pressed)
                        aqkey_down(KEY_9, false);
                    else
                        aqkey_down(KEY_8, true);
                    break;
                case SDL_SCANCODE_0:
                    if (!shift_pressed)
                        aqkey_down(KEY_0, false);
                    else
                        aqkey_down(KEY_9, true);
                    break;
                case SDL_SCANCODE_MINUS: aqkey_down(KEY_MINUS, shift_pressed); break;
                case SDL_SCANCODE_EQUALS: aqkey_down(KEY_EQUALS, shift_pressed); break;
                case SDL_SCANCODE_BACKSPACE: aqkey_down(KEY_BACKSPACE, false); break;

                case SDL_SCANCODE_Q: aqkey_down(KEY_Q, shift_pressed); break;
                case SDL_SCANCODE_W: aqkey_down(KEY_W, shift_pressed); break;
                case SDL_SCANCODE_E: aqkey_down(KEY_E, shift_pressed); break;
                case SDL_SCANCODE_R: aqkey_down(KEY_R, shift_pressed); break;
                case SDL_SCANCODE_T: aqkey_down(KEY_T, shift_pressed); break;
                case SDL_SCANCODE_Y: aqkey_down(KEY_Y, shift_pressed); break;
                case SDL_SCANCODE_U: aqkey_down(KEY_U, shift_pressed); break;
                case SDL_SCANCODE_I: aqkey_down(KEY_I, shift_pressed); break;
                case SDL_SCANCODE_O: aqkey_down(KEY_O, shift_pressed); break;
                case SDL_SCANCODE_P: aqkey_down(KEY_P, shift_pressed); break;

                case SDL_SCANCODE_A: aqkey_down(KEY_A, shift_pressed); break;
                case SDL_SCANCODE_S: aqkey_down(KEY_S, shift_pressed); break;
                case SDL_SCANCODE_D: aqkey_down(KEY_D, shift_pressed); break;
                case SDL_SCANCODE_F: aqkey_down(KEY_F, shift_pressed); break;
                case SDL_SCANCODE_G: aqkey_down(KEY_G, shift_pressed); break;
                case SDL_SCANCODE_H: aqkey_down(KEY_H, shift_pressed); break;
                case SDL_SCANCODE_J: aqkey_down(KEY_J, shift_pressed); break;
                case SDL_SCANCODE_K: aqkey_down(KEY_K, shift_pressed); break;
                case SDL_SCANCODE_L: aqkey_down(KEY_L, shift_pressed); break;
                case SDL_SCANCODE_SEMICOLON:
                    if (!shift_pressed)
                        aqkey_down(KEY_SEMICOLON, false);
                    else
                        aqkey_down(KEY_COLON, false);
                    break;
                case SDL_SCANCODE_APOSTROPHE:
                    if (!shift_pressed)
                        aqkey_down(KEY_7, true);
                    else
                        aqkey_down(KEY_2, true);
                    break;
                case SDL_SCANCODE_BACKSLASH:
                    if (!shift_pressed)
                        aqkey_down(KEY_BACKSPACE, true);
                    break;

                case SDL_SCANCODE_Z: aqkey_down(KEY_Z, shift_pressed); break;
                case SDL_SCANCODE_X: aqkey_down(KEY_X, shift_pressed); break;
                case SDL_SCANCODE_C: aqkey_down(KEY_C, shift_pressed); break;
                case SDL_SCANCODE_V: aqkey_down(KEY_V, shift_pressed); break;
                case SDL_SCANCODE_B: aqkey_down(KEY_B, shift_pressed); break;
                case SDL_SCANCODE_N: aqkey_down(KEY_N, shift_pressed); break;
                case SDL_SCANCODE_M: aqkey_down(KEY_M, shift_pressed); break;
                case SDL_SCANCODE_COMMA: aqkey_down(KEY_COMMA, shift_pressed); break;
                case SDL_SCANCODE_PERIOD: aqkey_down(KEY_PERIOD, shift_pressed); break;
                case SDL_SCANCODE_SLASH:
                    if (!shift_pressed)
                        aqkey_down(KEY_SLASH, false);
                    else
                        aqkey_down(KEY_0, true);
                    break;

                case SDL_SCANCODE_SPACE:
                    aqkey_down(KEY_SPACE, shift_pressed);
                    break;
            }
        }
    }
}

static uint8_t scancodes[127] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, SDL_SCANCODE_RETURN, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    SDL_SCANCODE_SPACE, 0x80 | SDL_SCANCODE_1, 0x80 | SDL_SCANCODE_APOSTROPHE, 0x80 | SDL_SCANCODE_3,
    0x80 | SDL_SCANCODE_4, 0x80 | SDL_SCANCODE_5, 0x80 | SDL_SCANCODE_7, SDL_SCANCODE_APOSTROPHE,
    0x80 | SDL_SCANCODE_9, 0x80 | SDL_SCANCODE_0, 0x80 | SDL_SCANCODE_8, 0x80 | SDL_SCANCODE_EQUALS,
    SDL_SCANCODE_COMMA, SDL_SCANCODE_MINUS, SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH,
    SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7,
    SDL_SCANCODE_8, SDL_SCANCODE_9, 0x80 | SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_SEMICOLON,
    0x80 | SDL_SCANCODE_COMMA, SDL_SCANCODE_EQUALS, 0x80 | SDL_SCANCODE_PERIOD, 0x80 | SDL_SCANCODE_SLASH,
    0x80 | SDL_SCANCODE_2, 0x80 | SDL_SCANCODE_A, 0x80 | SDL_SCANCODE_B, 0x80 | SDL_SCANCODE_C,
    0x80 | SDL_SCANCODE_D, 0x80 | SDL_SCANCODE_E, 0x80 | SDL_SCANCODE_F, 0x80 | SDL_SCANCODE_G,
    0x80 | SDL_SCANCODE_H, 0x80 | SDL_SCANCODE_I, 0x80 | SDL_SCANCODE_J, 0x80 | SDL_SCANCODE_K,
    0x80 | SDL_SCANCODE_L, 0x80 | SDL_SCANCODE_M, 0x80 | SDL_SCANCODE_N, 0x80 | SDL_SCANCODE_O,
    0x80 | SDL_SCANCODE_P, 0x80 | SDL_SCANCODE_Q, 0x80 | SDL_SCANCODE_R, 0x80 | SDL_SCANCODE_S,
    0x80 | SDL_SCANCODE_T, 0x80 | SDL_SCANCODE_U, 0x80 | SDL_SCANCODE_V, 0x80 | SDL_SCANCODE_W,
    0x80 | SDL_SCANCODE_X, 0x80 | SDL_SCANCODE_Y, 0x80 | SDL_SCANCODE_Z, SDL_SCANCODE_LEFTBRACKET,
    SDL_SCANCODE_BACKSLASH, SDL_SCANCODE_RIGHTBRACKET, 0x80 | SDL_SCANCODE_6, 0x80 | SDL_SCANCODE_MINUS,

    SDL_SCANCODE_GRAVE, SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C,
    SDL_SCANCODE_D, SDL_SCANCODE_E, SDL_SCANCODE_F, SDL_SCANCODE_G,
    SDL_SCANCODE_H, SDL_SCANCODE_I, SDL_SCANCODE_J, SDL_SCANCODE_K,
    SDL_SCANCODE_L, SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O,
    SDL_SCANCODE_P, SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S,
    SDL_SCANCODE_T, SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W,
    SDL_SCANCODE_X, SDL_SCANCODE_Y, SDL_SCANCODE_Z, 0x80 | SDL_SCANCODE_LEFTBRACKET,
    0x80 | SDL_SCANCODE_BACKSLASH, 0x80 | SDL_SCANCODE_RIGHTBRACKET, 0x80 | SDL_SCANCODE_GRAVE};

void keyboard_char(unsigned char ch, bool keydown) {
    if (ch >= 127)
        return;

    int scancode = scancodes[ch];
    if (scancode == 0)
        return;

    if (scancode & 0x80) {
        keyboard_scancode(SDL_SCANCODE_LSHIFT, keydown);
    }
    keyboard_scancode(scancode & 0x7F, keydown);
}
