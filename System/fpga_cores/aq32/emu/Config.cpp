#include "Config.h"
#include "cJSON.h"
#include "EmuState.h"

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

static uint32_t getUInt32Value(cJSON *parent, const std::string &key, uint32_t defaultValue) {
    if (auto obj = cJSON_GetObjectItem(parent, key.c_str())) {
        if (cJSON_IsNumber(obj)) {
            return (uint32_t)cJSON_GetNumberValue(obj);
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

        wndPosX                 = getIntValue(root, "wndPosX", SDL_WINDOWPOS_CENTERED);
        wndPosY                 = getIntValue(root, "wndPosY", SDL_WINDOWPOS_CENTERED);
        wndWidth                = getIntValue(root, "wndWidth", VIDEO_WIDTH);
        wndHeight               = getIntValue(root, "wndHeight", VIDEO_HEIGHT);
        enableSound             = getBoolValue(root, "enableSound", true);
        enableMouse             = getBoolValue(root, "enableMouse", true);
        emuState.enableDebugger = getBoolValue(root, "enableDebugger", false);

        displayScaling = (DisplayScaling)getIntValue(root, "displayScaling", (int)DisplayScaling::Linear);

        showMemEdit         = getBoolValue(root, "showMemEdit", false);
        showCpuState        = getBoolValue(root, "showCpuState", false);
        showIoRegsWindow    = getBoolValue(root, "showIoRegsWindow", false);
        showBreakpoints     = getBoolValue(root, "showBreakpoints", false);
        showAssemblyListing = getBoolValue(root, "showAssemblyListing", false);
        showCpuTrace        = getBoolValue(root, "showCpuTrace", false);
        showWatch           = getBoolValue(root, "showWatch", false);

        memEditMemSelect = getIntValue(root, "memEditMemSelect", 0);
        // auto breakpoints = cJSON_GetObjectItem(root, "breakpoints");
        // if (cJSON_IsArray(breakpoints)) {
        //     cJSON *breakpoint;
        //     cJSON_ArrayForEach(breakpoint, breakpoints) {
        //         if (!cJSON_IsObject(breakpoint))
        //             continue;

        //         EmuState::Breakpoint bp;
        //         bp.addr    = getUInt32Value(breakpoint, "addr", 0);
        //         bp.name    = getStringValue(breakpoint, "name", "");
        //         bp.enabled = getBoolValue(breakpoint, "enabled", false);
        //         emuState.breakpoints.push_back(bp);
        //     }
        // }
        // emuState.enableBreakpoints = getBoolValue(root, "enableBreakpoints", false);
        // emuState.traceEnable       = getBoolValue(root, "traceEnable", false);
        // emuState.traceDepth        = getIntValue(root, "traceDepth", 16);

        // auto watches = cJSON_GetObjectItem(root, "watches");
        // if (cJSON_IsArray(watches)) {
        //     cJSON *watch;
        //     cJSON_ArrayForEach(watch, watches) {
        //         if (!cJSON_IsObject(watch))
        //             continue;

        //         EmuState::Watch w;
        //         w.addr = getIntValue(watch, "addr", 0);
        //         w.name = getStringValue(watch, "name", "");
        //         w.type = (EmuState::WatchType)getIntValue(watch, "type", 0);
        //         emuState.watches.push_back(w);
        //     }
        // }

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
    cJSON_AddBoolToObject(root, "enableSound", enableSound);
    cJSON_AddBoolToObject(root, "enableMouse", enableMouse);
    cJSON_AddBoolToObject(root, "enableDebugger", emuState.enableDebugger);

    cJSON_AddNumberToObject(root, "displayScaling", (int)displayScaling);

    cJSON_AddBoolToObject(root, "showMemEdit", showMemEdit);
    cJSON_AddBoolToObject(root, "showCpuState", showCpuState);
    cJSON_AddBoolToObject(root, "showIoRegsWindow", showIoRegsWindow);
    cJSON_AddBoolToObject(root, "showBreakpoints", showBreakpoints);
    cJSON_AddBoolToObject(root, "showAssemblyListing", showAssemblyListing);
    cJSON_AddBoolToObject(root, "showCpuTrace", showCpuTrace);
    cJSON_AddBoolToObject(root, "showWatch", showWatch);

    cJSON_AddNumberToObject(root, "memEditMemSelect", memEditMemSelect);

    // cJSON_AddBoolToObject(root, "enableBreakpoints", emuState.enableBreakpoints);
    // cJSON_AddBoolToObject(root, "traceEnable", emuState.traceEnable);
    // cJSON_AddNumberToObject(root, "traceDepth", emuState.traceDepth);

    // auto breakpoints = cJSON_AddArrayToObject(root, "breakpoints");
    // for (auto &bp : emuState.breakpoints) {
    //     auto breakpoint = cJSON_CreateObject();
    //     cJSON_AddNumberToObject(breakpoint, "addr", bp.addr);
    //     cJSON_AddStringToObject(breakpoint, "name", bp.name.c_str());
    //     cJSON_AddBoolToObject(breakpoint, "enabled", bp.enabled);
    //     cJSON_AddItemToArray(breakpoints, breakpoint);
    // }

    // auto watches = cJSON_AddArrayToObject(root, "watches");
    // for (auto &w : emuState.watches) {
    //     auto watch = cJSON_CreateObject();
    //     cJSON_AddNumberToObject(watch, "addr", w.addr);
    //     cJSON_AddStringToObject(watch, "name", w.name.c_str());
    //     cJSON_AddNumberToObject(watch, "type", (int)w.type);
    //     cJSON_AddItemToArray(watches, watch);
    // }

    std::ofstream ofs(configPath);
    if (!ofs.good())
        return;

    auto str = cJSON_Print(root);
    ofs.write(str, strlen(str));
    cJSON_free(str);
    cJSON_Delete(root);
}
