#pragma once

#include "Common.h"
#include <SDL.h>

class Config {
    Config();

public:
    static Config &instance();

    void init(const std::string &appDataPath);

    void load();
    void save();

    std::string appDataPath;
    std::string configPath;
    std::string imguiConf;
    std::string sdCardPath;

    int  wndPosX     = SDL_WINDOWPOS_CENTERED;
    int  wndPosY     = SDL_WINDOWPOS_CENTERED;
    int  wndWidth    = VIDEO_WIDTH * 2;
    int  wndHeight   = VIDEO_HEIGHT * 2;
    int  scrScale    = 1;
    bool enableSound = true;

    bool showScreenWindow = false;
    bool showMemEdit      = false;
    bool showRegsWindow   = false;
    bool showBreakpoints  = false;
};
