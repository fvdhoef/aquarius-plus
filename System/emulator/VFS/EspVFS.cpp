#include "EspVFS.h"

EspVFS::EspVFS() {
    fileOffset = 0;
}

EspVFS &EspVFS::instance() {
    static EspVFS vfs;
    return vfs;
}

void EspVFS::init() {
}

int EspVFS::open(uint8_t flags, const std::string &path) {
    return ERR_OTHER;
}

int EspVFS::close(int fd) {
    return ERR_OTHER;
}

int EspVFS::read(int fd, size_t size, void *buf) {
    return ERR_OTHER;
}

int EspVFS::write(int fd, size_t size, const void *buf) {
    return ERR_OTHER;
}

// Directory operations
DirEnumCtx EspVFS::direnum(const std::string &path) {
    return nullptr;
}

// Filesystem operations
int EspVFS::stat(const std::string &path, struct stat *st) {
    return ERR_OTHER;
}
