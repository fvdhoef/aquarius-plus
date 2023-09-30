#pragma once

#include "Common.h"

enum class KeyLayout {
    US = 0,
    UK = 1,
    Count,
};

#define CAPS (1 << 10)
#define CTRL (1 << 9)
#define ALT (1 << 8)
#define SHFT (1 << 7)
#define GUI (1 << 6)

void        setKeyLayout(KeyLayout layout);
KeyLayout   getKeyLayout();
std::string getKeyLayoutName(KeyLayout layout);
