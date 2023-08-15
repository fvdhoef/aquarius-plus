#pragma once

#include "Common.h"

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
};
