// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
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

    // Directory operations
    DirEnumCtx direnum(const std::string &path) override;

    // Filesystem operations
    int stat(const std::string &path, struct stat *st) override;

private:
    int fileOffset;
};
