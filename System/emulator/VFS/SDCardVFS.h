// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#pragma once

#include "Common.h"
#include "VFS.h"

#ifndef EMULATOR
#    define MOUNT_POINT "/sdcard"
#endif

class SDCardVFS : public VFS {
    SDCardVFS();

public:
    static SDCardVFS &instance();

#ifdef EMULATOR
    void init(const std::string &basePath);
#else
    void init();
#endif

    // File operations
    int open(uint8_t flags, const std::string &path) override;
    int close(int fd) override;
    int read(int fd, size_t size, void *buf) override;
    int write(int fd, size_t size, const void *buf) override;
    int seek(int fd, size_t offset) override;
    int tell(int fd) override;

    // Directory operations
    DirEnumCtx direnum(const std::string &path) override;

    // Filesystem operations
    int delete_(const std::string &path) override;
    int rename(const std::string &path_old, const std::string &path_new) override;
    int mkdir(const std::string &path) override;
    int stat(const std::string &path, struct stat *st) override;

private:
    std::string getFullPath(const std::string &path);

#ifdef EMULATOR
    std::string basePath;
#endif
};
