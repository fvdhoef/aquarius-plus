#pragma once

#include "Common.h"

class DisplayOverlay {
public:
    virtual void init() = 0;

    virtual void clearScreen() = 0;
    virtual void drawBorder(
        int x, int y, int w, int h,
        unsigned colBorder, unsigned colFill,
        int selectedRow = -1, unsigned colFillSel = 0)                            = 0;
    virtual void drawStr(int x, int y, uint8_t attr, const char *str)             = 0;
    virtual void drawFmt(int x, int y, uint8_t attr, const char *fmt, ...)        = 0;
    virtual void fill(int x, int y, int w, int h, uint8_t attr, uint8_t ch = ' ') = 0;
    virtual void setAttr(int x, int y, uint8_t attr)                              = 0;

    virtual void setVisible(bool show) = 0;
    virtual bool isVisible()           = 0;

    virtual void render() = 0;

    static inline uint8_t makeAttr(unsigned fg, unsigned bg) {
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        return (fg << 4) | bg;
#else
        return (bg << 4) | fg;
#endif
    }
};

DisplayOverlay *getDisplayOverlay();
