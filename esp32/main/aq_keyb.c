#include "aq_keyb.h"
#include "aq_keyb_defs.h"

static const char *TAG = "keyboard";

static uint8_t keyb_matrix[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static inline void _aqkey_up(int key) {
    keyb_matrix[key / 6] |= (1 << (key % 6));
}
static inline void _aqkey_down(int key) {
    keyb_matrix[key / 6] &= ~(1 << (key % 6));
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
    if (keydown) {
        ESP_LOGI(TAG, "Key pressed:  %02X", scancode);
    } else {
        ESP_LOGI(TAG, "Key released: %02X", scancode);
    }

    // Hand controller emulation
    // handcontroller(scancode, keydown);

    // Reset key
    if (scancode == SDL_SCANCODE_ESCAPE && keydown) {
        // reset();
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
        keyb_matrix[i] = 0xFF;
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

void keyboard_update_matrix(void) {
    static uint8_t prev_matrix[8];
    if (memcmp(prev_matrix, keyb_matrix, 8) == 0) {
        return;
    }

    ESP_LOG_BUFFER_HEX(TAG, keyb_matrix, 8);

    memcpy(prev_matrix, keyb_matrix, 8);
}
