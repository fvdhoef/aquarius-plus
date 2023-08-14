#include "Config.h"
#include "cJSON.h"

Config::Config() {
}

Config &Config::instance() {
    static Config obj;
    return obj;
}

void Config::init(const std::string &_appDataPath) {
    appDataPath = _appDataPath;
    configPath  = appDataPath + "/config.json";
    load();
    save();
}

void Config::load() {
    std::ifstream ifs(configPath);
    if (!ifs.good())
        return;

    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    if (auto root = cJSON_ParseWithLength(str.c_str(), str.size())) {
        if (auto obj = cJSON_GetObjectItem(root, "imguiConfig")) {
            auto value = cJSON_GetStringValue(obj);
            if (value)
                imguiConf = value;
        }
        if (auto obj = cJSON_GetObjectItem(root, "sdCardPath")) {
            auto value = cJSON_GetStringValue(obj);
            if (value) {
                sdCardPath = value;
                printf("%s\n", sdCardPath.c_str());
            }
        }
        cJSON_free(root);
    }
}

void Config::save() {
    auto root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "imguiConfig", cJSON_CreateString(imguiConf.c_str()));
    cJSON_AddItemToObject(root, "sdCardPath", cJSON_CreateString(sdCardPath.c_str()));

    std::ofstream ofs(configPath);
    if (!ofs.good())
        return;

    auto str = cJSON_Print(root);
    ofs.write(str, strlen(str));
    cJSON_free(str);
    cJSON_Delete(root);
}
