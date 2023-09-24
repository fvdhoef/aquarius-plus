#include "KeyMaps.h"
#include "AqKeyboardDefs.h"

uint16_t keymapUS[] = {
    CAPS | KEY_A,         //   4: SCANCODE_A
    CAPS | KEY_B,         //   5: SCANCODE_B
    CAPS | KEY_C,         //   6: SCANCODE_C
    CAPS | KEY_D,         //   7: SCANCODE_D
    CAPS | KEY_E,         //   8: SCANCODE_E
    CAPS | KEY_F,         //   9: SCANCODE_F
    CAPS | KEY_G,         //  10: SCANCODE_G
    CAPS | KEY_H,         //  11: SCANCODE_H
    CAPS | KEY_I,         //  12: SCANCODE_I
    CAPS | KEY_J,         //  13: SCANCODE_J
    CAPS | KEY_K,         //  14: SCANCODE_K
    CAPS | KEY_L,         //  15: SCANCODE_L
    CAPS | KEY_M,         //  16: SCANCODE_M
    CAPS | KEY_N,         //  17: SCANCODE_N
    CAPS | KEY_O,         //  18: SCANCODE_O
    CAPS | KEY_P,         //  19: SCANCODE_P
    CAPS | KEY_Q,         //  20: SCANCODE_Q
    CAPS | KEY_R,         //  21: SCANCODE_R
    CAPS | KEY_S,         //  22: SCANCODE_S
    CAPS | KEY_T,         //  23: SCANCODE_T
    CAPS | KEY_U,         //  24: SCANCODE_U
    CAPS | KEY_V,         //  25: SCANCODE_V
    CAPS | KEY_W,         //  26: SCANCODE_W
    CAPS | KEY_X,         //  27: SCANCODE_X
    CAPS | KEY_Y,         //  28: SCANCODE_Y
    CAPS | KEY_Z,         //  29: SCANCODE_Z
    KEY_1,                //  30: SCANCODE_1
    KEY_2,                //  31: SCANCODE_2
    KEY_3,                //  32: SCANCODE_3
    KEY_4,                //  33: SCANCODE_4
    KEY_5,                //  34: SCANCODE_5
    KEY_6,                //  35: SCANCODE_6
    KEY_7,                //  36: SCANCODE_7
    KEY_8,                //  37: SCANCODE_8
    KEY_9,                //  38: SCANCODE_9
    KEY_0,                //  39: SCANCODE_0
    KEY_RETURN,           //  40: SCANCODE_RETURN
    CTRL | KEY_C,         //  41: SCANCODE_ESCAPE
    KEY_BACKSPACE,        //  42: SCANCODE_BACKSPACE
    KEY_TAB,              //  43: SCANCODE_TAB
    KEY_SPACE,            //  44: SCANCODE_SPACE
    KEY_MINUS,            //  45: SCANCODE_MINUS
    KEY_EQUALS,           //  46: SCANCODE_EQUALS
    CTRL | KEY_8,         //  47: SCANCODE_LEFTBRACKET
    CTRL | KEY_9,         //  48: SCANCODE_RIGHTBRACKET
    SHFT | KEY_BACKSPACE, //  49: SCANCODE_BACKSLASH
    SHFT | KEY_BACKSPACE, //  50: SCANCODE_NONUSHASH
    KEY_SEMICOLON,        //  51: SCANCODE_SEMICOLON
    SHFT | KEY_7,         //  52: SCANCODE_APOSTROPHE
    CTRL | KEY_7,         //  53: SCANCODE_GRAVE
    KEY_COMMA,            //  54: SCANCODE_COMMA
    KEY_PERIOD,           //  55: SCANCODE_PERIOD
    KEY_SLASH,            //  56: SCANCODE_SLASH
    0xFFFF,               //  57: SCANCODE_CAPSLOCK
    ALT | KEY_1,          //  58: SCANCODE_F1
    ALT | KEY_2,          //  59: SCANCODE_F2
    ALT | KEY_3,          //  60: SCANCODE_F3
    ALT | KEY_4,          //  61: SCANCODE_F4
    ALT | KEY_5,          //  62: SCANCODE_F5
    ALT | KEY_6,          //  63: SCANCODE_F6
    ALT | KEY_7,          //  64: SCANCODE_F7
    ALT | KEY_8,          //  65: SCANCODE_F8
    ALT | KEY_9,          //  66: SCANCODE_F9
    ALT | KEY_0,          //  67: SCANCODE_F10
    ALT | KEY_MINUS,      //  68: SCANCODE_F11
    ALT | KEY_EQUALS,     //  69: SCANCODE_F12
    KEY_PRTSCR,           //  70: SCANCODE_PRINTSCREEN
    0xFFFF,               //  71: SCANCODE_SCROLLLOCK
    KEY_PAUSE,            //  72: SCANCODE_PAUSE
    KEY_INSERT,           //  73: SCANCODE_INSERT
    KEY_HOME,             //  74: SCANCODE_HOME
    KEY_PGUP,             //  75: SCANCODE_PAGEUP
    KEY_DELETE,           //  76: SCANCODE_DELETE
    KEY_END,              //  77: SCANCODE_END
    KEY_PGDN,             //  78: SCANCODE_PAGEDOWN
    KEY_RIGHT,            //  79: SCANCODE_RIGHT
    KEY_LEFT,             //  80: SCANCODE_LEFT
    KEY_DOWN,             //  81: SCANCODE_DOWN
    KEY_UP,               //  82: SCANCODE_UP
    0xFFFF,               //  83: SCANCODE_NUMLOCKCLEAR
    KEY_SLASH,            //  84: SCANCODE_KP_DIVIDE
    SHFT | KEY_COLON,     //  85: SCANCODE_KP_MULTIPLY
    KEY_MINUS,            //  86: SCANCODE_KP_MINUS
    SHFT | KEY_EQUALS,    //  87: SCANCODE_KP_PLUS
    KEY_RETURN,           //  88: SCANCODE_KP_ENTER
    KEY_1,                //  89: SCANCODE_KP_1
    KEY_2,                //  90: SCANCODE_KP_2
    KEY_3,                //  91: SCANCODE_KP_3
    KEY_4,                //  92: SCANCODE_KP_4
    KEY_5,                //  93: SCANCODE_KP_5
    KEY_6,                //  94: SCANCODE_KP_6
    KEY_7,                //  95: SCANCODE_KP_7
    KEY_8,                //  96: SCANCODE_KP_8
    KEY_9,                //  97: SCANCODE_KP_9
    KEY_0,                //  98: SCANCODE_KP_0
    KEY_PERIOD,           //  99: SCANCODE_KP_PERIOD
    SHFT | KEY_BACKSPACE, // 100: SCANCODE_NONUSBACKSLASH
    KEY_MENU,             // 101: SCANCODE_APPLICATION
};

uint16_t keymapUS_shifted[] = {
    CAPS | SHFT | KEY_A,  //   4: SCANCODE_A
    CAPS | SHFT | KEY_B,  //   5: SCANCODE_B
    CAPS | SHFT | KEY_C,  //   6: SCANCODE_C
    CAPS | SHFT | KEY_D,  //   7: SCANCODE_D
    CAPS | SHFT | KEY_E,  //   8: SCANCODE_E
    CAPS | SHFT | KEY_F,  //   9: SCANCODE_F
    CAPS | SHFT | KEY_G,  //  10: SCANCODE_G
    CAPS | SHFT | KEY_H,  //  11: SCANCODE_H
    CAPS | SHFT | KEY_I,  //  12: SCANCODE_I
    CAPS | SHFT | KEY_J,  //  13: SCANCODE_J
    CAPS | SHFT | KEY_K,  //  14: SCANCODE_K
    CAPS | SHFT | KEY_L,  //  15: SCANCODE_L
    CAPS | SHFT | KEY_M,  //  16: SCANCODE_M
    CAPS | SHFT | KEY_N,  //  17: SCANCODE_N
    CAPS | SHFT | KEY_O,  //  18: SCANCODE_O
    CAPS | SHFT | KEY_P,  //  19: SCANCODE_P
    CAPS | SHFT | KEY_Q,  //  20: SCANCODE_Q
    CAPS | SHFT | KEY_R,  //  21: SCANCODE_R
    CAPS | SHFT | KEY_S,  //  22: SCANCODE_S
    CAPS | SHFT | KEY_T,  //  23: SCANCODE_T
    CAPS | SHFT | KEY_U,  //  24: SCANCODE_U
    CAPS | SHFT | KEY_V,  //  25: SCANCODE_V
    CAPS | SHFT | KEY_W,  //  26: SCANCODE_W
    CAPS | SHFT | KEY_X,  //  27: SCANCODE_X
    CAPS | SHFT | KEY_Y,  //  28: SCANCODE_Y
    CAPS | SHFT | KEY_Z,  //  29: SCANCODE_Z
    SHFT | KEY_1,         //  30: SCANCODE_1
    SHFT | KEY_SEMICOLON, //  31: SCANCODE_2
    SHFT | KEY_3,         //  32: SCANCODE_3
    SHFT | KEY_4,         //  33: SCANCODE_4
    SHFT | KEY_5,         //  34: SCANCODE_5
    SHFT | KEY_SLASH,     //  35: SCANCODE_6
    SHFT | KEY_6,         //  36: SCANCODE_7
    SHFT | KEY_COLON,     //  37: SCANCODE_8
    SHFT | KEY_8,         //  38: SCANCODE_9
    SHFT | KEY_9,         //  39: SCANCODE_0
    SHFT | KEY_RETURN,    //  40: SCANCODE_RETURN
    CTRL | KEY_C,         //  41: SCANCODE_ESCAPE
    KEY_BACKSPACE,        //  42: SCANCODE_BACKSPACE
    SHFT | KEY_TAB,       //  43: SCANCODE_TAB
    SHFT | KEY_SPACE,     //  44: SCANCODE_SPACE
    SHFT | KEY_MINUS,     //  45: SCANCODE_MINUS
    SHFT | KEY_EQUALS,    //  46: SCANCODE_EQUALS
    CTRL | KEY_COMMA,     //  47: SCANCODE_LEFTBRACKET
    CTRL | KEY_PERIOD,    //  48: SCANCODE_RIGHTBRACKET
    CTRL | KEY_1,         //  49: SCANCODE_BACKSLASH
    CTRL | KEY_1,         //  50: SCANCODE_NONUSHASH
    KEY_COLON,            //  51: SCANCODE_SEMICOLON
    SHFT | KEY_2,         //  52: SCANCODE_APOSTROPHE
    CTRL | KEY_2,         //  53: SCANCODE_GRAVE
    SHFT | KEY_COMMA,     //  54: SCANCODE_COMMA
    SHFT | KEY_PERIOD,    //  55: SCANCODE_PERIOD
    SHFT | KEY_0,         //  56: SCANCODE_SLASH
    0xFFFF,               //  57: SCANCODE_CAPSLOCK
    ALT | KEY_1,          //  58: SCANCODE_F1
    ALT | KEY_2,          //  59: SCANCODE_F2
    ALT | KEY_3,          //  60: SCANCODE_F3
    ALT | KEY_4,          //  61: SCANCODE_F4
    ALT | KEY_5,          //  62: SCANCODE_F5
    ALT | KEY_6,          //  63: SCANCODE_F6
    ALT | KEY_7,          //  64: SCANCODE_F7
    ALT | KEY_8,          //  65: SCANCODE_F8
    ALT | KEY_9,          //  66: SCANCODE_F9
    ALT | KEY_0,          //  67: SCANCODE_F10
    ALT | KEY_MINUS,      //  68: SCANCODE_F11
    ALT | KEY_EQUALS,     //  69: SCANCODE_F12
    SHFT | KEY_PRTSCR,    //  70: SCANCODE_PRINTSCREEN
    0xFFFF,               //  71: SCANCODE_SCROLLLOCK
    SHFT | KEY_PAUSE,     //  72: SCANCODE_PAUSE
    SHFT | KEY_INSERT,    //  73: SCANCODE_INSERT
    SHFT | KEY_HOME,      //  74: SCANCODE_HOME
    SHFT | KEY_PGUP,      //  75: SCANCODE_PAGEUP
    SHFT | KEY_DELETE,    //  76: SCANCODE_DELETE
    SHFT | KEY_END,       //  77: SCANCODE_END
    SHFT | KEY_PGDN,      //  78: SCANCODE_PAGEDOWN
    SHFT | KEY_RIGHT,     //  79: SCANCODE_RIGHT
    SHFT | KEY_LEFT,      //  80: SCANCODE_LEFT
    SHFT | KEY_DOWN,      //  81: SCANCODE_DOWN
    SHFT | KEY_UP,        //  82: SCANCODE_UP
    0xFFFF,               //  83: SCANCODE_NUMLOCKCLEAR
    KEY_SLASH,            //  84: SCANCODE_KP_DIVIDE
    SHFT | KEY_COLON,     //  85: SCANCODE_KP_MULTIPLY
    KEY_MINUS,            //  86: SCANCODE_KP_MINUS
    SHFT | KEY_EQUALS,    //  87: SCANCODE_KP_PLUS
    KEY_RETURN,           //  88: SCANCODE_KP_ENTER
    KEY_1,                //  89: SCANCODE_KP_1
    KEY_2,                //  90: SCANCODE_KP_2
    KEY_3,                //  91: SCANCODE_KP_3
    KEY_4,                //  92: SCANCODE_KP_4
    KEY_5,                //  93: SCANCODE_KP_5
    KEY_6,                //  94: SCANCODE_KP_6
    KEY_7,                //  95: SCANCODE_KP_7
    KEY_8,                //  96: SCANCODE_KP_8
    KEY_9,                //  97: SCANCODE_KP_9
    KEY_0,                //  98: SCANCODE_KP_0
    KEY_PERIOD,           //  99: SCANCODE_KP_PERIOD
    CTRL | KEY_1,         // 100: SCANCODE_NONUSBACKSLASH
    KEY_MENU,             // 101: SCANCODE_APPLICATION
};
