#include "vfs.h"

int VFS::open(uint8_t flags, const std::string &path) {
    return ERR_OTHER;
}
int VFS::close(int fd) {
    return ERR_OTHER;
}
int VFS::read(int fd, size_t size, void *buf) {
    return ERR_OTHER;
}
int VFS::write(int fd, size_t size, const void *buf) {
    return ERR_OTHER;
}
int VFS::seek(int fd, size_t offset) {
    return ERR_OTHER;
}
int VFS::tell(int fd) {
    return ERR_OTHER;
}
DirEnumCtx VFS::direnum(const std::string &path) {
    return nullptr;
}
int VFS::delete_(const std::string &path) {
    return ERR_OTHER;
}
int VFS::rename(const std::string &path_old, const std::string &path_new) {
    return ERR_OTHER;
}
int VFS::mkdir(const std::string &path) {
    return ERR_OTHER;
}
int VFS::stat(const std::string &path, struct stat *st) {
    return ERR_OTHER;
}
