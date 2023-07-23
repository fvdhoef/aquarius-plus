#include "vfs.h"

int VFS::open(uint8_t flags, const char *path) {
    return ERR_OTHER;
}
int VFS::close(int fd) {
    return ERR_OTHER;
}
int VFS::read(int fd, uint16_t size, void *buf) {
    return ERR_OTHER;
}
int VFS::write(int fd, uint16_t size, const void *buf) {
    return ERR_OTHER;
}
int VFS::seek(int fd, uint32_t offset) {
    return ERR_OTHER;
}
int VFS::tell(int fd) {
    return ERR_OTHER;
}
int VFS::opendir(const char *path) {
    return ERR_OTHER;
}
int VFS::closedir(int dd) {
    return ERR_OTHER;
}
struct direnum_ent *VFS::readdir(int dd) {
    return nullptr;
}
int VFS::delete_(const char *path) {
    return ERR_OTHER;
}
int VFS::rename(const char *path_old, const char *path_new) {
    return ERR_OTHER;
}
int VFS::mkdir(const char *path) {
    return ERR_OTHER;
}
int VFS::stat(const char *path, struct stat *st) {
    return ERR_OTHER;
}
