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
int VFS::readline(int, size_t, void *) {
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
std::pair<int, DirEnumCtx> VFS::direnum(const std::string &, uint8_t) {
    return std::make_pair(ERR_OTHER, nullptr);
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
