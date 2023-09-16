// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#include "EspVFS.h"
#include "EspSettingsConsole.h"

#ifdef EMULATOR
#    include "settings_aqx.h"
const uint8_t *settings_aqx_end = settings_aqx_start + sizeof(settings_aqx_start);

#else
extern const uint8_t settings_aqx_start[] asm("_binary_settings_aqx_start");
extern const uint8_t settings_aqx_end[] asm("_binary_settings_aqx_end");
#endif

static const char *fn_settings = "settings.aqx";
static const char *fn_com      = "com";

EspVFS::EspVFS() {
    fileOffset = 0;
}

EspVFS &EspVFS::instance() {
    static EspVFS vfs;
    return vfs;
}

void EspVFS::init() {
    EspSettingsConsole::instance().init();
}

int EspVFS::open(uint8_t flags, const std::string &_path) {
    (void)flags;

    // Skip leading slashes
    auto idx = _path.find_first_not_of('/');
    if (idx == std::string::npos) {
        idx = _path.size();
    }
    auto path = _path.substr(idx);

    // printf("esp_open(%u, \"%s\")\n", flags, path);

    if (strcasecmp(path.c_str(), fn_com) == 0) {
        EspSettingsConsole::instance().newSession();
        return 1;
    }
    if (strcasecmp(path.c_str(), fn_settings) == 0) {
        fileOffset = 0;
        return 0;
    }
    return ERR_OTHER;
}

int EspVFS::read(int fd, size_t size, void *buf) {
    if (fd == 0) {
        int filesize  = (int)(settings_aqx_end - settings_aqx_start);
        int remaining = filesize - fileOffset;

        if ((int)size > remaining) {
            size = remaining;
        }
        memcpy(buf, settings_aqx_start + fileOffset, size);
        fileOffset += (int)size;
        return (int)size;

    } else if (fd == 1) {
        return EspSettingsConsole::instance().recv(buf, size);
    } else {
        return ERR_OTHER;
    }
}

int EspVFS::write(int fd, size_t size, const void *buf) {
    if (fd == 1) {
        return EspSettingsConsole::instance().send(buf, size);
    } else {
        return ERR_OTHER;
    }
}

int EspVFS::close(int fd) {
    (void)fd;
    return 0;
}

DirEnumCtx EspVFS::direnum(const std::string &path) {
    (void)path;
    auto result = std::make_shared<std::vector<DirEnumEntry>>();
    result->emplace_back(fn_settings, (int)(settings_aqx_end - settings_aqx_start), 0, 0, 0);
    return result;
}

int EspVFS::stat(const std::string &path, struct stat *st) {
    if (strcasecmp(path.c_str(), "") == 0) {
        memset(st, 0, sizeof(*st));
        st->st_mode = S_IFDIR;
        return 0;
    }
    return ERR_OTHER;
}
