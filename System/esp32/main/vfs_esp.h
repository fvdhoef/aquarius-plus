#pragma once

#include "common.h"
#include "vfs.h"
#include <freertos/stream_buffer.h>

class EspVFS : public VFS {
    EspVFS();

public:
    static EspVFS &instance();
    void           init();

    // File operations
    int open(uint8_t flags, const char *path) override;
    int close(int fd) override;
    int read(int fd, uint16_t size, void *buf) override;
    int write(int fd, uint16_t size, const void *buf) override;

    // Directory operations
    DirEnumCtx direnum(const char *path) override;

    // Filesystem operations
    int stat(const char *path, struct stat *st) override;

private:
    int dir_idx;
    int file_offset;
    int file_idx;
};
