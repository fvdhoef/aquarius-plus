#pragma once

#include "Common.h"
#include "VFS.h"

class HttpVFS : public VFS {
    HttpVFS();

public:
    static HttpVFS &instance();
    void            init();

    // File operations
    int open(uint8_t flags, const std::string &path) override;
    int close(int fd) override;
    int read(int fd, size_t size, void *buf) override;
    int write(int fd, size_t size, const void *buf) override;

private:
};
