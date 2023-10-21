#pragma once

#include "Common.h"
#include <SDL.h>

enum class DisplayScaling {
    NearestNeighbor = 0,
    Linear,
    Integer,
};

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
    std::string asmListingPath;

    int  wndPosX     = SDL_WINDOWPOS_CENTERED;
    int  wndPosY     = SDL_WINDOWPOS_CENTERED;
    int  wndWidth    = VIDEO_WIDTH * 2;
    int  wndHeight   = (VIDEO_HEIGHT * 2) * 2;
    int  scrScale    = 1;
    bool enableSound = true;
    bool enableMouse = true;

    bool handCtrlEmulation = false;

    bool showScreenWindow    = false;
    bool showMemEdit         = false;
    bool showCpuState        = false;
    bool showIoRegsWindow    = false;
    bool showBreakpoints     = false;
    bool showAssemblyListing = false;
    bool showCpuTrace        = false;
    bool showEspInfo         = false;

    int memEditMemSelect = 0;

    DisplayScaling displayScaling = DisplayScaling::Linear;
};
