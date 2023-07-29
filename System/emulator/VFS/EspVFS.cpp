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
    (void)flags;
    (void)path;
    return ERR_OTHER;
}

int EspVFS::close(int fd) {
    (void)fd;
    return ERR_OTHER;
}

int EspVFS::read(int fd, size_t size, void *buf) {
    (void)fd;
    (void)size;
    (void)buf;
    return ERR_OTHER;
}

int EspVFS::write(int fd, size_t size, const void *buf) {
    (void)fd;
    (void)size;
    (void)buf;
    return ERR_OTHER;
}

// Directory operations
DirEnumCtx EspVFS::direnum(const std::string &path) {
    (void)path;
    return nullptr;
}

// Filesystem operations
int EspVFS::stat(const std::string &path, struct stat *st) {
    (void)path;
    (void)st;
    return ERR_OTHER;
}
