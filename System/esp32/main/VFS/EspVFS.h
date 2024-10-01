#pragma once

#include "Common.h"
#include "VFS.h"

class EspVFS : public VFS {
    EspVFS();

public:
    static EspVFS &instance();
    void           init();

    // File operations
    int open(uint8_t flags, const std::string &path) override;
    int close(int fd) override;
    int read(int fd, size_t size, void *buf) override;
    int write(int fd, size_t size, const void *buf) override;
    int seek(int fd, size_t offset) override;
    int tell(int fd) override;

    // Directory operations
    std::pair<int, DirEnumCtx> direnum(const std::string &path, uint8_t flags) override;

    // Filesystem operations
    int stat(const std::string &path, struct stat *st) override;
};
