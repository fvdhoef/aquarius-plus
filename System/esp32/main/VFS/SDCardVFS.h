// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#pragma once

#include "Common.h"
#include "VFS.h"

#ifndef EMULATOR
#    include <sdmmc_cmd.h>
#    include <driver/sdmmc_types.h>
#    include <driver/sdspi_host.h>
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
#ifdef EMULATOR
    std::string getFullPath(const std::string &path);
#endif

#ifndef EMULATOR
    sdspi_dev_handle_t devHandle = -1;
    sdmmc_card_t      *card      = nullptr;
    sdmmc_host_t       host;
    void              *fatfs;

    // Used internally
public:
    uint8_t diskStatus(uint8_t pdrv);
    uint8_t diskInitialize(uint8_t pdrv);
    int     diskRead(uint8_t pdrv, uint8_t *buf, size_t sector, size_t count);
    int     diskWrite(uint8_t pdrv, const uint8_t *buf, size_t sector, size_t count);
    int     diskIoctl(uint8_t pdrv, uint8_t cmd, void *buf);

private:
#endif

#ifdef EMULATOR
    std::string basePath;
#endif
};
