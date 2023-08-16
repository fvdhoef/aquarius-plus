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

static std::string getStringValue(cJSON *parent, const std::string &key, const std::string &defaultValue) {
    if (auto obj = cJSON_GetObjectItem(parent, key.c_str())) {
        auto value = cJSON_GetStringValue(obj);
        if (value)
            return value;
    }
    return defaultValue;
}

static bool getBoolValue(cJSON *parent, const std::string &key, bool defaultValue) {
    if (auto obj = cJSON_GetObjectItem(parent, key.c_str())) {
        if (cJSON_IsBool(obj)) {
            return cJSON_IsTrue(obj);
        }
    }
    return defaultValue;
}

static int getIntValue(cJSON *parent, const std::string &key, int defaultValue) {
    if (auto obj = cJSON_GetObjectItem(parent, key.c_str())) {
        if (cJSON_IsNumber(obj)) {
            return (int)cJSON_GetNumberValue(obj);
        }
    }
    return defaultValue;
}

void Config::load() {
    std::ifstream ifs(configPath);
    if (!ifs.good())
        return;

    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    if (auto root = cJSON_ParseWithLength(str.c_str(), str.size())) {
        imguiConf  = getStringValue(root, "imguiConfig", "");
        sdCardPath = getStringValue(root, "sdCardPath", "");

        wndPosX     = getIntValue(root, "wndPosX", VIDEO_WIDTH * 2);
        wndPosY     = getIntValue(root, "wndPosY", VIDEO_HEIGHT * 2);
        wndWidth    = getIntValue(root, "wndWidth", VIDEO_WIDTH * 2);
        wndHeight   = getIntValue(root, "wndHeight", VIDEO_HEIGHT * 2);
        scrScale    = getIntValue(root, "scrScale", 1);
        enableSound = getBoolValue(root, "enableSound", true);

        handCtrlEmulation = getBoolValue(root, "handCtrlEmulation", false);

        showScreenWindow = getBoolValue(root, "showScreenWindow", false);
        showMemEdit      = getBoolValue(root, "showMemEdit", false);
        showCpuState     = getBoolValue(root, "showCpuState", false);
        showIoRegsWindow = getBoolValue(root, "showIoRegsWindow", false);
        showBreakpoints  = getBoolValue(root, "showBreakpoints", false);

        cJSON_free(root);
    }

    // Sanitize some variables
    wndPosX   = std::max(wndPosX, 0);
    wndPosY   = std::max(wndPosY, 0);
    wndWidth  = std::max(wndWidth, VIDEO_WIDTH);
    wndHeight = std::max(wndHeight, VIDEO_HEIGHT);
}

void Config::save() {
    auto root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "imguiConfig", cJSON_CreateString(imguiConf.c_str()));
    cJSON_AddStringToObject(root, "sdCardPath", sdCardPath.c_str());

    cJSON_AddNumberToObject(root, "wndPosX", wndPosX);
    cJSON_AddNumberToObject(root, "wndPosY", wndPosY);
    cJSON_AddNumberToObject(root, "wndWidth", wndWidth);
    cJSON_AddNumberToObject(root, "wndHeight", wndHeight);
    cJSON_AddNumberToObject(root, "scrScale", scrScale);
    cJSON_AddBoolToObject(root, "enableSound", enableSound);

    cJSON_AddBoolToObject(root, "handCtrlEmulation", handCtrlEmulation);

    cJSON_AddBoolToObject(root, "showScreenWindow", showScreenWindow);
    cJSON_AddBoolToObject(root, "showMemEdit", showMemEdit);
    cJSON_AddBoolToObject(root, "showCpuState", showCpuState);
    cJSON_AddBoolToObject(root, "showIoRegsWindow", showIoRegsWindow);
    cJSON_AddBoolToObject(root, "showBreakpoints", showBreakpoints);

    std::ofstream ofs(configPath);
    if (!ofs.good())
        return;

    auto str = cJSON_Print(root);
    ofs.write(str, strlen(str));
    cJSON_free(str);
    cJSON_Delete(root);
}
