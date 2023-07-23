#include "EspVFS.h"
#include "EspSettingsConsole.h"

extern const uint8_t settings_caq_start[] asm("_binary_settings_caq_start");
extern const uint8_t settings_caq_end[] asm("_binary_settings_caq_end");
static const char   *fn_settings = "settings.caq";
static const char   *fn_com      = "com";

static void console_task(void *pvParameters);

EspVFS::EspVFS() {
    dir_idx     = 0;
    file_offset = 0;
    file_idx    = 0;
}

EspVFS &EspVFS::instance() {
    static EspVFS vfs;
    return vfs;
}

void EspVFS::init(void) {
    EspSettingsConsole::instance().init();
}

int EspVFS::open(uint8_t flags, const std::string &_path) {
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
        file_idx = 0;
    } else {
        file_idx = -1;
    }

    file_offset = 0;
    return file_idx < 0 ? ERR_NOT_FOUND : 0;
}

int EspVFS::read(int fd, size_t size, void *buf) {
    if (fd == 0) {
        int filesize  = settings_caq_end - settings_caq_start;
        int remaining = filesize - file_offset;

        if (size > remaining) {
            size = remaining;
        }
        memcpy(buf, settings_caq_start + file_offset, size);
        file_offset += size;
        return size;

    } else if (fd == 1) {
        return EspSettingsConsole::instance().recv(buf, size, 0);
    } else {
        return ERR_OTHER;
    }
}

int EspVFS::write(int fd, size_t size, const void *buf) {
    if (fd == 1) {
        return EspSettingsConsole::instance().send(buf, size, 0);
    } else {
        return ERR_OTHER;
    }
}

int EspVFS::close(int fd) {
    return 0;
}

DirEnumCtx EspVFS::direnum(const std::string &path) {
    auto result = std::make_shared<std::vector<DirEnumEntry>>();
    result->emplace_back(fn_settings, settings_caq_end - settings_caq_start, 0, 0, 0);
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
