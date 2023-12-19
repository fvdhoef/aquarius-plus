#include "TcpVFS.h"

TcpVFS::TcpVFS() {
}

TcpVFS &TcpVFS::instance() {
    static TcpVFS vfs;
    return vfs;
}

void TcpVFS::init() {
}

int TcpVFS::open(uint8_t flags, const std::string &_path) {
    (void)flags;
    printf("TCP open: %s\n", _path.c_str());
    return ERR_OTHER;
}

int TcpVFS::read(int fd, size_t size, void *buf) {
    (void)fd;
    (void)size;
    (void)buf;
    return ERR_OTHER;
}

int TcpVFS::write(int fd, size_t size, const void *buf) {
    (void)fd;
    (void)size;
    (void)buf;
    return ERR_OTHER;
}

int TcpVFS::close(int fd) {
    (void)fd;
    return 0;
}
