#pragma once

#include "Common.h"

class UI {
public:
    static UI *instance();

    virtual void start(
        const std::string &cartRomPath,
        const std::string &typeInStr) = 0;
};
