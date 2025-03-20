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

    int  wndPosX             = 0;
    int  wndPosY             = 0;
    int  wndWidth            = 0;
    int  wndHeight           = 0;
    bool enableSound         = true;
    bool enableMouse         = true;
    bool showMemEdit         = false;
    bool showCpuState        = false;
    bool showIoRegsWindow    = false;
    bool showBreakpoints     = false;
    bool showAssemblyListing = false;
    bool showCpuTrace        = false;
    bool showWatch           = false;
    int  memEditMemSelect    = 0;

    DisplayScaling displayScaling = DisplayScaling::Linear;
};
