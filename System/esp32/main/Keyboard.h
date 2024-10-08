#pragma once

#include "Common.h"

enum class KeyLayout {
    US = 0,
    UK = 1,
    FR = 2,
    DE = 3,
    Count,
};

class Keyboard {
public:
#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
    virtual void updateKeys(uint64_t keys) = 0;
#endif
    virtual void handleScancode(unsigned scanCode, bool keyDown) = 0;

    virtual int getKey(TickType_t ticksToWait) = 0;

    virtual void        setKeyLayout(KeyLayout layout)     = 0;
    virtual KeyLayout   getKeyLayout()                     = 0;
    virtual std::string getKeyLayoutName(KeyLayout layout) = 0;

    virtual void    setKeyMode(uint8_t mode) = 0;
    virtual uint8_t getKeyMode()             = 0;

    virtual void pressKey(uint8_t ch) = 0;
};

Keyboard *getKeyboard();

enum ScanCode {
    SCANCODE_UNKNOWN        = 0,
    SCANCODE_A              = 4,
    SCANCODE_B              = 5,
    SCANCODE_C              = 6,
    SCANCODE_D              = 7,
    SCANCODE_E              = 8,
    SCANCODE_F              = 9,
    SCANCODE_G              = 10,
    SCANCODE_H              = 11,
    SCANCODE_I              = 12,
    SCANCODE_J              = 13,
    SCANCODE_K              = 14,
    SCANCODE_L              = 15,
    SCANCODE_M              = 16,
    SCANCODE_N              = 17,
    SCANCODE_O              = 18,
    SCANCODE_P              = 19,
    SCANCODE_Q              = 20,
    SCANCODE_R              = 21,
    SCANCODE_S              = 22,
    SCANCODE_T              = 23,
    SCANCODE_U              = 24,
    SCANCODE_V              = 25,
    SCANCODE_W              = 26,
    SCANCODE_X              = 27,
    SCANCODE_Y              = 28,
    SCANCODE_Z              = 29,
    SCANCODE_1              = 30,
    SCANCODE_2              = 31,
    SCANCODE_3              = 32,
    SCANCODE_4              = 33,
    SCANCODE_5              = 34,
    SCANCODE_6              = 35,
    SCANCODE_7              = 36,
    SCANCODE_8              = 37,
    SCANCODE_9              = 38,
    SCANCODE_0              = 39,
    SCANCODE_RETURN         = 40,
    SCANCODE_ESCAPE         = 41,
    SCANCODE_BACKSPACE      = 42,
    SCANCODE_TAB            = 43,
    SCANCODE_SPACE          = 44,
    SCANCODE_MINUS          = 45,
    SCANCODE_EQUALS         = 46,
    SCANCODE_LEFTBRACKET    = 47,
    SCANCODE_RIGHTBRACKET   = 48,
    SCANCODE_BACKSLASH      = 49,
    SCANCODE_NONUSHASH      = 50,
    SCANCODE_SEMICOLON      = 51,
    SCANCODE_APOSTROPHE     = 52,
    SCANCODE_GRAVE          = 53,
    SCANCODE_COMMA          = 54,
    SCANCODE_PERIOD         = 55,
    SCANCODE_SLASH          = 56,
    SCANCODE_CAPSLOCK       = 57,
    SCANCODE_F1             = 58,
    SCANCODE_F2             = 59,
    SCANCODE_F3             = 60,
    SCANCODE_F4             = 61,
    SCANCODE_F5             = 62,
    SCANCODE_F6             = 63,
    SCANCODE_F7             = 64,
    SCANCODE_F8             = 65,
    SCANCODE_F9             = 66,
    SCANCODE_F10            = 67,
    SCANCODE_F11            = 68,
    SCANCODE_F12            = 69,
    SCANCODE_PRINTSCREEN    = 70,
    SCANCODE_SCROLLLOCK     = 71,
    SCANCODE_PAUSE          = 72,
    SCANCODE_INSERT         = 73,
    SCANCODE_HOME           = 74,
    SCANCODE_PAGEUP         = 75,
    SCANCODE_DELETE         = 76,
    SCANCODE_END            = 77,
    SCANCODE_PAGEDOWN       = 78,
    SCANCODE_RIGHT          = 79,
    SCANCODE_LEFT           = 80,
    SCANCODE_DOWN           = 81,
    SCANCODE_UP             = 82,
    SCANCODE_NUMLOCK        = 83,
    SCANCODE_KP_DIVIDE      = 84,
    SCANCODE_KP_MULTIPLY    = 85,
    SCANCODE_KP_MINUS       = 86,
    SCANCODE_KP_PLUS        = 87,
    SCANCODE_KP_ENTER       = 88,
    SCANCODE_KP_1           = 89,
    SCANCODE_KP_2           = 90,
    SCANCODE_KP_3           = 91,
    SCANCODE_KP_4           = 92,
    SCANCODE_KP_5           = 93,
    SCANCODE_KP_6           = 94,
    SCANCODE_KP_7           = 95,
    SCANCODE_KP_8           = 96,
    SCANCODE_KP_9           = 97,
    SCANCODE_KP_0           = 98,
    SCANCODE_KP_PERIOD      = 99,
    SCANCODE_NONUSBACKSLASH = 100,
    SCANCODE_APPLICATION    = 101,
    SCANCODE_LCTRL          = 224,
    SCANCODE_LSHIFT         = 225,
    SCANCODE_LALT           = 226,
    SCANCODE_LGUI           = 227,
    SCANCODE_RCTRL          = 228,
    SCANCODE_RSHIFT         = 229,
    SCANCODE_RALT           = 230,
    SCANCODE_RGUI           = 231,
    NUM_SCANCODES           = 512,
};

enum {
    ModLCtrl  = (1 << 0),
    ModLShift = (1 << 1),
    ModLAlt   = (1 << 2),
    ModLGui   = (1 << 3),
    ModRCtrl  = (1 << 4),
    ModRShift = (1 << 5),
    ModRAlt   = (1 << 6),
    ModRGui   = (1 << 7),
};
