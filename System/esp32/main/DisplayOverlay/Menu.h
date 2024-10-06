#pragma once

#include "Common.h"

enum class MenuItemType {
    none = 0,
    percentage,
    onOff,
    subMenu,
    separator,
};

struct Menu;

struct MenuItem {
    MenuItem(MenuItemType _type, const std::string &_name = "")
        : type(_type), name(_name) {
    }

    MenuItemType type;
    std::string  name;

    std::function<void(int newVal)> setter;
    std::function<int()>            getter;
    std::function<void()>           onEnter;
};

class Menu {
public:
    Menu(const std::string &_title = "", int _width = 30)
        : title(_title), width(_width) {
    }

    std::string           title;
    int                   width;
    std::vector<MenuItem> items;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual bool onTick() { return false; }
    virtual void onUpdate() {}

    virtual void show();

    static const unsigned colBg      = 11;
    static const unsigned colFg      = 8;
    static const unsigned colTitleFg = 7;
    static const unsigned colBorder  = 0;

    static const unsigned colSelectedBg = 8;
    static const unsigned colSelectedFg = 0;

    static const unsigned colSeparatorFg = 9; // 4;

    void setNeedsRedraw() { needsRedraw = true; }
    void setNeedsUpdate() { needsUpdate = true; }
    void resetSelectedRow() { selectedRow = 0; }
    void setExitMenu() { exitMenu = true; }

    void drawMessage(const char *msg);
    bool editString(const std::string &title, std::string &value, int maxLen);

protected:
    void draw();

    bool exitMenu    = false;
    bool needsRedraw = true;
    bool needsUpdate = true;
    int  selectedRow = 0;
    int  firstRow    = 0;
};
