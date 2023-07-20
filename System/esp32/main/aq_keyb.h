#pragma once

#include "common.h"

void keyboard_scancode(unsigned scancode, bool keydown);
void keyboard_update_matrix(void);

void keyboard_init(void);

void keyboard_press_key(unsigned ch);
