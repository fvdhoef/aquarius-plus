#include "Config.h"
#include "EmuState.h"
#include "Keyboard.h"

Config::Config() {
}

Config *Config::instance() {
    static Config obj;
    return &obj;
}

void Config::init(const std::string &_appDataPath) {
    appDataPath = _appDataPath;
    load();
    save();
}

cJSON *Config::loadConfigFile(const std::string &filename) {
    std::string   jsonStr = "{}";
    std::ifstream ifs(appDataPath + "/" + filename);
    if (ifs.good()) {
        jsonStr = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }

    auto root = cJSON_ParseWithLength(jsonStr.c_str(), jsonStr.size());
    return root;
}

void Config::saveConfigFile(const std::string &filename, cJSON *root) {
    auto          path = appDataPath + "/" + filename;
    std::ofstream ofs(path);
    if (ofs.good()) {
        auto str = cJSON_Print(root);
        ofs.write(str, strlen(str));
        cJSON_free(str);
    }
    cJSON_Delete(root);
}

static int digitVal(uint8_t digit) {
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    return -1;
}

void Config::load() {
    if (auto root = loadConfigFile("config.json")) {
        imguiConf      = getStringValue(root, "imguiConfig", "");
        sdCardPath     = getStringValue(root, "sdCardPath", "");
        asmListingPath = getStringValue(root, "asmListingPath", "");

        wndPosX        = getIntValue(root, "wndPosX", SDL_WINDOWPOS_CENTERED);
        wndPosY        = getIntValue(root, "wndPosY", SDL_WINDOWPOS_CENTERED);
        wndWidth       = getIntValue(root, "wndWidth", 800);
        wndHeight      = getIntValue(root, "wndHeight", 600);
        enableSound    = getBoolValue(root, "enableSound", true);
        enableMouse    = getBoolValue(root, "enableMouse", true);
        fontScale2x    = getBoolValue(root, "fontScale2x", false);
        enableDebugger = getBoolValue(root, "enableDebugger", false);

        displayScaling = (DisplayScaling)getIntValue(root, "displayScaling", (int)DisplayScaling::Linear);

        Keyboard::instance()->setKeyLayout((KeyLayout)getIntValue(root, "keyLayout", 0));

        showEspInfo = getBoolValue(root, "showEspInfo", false);

        // Read all NVS U8 items
        {
            nvs_u8.clear();
            if (auto obj = cJSON_GetObjectItem(root, "nvs_u8")) {
                cJSON *item;
                cJSON_ArrayForEach(item, obj) {
                    if (item->type != cJSON_Number)
                        continue;

                    uint8_t val = item->valueint;
                    nvs_u8.insert_or_assign(item->string, val);
                }
            }
        }

        // Read all NVS blob items
        {
            nvs_blobs.clear();
            if (auto obj = cJSON_GetObjectItem(root, "nvs_blobs")) {
                cJSON *item;
                cJSON_ArrayForEach(item, obj) {
                    if (item->type != cJSON_String)
                        continue;

                    const char *str = cJSON_GetStringValue(item);
                    unsigned    len = (unsigned)strlen(str);
                    if (len % 2 != 0)
                        continue;

                    std::vector<uint8_t> val;
                    val.reserve(len / 2);

                    bool err = false;
                    for (unsigned i = 0; i < len / 2; i++) {
                        int digit0 = digitVal(str[i * 2 + 0]);
                        int digit1 = digitVal(str[i * 2 + 1]);
                        if (digit0 < 0 || digit1 < 0) {
                            err = true;
                            break;
                        }
                        val.push_back((digit0 << 4) | digit1);
                    }
                    if (err)
                        continue;

                    nvs_blobs.insert_or_assign(item->string, val);
                }
            }
        }
        cJSON_free(root);
    }

    // Sanitize some variables
    wndPosX   = std::max(wndPosX, 0);
    wndPosY   = std::max(wndPosY, 0);
    wndWidth  = std::max(wndWidth, 800);
    wndHeight = std::max(wndHeight, 600);
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
    cJSON_AddBoolToObject(root, "fontScale2x", fontScale2x);
    cJSON_AddBoolToObject(root, "enableDebugger", enableDebugger);

    cJSON_AddNumberToObject(root, "displayScaling", (int)displayScaling);

    cJSON_AddNumberToObject(root, "keyLayout", (int)Keyboard::instance()->getKeyLayout());

    cJSON_AddBoolToObject(root, "showEspInfo", showEspInfo);

    // Store all NVS U8 items
    {
        cJSON *obj = cJSON_AddObjectToObject(root, "nvs_u8");
        for (auto &item : nvs_u8) {
            cJSON_AddNumberToObject(obj, item.first.c_str(), item.second);
        }
    }
    // Store all NVS blob items
    {
        cJSON *obj = cJSON_AddObjectToObject(root, "nvs_blobs");
        for (auto &item : nvs_blobs) {
            std::string str;
            str.reserve(item.second.size() * 2);

            for (uint8_t val : item.second) {
                static const char *digits = "0123456789abcdef";
                str.push_back(digits[val >> 4]);
                str.push_back(digits[val & 0xF]);
            }
            cJSON_AddStringToObject(obj, item.first.c_str(), str.c_str());
        }
    }

    saveConfigFile("config.json", root);
}
