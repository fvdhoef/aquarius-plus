#include <stdint.h>

extern uint8_t kb_pressing_prev[8];
extern uint8_t kb_pressing_keys[8];

void kb_scan(void);
bool kb_pressing(uint8_t scancode);
bool kb_pressed(uint8_t scancode);
bool kb_released(uint8_t scancode);
