#include "aqplus.h"

uint8_t kb_pressing_prev[8];
uint8_t kb_pressing_keys[8];

// Scan keyboard and store in kb_pressed_keys
void kb_scan(void) {
    for (uint8_t i = 0; i < 8; i++) {
        kb_pressing_prev[i] = kb_pressing_keys[i];
    }

    kb_pressing_keys[0] = ~IO_KEYBOARD_COL0;
    kb_pressing_keys[1] = ~IO_KEYBOARD_COL1;
    kb_pressing_keys[2] = ~IO_KEYBOARD_COL2;
    kb_pressing_keys[3] = ~IO_KEYBOARD_COL3;
    kb_pressing_keys[4] = ~IO_KEYBOARD_COL4;
    kb_pressing_keys[5] = ~IO_KEYBOARD_COL5;
    kb_pressing_keys[6] = ~IO_KEYBOARD_COL6;
    kb_pressing_keys[7] = ~IO_KEYBOARD_COL7;
}

// Check if key with specified scancode is pressed
bool kb_pressing(uint8_t scancode) {
    return (kb_pressing_keys[scancode / 8] & (1 << (scancode & 7))) != 0;
}

bool kb_pressed(uint8_t scancode) {
    return ((~kb_pressing_prev[scancode / 8] & kb_pressing_keys[scancode / 8]) & (1 << (scancode & 7))) != 0;
}

bool kb_released(uint8_t scancode) {
    return ((kb_pressing_prev[scancode / 8] & ~kb_pressing_keys[scancode / 8]) & (1 << (scancode & 7))) != 0;
}
