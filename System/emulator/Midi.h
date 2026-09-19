#pragma once

#include "Common.h"

class Midi {
    Midi();

public:
    static Midi *instance();
    void         init();
};
