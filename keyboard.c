#include "keyboard.h"
#include "common.h"
#include <SDL.h>
#include "emustate.h"

void keyboard_scancode(unsigned scancode, bool keydown) {
    // printf("%c: %d\n", keydown ? 'D' : 'U', scancode);

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

    int key = -1;
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

        case SDL_SCANCODE_ESCAPE:
            if (keydown)
                reset();
            break;

        case SDL_SCANCODE_EQUALS: key = 0; break;
        case SDL_SCANCODE_BACKSPACE: key = 1; break;
        case SDL_SCANCODE_APOSTROPHE: key = 2; break;
        case SDL_SCANCODE_RETURN: key = 3; break;
        case SDL_SCANCODE_SEMICOLON: key = 4; break;
        case SDL_SCANCODE_PERIOD: key = 5; break;

        case SDL_SCANCODE_MINUS: key = 6; break;
        case SDL_SCANCODE_SLASH: key = 7; break;
        case SDL_SCANCODE_0: key = 8; break;
        case SDL_SCANCODE_P: key = 9; break;
        case SDL_SCANCODE_L: key = 10; break;
        case SDL_SCANCODE_COMMA: key = 11; break;

        case SDL_SCANCODE_9: key = 12; break;
        case SDL_SCANCODE_O: key = 13; break;
        case SDL_SCANCODE_K: key = 14; break;
        case SDL_SCANCODE_M: key = 15; break;
        case SDL_SCANCODE_N: key = 16; break;
        case SDL_SCANCODE_J: key = 17; break;

        case SDL_SCANCODE_8: key = 18; break;
        case SDL_SCANCODE_I: key = 19; break;
        case SDL_SCANCODE_7: key = 20; break;
        case SDL_SCANCODE_U: key = 21; break;
        case SDL_SCANCODE_H: key = 22; break;
        case SDL_SCANCODE_B: key = 23; break;

        case SDL_SCANCODE_6: key = 24; break;
        case SDL_SCANCODE_Y: key = 25; break;
        case SDL_SCANCODE_G: key = 26; break;
        case SDL_SCANCODE_V: key = 27; break;
        case SDL_SCANCODE_C: key = 28; break;
        case SDL_SCANCODE_F: key = 29; break;

        case SDL_SCANCODE_5: key = 30; break;
        case SDL_SCANCODE_T: key = 31; break;
        case SDL_SCANCODE_4: key = 32; break;
        case SDL_SCANCODE_R: key = 33; break;
        case SDL_SCANCODE_D: key = 34; break;
        case SDL_SCANCODE_X: key = 35; break;

        case SDL_SCANCODE_3: key = 36; break;
        case SDL_SCANCODE_E: key = 37; break;
        case SDL_SCANCODE_S: key = 38; break;
        case SDL_SCANCODE_Z: key = 39; break;
        case SDL_SCANCODE_SPACE: key = 40; break;
        case SDL_SCANCODE_A: key = 41; break;

        case SDL_SCANCODE_2: key = 42; break;
        case SDL_SCANCODE_W: key = 43; break;
        case SDL_SCANCODE_1: key = 44; break;
        case SDL_SCANCODE_Q: key = 45; break;
        case SDL_SCANCODE_LSHIFT: key = 46; break;
        case SDL_SCANCODE_LCTRL: key = 47; break;
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

    if (key < 0) {
        return;
    }

    if (keydown) {
        emustate.keyb_matrix[key / 6] &= ~(1 << (key % 6));
    } else {
        emustate.keyb_matrix[key / 6] |= (1 << (key % 6));
    }
}
