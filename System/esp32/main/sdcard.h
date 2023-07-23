#pragma once

#include "common.h"
#include "vfs.h"

#define MOUNT_POINT "/sdcard"

class SDCardVFS : public VFS {
    SDCardVFS();

public:
    static SDCardVFS &instance();
    void              init();

    // File operations
    int open(uint8_t flags, const char *path) override;
    int close(int fd) override;
    int read(int fd, uint16_t size, void *buf) override;
    int write(int fd, uint16_t size, const void *buf) override;
    int seek(int fd, uint32_t offset) override;
    int tell(int fd) override;

    // Directory operations
    DirEnumCtx direnum(const char *path) override;

    // Filesystem operations
    int delete_(const char *path) override;
    int rename(const char *path_old, const char *path_new) override;
    int mkdir(const char *path) override;
    int stat(const char *path, struct stat *st) override;

private:
};
