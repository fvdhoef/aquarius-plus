#include "KeyMaps.h"
#include "AqKeyboardDefs.h"

#ifdef EMULATOR
#    include "Config.h"
#endif

static KeyLayout curLayout = KeyLayout::US;

void setKeyLayout(KeyLayout layout) {
    curLayout = layout;
}

KeyLayout getKeyLayout() {
    return curLayout;
}

std::string getKeyLayoutName(KeyLayout layout) {
    switch (layout) {
        default: return "Unknown";
        case KeyLayout::US: return "US";
        case KeyLayout::UK: return "UK";
    }
}
