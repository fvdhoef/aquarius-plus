#include "Config.h"
#include "cJSON.h"
#include "EmuState.h"
#include "AqKeyboard.h"

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
    std::string jsonStr = "{}";

    std::ifstream ifs(configPath);
    if (ifs.good()) {
        jsonStr = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }

    if (auto root = cJSON_ParseWithLength(jsonStr.c_str(), jsonStr.size())) {
        imguiConf      = getStringValue(root, "imguiConfig", "");
        sdCardPath     = getStringValue(root, "sdCardPath", "");
        asmListingPath = getStringValue(root, "asmListingPath", "");

        wndPosX     = getIntValue(root, "wndPosX", SDL_WINDOWPOS_CENTERED);
        wndPosY     = getIntValue(root, "wndPosY", SDL_WINDOWPOS_CENTERED);
        wndWidth    = getIntValue(root, "wndWidth", VIDEO_WIDTH);
        wndHeight   = getIntValue(root, "wndHeight", VIDEO_HEIGHT * 2);
        scrScale    = getIntValue(root, "scrScale", 1);
        enableSound = getBoolValue(root, "enableSound", true);
        enableMouse = getBoolValue(root, "enableMouse", true);

        displayScaling = (DisplayScaling)getIntValue(root, "displayScaling", (int)DisplayScaling::Linear);

        setKeyLayout((KeyLayout)getIntValue(root, "keyLayout", 0));

        handCtrlEmulation = getBoolValue(root, "handCtrlEmulation", false);

        showScreenWindow    = getBoolValue(root, "showScreenWindow", false);
        showMemEdit         = getBoolValue(root, "showMemEdit", false);
        showCpuState        = getBoolValue(root, "showCpuState", false);
        showIoRegsWindow    = getBoolValue(root, "showIoRegsWindow", false);
        showBreakpoints     = getBoolValue(root, "showBreakpoints", false);
        showAssemblyListing = getBoolValue(root, "showAssemblyListing", false);
        showCpuTrace        = getBoolValue(root, "showCpuTrace", false);
        showEspInfo         = getBoolValue(root, "showEspInfo", false);

        memEditMemSelect = getIntValue(root, "memEditMemSelect", 0);

        auto breakpoints = cJSON_GetObjectItem(root, "breakpoints");
        if (cJSON_IsArray(breakpoints)) {
            cJSON *breakpoint;
            cJSON_ArrayForEach(breakpoint, breakpoints) {
                if (!cJSON_IsObject(breakpoint))
                    continue;

                EmuState::Breakpoint bp;
                bp.value   = getIntValue(breakpoint, "addr", 0);
                bp.enabled = getBoolValue(breakpoint, "enabled", false);
                bp.type    = getIntValue(breakpoint, "type", 0);
                bp.onR     = getBoolValue(breakpoint, "onR", false);
                bp.onW     = getBoolValue(breakpoint, "onW", false);
                bp.onX     = getBoolValue(breakpoint, "onX", false);
                emuState.breakpoints.push_back(bp);
            }
        }
        emuState.enableBreakpoints = getBoolValue(root, "enableBreakpoints", false);
        emuState.traceEnable       = getBoolValue(root, "traceEnable", false);
        emuState.traceDepth        = getIntValue(root, "traceDepth", 16);

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
    cJSON_AddStringToObject(root, "asmListingPath", asmListingPath.c_str());

    cJSON_AddNumberToObject(root, "wndPosX", wndPosX);
    cJSON_AddNumberToObject(root, "wndPosY", wndPosY);
    cJSON_AddNumberToObject(root, "wndWidth", wndWidth);
    cJSON_AddNumberToObject(root, "wndHeight", wndHeight);
    cJSON_AddNumberToObject(root, "scrScale", scrScale);
    cJSON_AddBoolToObject(root, "enableSound", enableSound);
    cJSON_AddBoolToObject(root, "enableMouse", enableMouse);

    cJSON_AddNumberToObject(root, "displayScaling", (int)displayScaling);

    cJSON_AddNumberToObject(root, "keyLayout", (int)getKeyLayout());

    cJSON_AddBoolToObject(root, "handCtrlEmulation", handCtrlEmulation);

    cJSON_AddBoolToObject(root, "showScreenWindow", showScreenWindow);
    cJSON_AddBoolToObject(root, "showMemEdit", showMemEdit);
    cJSON_AddBoolToObject(root, "showCpuState", showCpuState);
    cJSON_AddBoolToObject(root, "showIoRegsWindow", showIoRegsWindow);
    cJSON_AddBoolToObject(root, "showBreakpoints", showBreakpoints);
    cJSON_AddBoolToObject(root, "showAssemblyListing", showAssemblyListing);
    cJSON_AddBoolToObject(root, "showCpuTrace", showCpuTrace);
    cJSON_AddBoolToObject(root, "showEspInfo", showEspInfo);

    cJSON_AddNumberToObject(root, "memEditMemSelect", memEditMemSelect);

    cJSON_AddBoolToObject(root, "enableBreakpoints", emuState.enableBreakpoints);
    cJSON_AddBoolToObject(root, "traceEnable", emuState.traceEnable);
    cJSON_AddNumberToObject(root, "traceDepth", emuState.traceDepth);

    auto breakpoints = cJSON_AddArrayToObject(root, "breakpoints");
    for (auto &bp : emuState.breakpoints) {
        auto breakpoint = cJSON_CreateObject();

        cJSON_AddNumberToObject(breakpoint, "addr", bp.value);
        cJSON_AddBoolToObject(breakpoint, "enabled", bp.enabled);
        cJSON_AddNumberToObject(breakpoint, "type", bp.type);
        cJSON_AddBoolToObject(breakpoint, "onR", bp.onR);
        cJSON_AddBoolToObject(breakpoint, "onW", bp.onW);
        cJSON_AddBoolToObject(breakpoint, "onX", bp.onX);

        cJSON_AddItemToArray(breakpoints, breakpoint);
    }

    std::ofstream ofs(configPath);
    if (!ofs.good())
        return;

    auto str = cJSON_Print(root);
    ofs.write(str, strlen(str));
    cJSON_free(str);
    cJSON_Delete(root);
}
