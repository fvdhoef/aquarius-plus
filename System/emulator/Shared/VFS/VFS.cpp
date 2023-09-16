// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#include "VFS.h"

int VFS::open(uint8_t, const std::string &) {
    return ERR_OTHER;
}
int VFS::close(int) {
    return ERR_OTHER;
}
int VFS::read(int, size_t, void *) {
    return ERR_OTHER;
}
int VFS::write(int, size_t, const void *) {
    return ERR_OTHER;
}
int VFS::seek(int, size_t) {
    return ERR_OTHER;
}
int VFS::tell(int) {
    return ERR_OTHER;
}
DirEnumCtx VFS::direnum(const std::string &) {
    return nullptr;
}
int VFS::delete_(const std::string &) {
    return ERR_OTHER;
}
int VFS::rename(const std::string &, const std::string &) {
    return ERR_OTHER;
}
int VFS::mkdir(const std::string &) {
    return ERR_OTHER;
}
int VFS::stat(const std::string &, struct stat *) {
    return ERR_OTHER;
}
