#include "SDCardVFS.h"

#ifndef EMULATOR
#    include <sys/unistd.h>
#    include <sys/stat.h>
#    include <errno.h>
#    include "PowerLED.h"
#    include "ff.h"
#    include "diskio.h"
#else
#    ifndef _WIN32
#        include <sys/types.h>
#        include <sys/stat.h>
#        include <dirent.h>
#        include <unistd.h>
#    else
#        include <io.h>
#        include <time.h>
#    endif
#endif

#ifndef EMULATOR
static const char *TAG = "SDCardVFS";
#endif

#define MAX_FDS (10)

#ifndef EMULATOR
struct state {
    FIL *fds[MAX_FDS];
};
#else
struct state {
    FILE *fds[MAX_FDS];
};
#endif

static struct state state;

SDCardVFS::SDCardVFS() {
}

SDCardVFS &SDCardVFS::instance() {
    static SDCardVFS vfs;
    return vfs;
}

#ifndef EMULATOR
void SDCardVFS::init() {
    fatfs = calloc(1, sizeof(FATFS));
    assert(fatfs != nullptr);

    host                     = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num   = (gpio_num_t)IOPIN_SD_MOSI,
        .miso_io_num   = (gpio_num_t)IOPIN_SD_MISO,
        .sclk_io_num   = (gpio_num_t)IOPIN_SD_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        // .max_transfer_sz = 4000,
    };
    ESP_ERROR_CHECK(spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SPI_DMA_CH_AUTO));

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs               = (gpio_num_t)IOPIN_SD_SSEL_N;
    slot_config.gpio_cd               = (gpio_num_t)IOPIN_SD_CD_N;
    slot_config.gpio_wp               = (gpio_num_t)IOPIN_SD_WP_N;
    slot_config.host_id               = (spi_host_device_t)host.slot;
    ESP_ERROR_CHECK(sdspi_host_init_device(&slot_config, &devHandle));

    f_mount((FATFS *)fatfs, "", 0);
}

static int mapFatFsResult(FRESULT res) {
    switch (res) {
        case FR_OK: return 0;
        case FR_DISK_ERR: return ERR_NO_DISK;
        case FR_INT_ERR: return ERR_OTHER;
        case FR_NOT_READY: return ERR_NO_DISK;
        case FR_NO_FILE: return ERR_NOT_FOUND;
        case FR_NO_PATH: return ERR_NOT_FOUND;
        case FR_INVALID_NAME: return ERR_NOT_FOUND;
        case FR_DENIED: return ERR_OTHER;
        case FR_EXIST: return ERR_EXISTS;
        case FR_INVALID_OBJECT: return ERR_OTHER;
        case FR_WRITE_PROTECTED: return ERR_WRITE_PROTECTED;
        case FR_INVALID_DRIVE: return ERR_NO_DISK;
        case FR_NOT_ENABLED: return ERR_NO_DISK;
        case FR_NO_FILESYSTEM: return ERR_NO_DISK;
        case FR_MKFS_ABORTED: return ERR_OTHER;
        case FR_TIMEOUT: return ERR_OTHER;
        case FR_LOCKED: return ERR_OTHER;
        case FR_NOT_ENOUGH_CORE: return ERR_OTHER;
        case FR_TOO_MANY_OPEN_FILES: return ERR_OTHER;
        case FR_INVALID_PARAMETER: return ERR_PARAM;
    }
    return ERR_OTHER;
}

int SDCardVFS::open(uint8_t flags, const std::string &path) {
    // Translate flags
    uint8_t mode = 0;
    switch (flags & FO_ACCMODE) {
        case FO_RDONLY:
            mode |= FA_READ;
            break;
        case FO_WRONLY:
            mode |= FA_WRITE;
            if (flags & FO_APPEND) {
                mode |= FA_OPEN_APPEND;
            } else if (flags & FO_CREATE) {
                if (flags & FO_EXCL) {
                    mode |= FA_CREATE_NEW;
                } else {
                    mode |= FA_CREATE_ALWAYS;
                }
            }
            break;
        case FO_RDWR:
            mode |= FA_READ | FA_WRITE;
            if (flags & FO_APPEND) {
                mode |= FA_OPEN_APPEND;
            } else if (flags & FO_CREATE) {
                if (flags & FO_EXCL) {
                    mode |= FA_CREATE_NEW;
                } else {
                    mode |= FA_CREATE_ALWAYS;
                }
            }
            break;
        default:
            // Error
            return ERR_PARAM;
    }

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (state.fds[i] == nullptr) {
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return ERR_TOO_MANY_OPEN;

    state.fds[fd] = (FIL *)calloc(1, sizeof(FIL));
    assert(state.fds[fd]);

    auto res = f_open(state.fds[fd], path.c_str(), mode);
    if (res != FR_OK) {
        free(state.fds[fd]);
        state.fds[fd] = nullptr;
        return mapFatFsResult(res);
    }
    return fd;
}

int SDCardVFS::close(int fd) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    auto res = f_close(state.fds[fd]);

    free(state.fds[fd]);
    state.fds[fd] = nullptr;
    return mapFatFsResult(res);
}

int SDCardVFS::read(int fd, size_t size, void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;

    UINT br;
    auto res = f_read(state.fds[fd], buf, size, &br);
    if (res != FR_OK)
        return mapFatFsResult(res);
    return br;
}

int SDCardVFS::readline(int fd, size_t size, void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;

    char *p = (char *)buf;
    p[0]    = 0;

    TCHAR *result = f_gets((TCHAR *)buf, size, state.fds[fd]);
    if (result == NULL) {
        if (f_eof(state.fds[fd]))
            return ERR_EOF;
        FRESULT res = (FRESULT)f_error(state.fds[fd]);
        return mapFatFsResult(res);
    }
    return 0;
}

int SDCardVFS::write(int fd, size_t size, const void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;

    UINT bw;
    auto res = f_write(state.fds[fd], buf, size, &bw);
    if (res != FR_OK)
        return mapFatFsResult(res);
    return bw;
}

int SDCardVFS::seek(int fd, size_t offset) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;

    auto res = f_lseek(state.fds[fd], offset);
    return mapFatFsResult(res);
}

int SDCardVFS::tell(int fd) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    return f_tell(state.fds[fd]);
}

DirEnumCtx SDCardVFS::direnum(const std::string &path, uint8_t flags) {
    bool mode83 = (flags & DE_FLAG_MODE83) != 0;

    DIR dir;
    if (f_opendir(&dir, path.c_str()) != F_OK) {
        return nullptr;
    }

    auto result = std::make_shared<std::vector<DirEnumEntry>>();

    // Read directory contents
    FILINFO fno;
    while (1) {
        if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) {
            // Done
            break;
        }

        if ((flags & DE_FLAG_HIDDEN) == 0) {
            // Skip hidden and system files
            if ((fno.fattrib & (AM_SYS | AM_HID)))
                continue;

            // Skip files beginning with a space
            if (fno.fname[0] == '.')
                continue;
        }

        result->emplace_back(
            (mode83 && fno.altname[0] != 0) ? fno.altname : fno.fname,
            fno.fsize, (fno.fattrib & AM_DIR) ? DE_ATTR_DIR : 0, fno.fdate, fno.ftime);
    }

    // Close directory
    f_closedir(&dir);

    return result;
}

int SDCardVFS::delete_(const std::string &path) {
    FRESULT res;

    if ((res = f_unlink(path.c_str())) != FR_OK) {
        res = f_rmdir(path.c_str());
    }
    return mapFatFsResult(res);
}

int SDCardVFS::rename(const std::string &pathOld, const std::string &pathNew) {
    if (pathOld == pathNew)
        return 0;

    auto res = f_rename(pathOld.c_str(), pathNew.c_str());
    return mapFatFsResult(res);
}

int SDCardVFS::mkdir(const std::string &path) {
    auto res = f_mkdir(path.c_str());
    return mapFatFsResult(res);
}

typedef union {
    struct {
        uint16_t mday : 5; /* Day of month, 1 - 31 */
        uint16_t mon : 4;  /* Month, 1 - 12 */
        uint16_t year : 7; /* Year, counting from 1980. E.g. 37 for 2017 */
    };
    uint16_t as_int;
} fat_date_t;

typedef union {
    struct {
        uint16_t sec : 5;  /* Seconds divided by 2. E.g. 21 for 42 seconds */
        uint16_t min : 6;  /* Minutes, 0 - 59 */
        uint16_t hour : 5; /* Hour, 0 - 23 */
    };
    uint16_t as_int;
} fat_time_t;

int SDCardVFS::stat(const std::string &path, struct stat *st) {
    if (path.empty() || path == "/") {
        // Handle root
        memset(st, 0, sizeof(*st));
        st->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | S_IFDIR;
        return 0;
    }

    FILINFO info;
    auto    res = f_stat(path.c_str(), &info);
    if (res == FR_OK) {
        memset(st, 0, sizeof(*st));
        st->st_size = info.fsize;
        st->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | ((info.fattrib & AM_DIR) != 0 ? S_IFDIR : S_IFREG);

        fat_date_t fdate = {.as_int = info.fdate};
        fat_time_t ftime = {.as_int = info.ftime};

        struct tm tm;
        memset(&tm, 0, sizeof(tm));
        tm.tm_mday  = fdate.mday,
        tm.tm_mon   = fdate.mon - 1,
        tm.tm_year  = fdate.year + 80,
        tm.tm_sec   = ftime.sec * 2,
        tm.tm_min   = ftime.min,
        tm.tm_hour  = ftime.hour,
        tm.tm_isdst = -1;

        st->st_mtime = mktime(&tm);
        st->st_atime = 0;
        st->st_ctime = 0;
    }
    return mapFatFsResult(res);
}

uint8_t SDCardVFS::diskStatus(uint8_t pdrv) {
    (void)pdrv;
    bool hasDisk         = !gpio_get_level((gpio_num_t)IOPIN_SD_CD_N);
    bool hasWriteProtect = !gpio_get_level((gpio_num_t)IOPIN_SD_WP_N);

    uint8_t status = 0;
    if (hasDisk) {
        if (card == nullptr || sdmmc_get_status(card) != ESP_OK)
            status |= STA_NOINIT;

        if (hasWriteProtect)
            status |= STA_PROTECT;
    } else {
        status |= STA_NOINIT | STA_NODISK;
    }
    return status;
}

uint8_t SDCardVFS::diskInitialize(uint8_t pdrv) {
    uint8_t status = diskStatus(pdrv);
    if (status & STA_NODISK)
        return status;

    if (status & STA_NOINIT) {
        ESP_LOGI(TAG, "Initializing SD card...");
        if (card == nullptr) {
            card = new sdmmc_card_t();
            assert(card != nullptr);
        }
        memset(card, 0, sizeof(*card));

        auto err = sdmmc_card_init(&host, card);
        if (err == ESP_OK) {
            sdmmc_card_print_info(stdout, card);
            status &= ~STA_NOINIT;
        } else {
            ESP_LOGE(TAG, "Error initializing SD card: %d", err);
            delete card;
            card = nullptr;
        }
    }
    return status;
}

int SDCardVFS::diskRead(uint8_t pdrv, uint8_t *buf, size_t sector, size_t count) {
    (void)pdrv;
    if (!card)
        return RES_PARERR;

    PowerLED::instance().flashStart();
    esp_err_t err = sdmmc_read_sectors(card, buf, sector, count);
    PowerLED::instance().flashStop();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "sdmmc_read_blocks failed (%d)", err);
        return RES_ERROR;
    }
    return RES_OK;
}

int SDCardVFS::diskWrite(uint8_t pdrv, const uint8_t *buf, size_t sector, size_t count) {
    (void)pdrv;
    if (!card)
        return RES_PARERR;

    PowerLED::instance().flashStart();
    esp_err_t err = sdmmc_write_sectors(card, buf, sector, count);
    PowerLED::instance().flashStop();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "sdmmc_write_blocks failed (%d)", err);
        return RES_ERROR;
    }
    return RES_OK;
}

int SDCardVFS::diskIoctl(uint8_t pdrv, uint8_t cmd, void *buf) {
    (void)pdrv;

    if (!card)
        return RES_PARERR;
    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT: *((DWORD *)buf) = card->csd.capacity; return RES_OK;
        case GET_SECTOR_SIZE: *((WORD *)buf) = card->csd.sector_size; return RES_OK;
        case GET_BLOCK_SIZE: return RES_ERROR;
#    if FF_USE_TRIM
        case CTRL_TRIM:
            return ff_sdmmc_trim(pdrv, *((DWORD *)buf),                        // start_sector
                                 (*((DWORD *)buf + 1) - *((DWORD *)buf) + 1)); // sector_count
#    endif                                                                     // FF_USE_TRIM
    }
    return RES_ERROR;
}

DSTATUS disk_status(BYTE pdrv) {
    return (DSTATUS)SDCardVFS::instance().diskStatus(pdrv);
}

DSTATUS disk_initialize(BYTE pdrv) {
    return (DSTATUS)SDCardVFS::instance().diskInitialize(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buf, LBA_t sector, UINT count) {
    return (DRESULT)SDCardVFS::instance().diskRead(pdrv, buf, sector, count);
}

DRESULT disk_write(BYTE pdrv, const BYTE *buf, LBA_t sector, UINT count) {
    return (DRESULT)SDCardVFS::instance().diskWrite(pdrv, buf, sector, count);
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buf) {
    return (DRESULT)SDCardVFS::instance().diskIoctl(pdrv, cmd, buf);
}

DWORD get_fattime(void) {
    time_t    t = time(NULL);
    struct tm tmr;
    localtime_r(&t, &tmr);
    int year = tmr.tm_year < 80 ? 0 : tmr.tm_year - 80;
    return ((DWORD)(year) << 25) | ((DWORD)(tmr.tm_mon + 1) << 21) | ((DWORD)tmr.tm_mday << 16) | (WORD)(tmr.tm_hour << 11) | (WORD)(tmr.tm_min << 5) | (WORD)(tmr.tm_sec >> 1);
}

#else
void SDCardVFS::init(const std::string &_basePath) {
    // printf("Settings SD card directory: '%s'\n", _basePath.c_str());

    basePath = _basePath;
    stripTrailingSlashes(basePath);
}

std::string SDCardVFS::getFullPath(const std::string &path) {
    // Compose full path
    std::string result = basePath;
    result += "/";
    result += path;
    stripTrailingSlashes(result);
    return result;
}

static int mapErrNoResult(void) {
    switch (errno) {
        case EEXIST: return ERR_EXISTS;
        case EACCES:
        case ENOENT: return ERR_NOT_FOUND;
        case EINVAL:
        case EBADF: return ERR_PARAM;
        case ENOTEMPTY: return ERR_NOT_EMPTY;
    }
    return ERR_OTHER;
}

int SDCardVFS::open(uint8_t flags, const std::string &path) {
    if (basePath.empty())
        return ERR_NO_DISK;

    // Translate flags
    int  mi = 0;
    char mode[5];

    // if (flags & FO_CREATE)
    //     oflag |= O_CREAT;

    switch (flags & FO_ACCMODE) {
        case FO_RDONLY:
            mode[mi++] = 'r';
            break;
        case FO_WRONLY:
            if (flags & FO_APPEND) {
                mode[mi++] = 'a';
            } else {
                mode[mi++] = 'w';
            }
            if (flags & FO_EXCL) {
                mode[mi++] = 'x';
            }

            break;
        case FO_RDWR:
            if (flags & FO_APPEND) {
                mode[mi++] = 'a';
                mode[mi++] = '+';
            } else if (flags & FO_TRUNC) {
                mode[mi++] = 'w';
                mode[mi++] = '+';
            } else {
                mode[mi++] = 'r';
                mode[mi++] = '+';
            }
            if (flags & FO_EXCL) {
                mode[mi++] = 'x';
            }
            break;

        default: {
            // Error
            return ERR_PARAM;
        }
    }
    mode[mi++] = 'b';
    mode[mi]   = 0;

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (state.fds[i] == nullptr) {
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return ERR_TOO_MANY_OPEN;

    auto  fullPath = getFullPath(path);
    FILE *f        = ::fopen(fullPath.c_str(), mode);
    if (f == nullptr) {
        return mapErrNoResult();
    }
    state.fds[fd] = f;
    return fd;
}

int SDCardVFS::close(int fd) {
    if (basePath.empty())
        return ERR_NO_DISK;

    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    ::fclose(f);
    state.fds[fd] = nullptr;
    return 0;
}

int SDCardVFS::read(int fd, size_t size, void *buf) {
    if (basePath.empty())
        return ERR_NO_DISK;

    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)::fread(buf, 1, size, f);
    return (result < 0) ? mapErrNoResult() : result;
}

int SDCardVFS::readline(int fd, size_t size, void *buf) {
    if (basePath.empty())
        return ERR_NO_DISK;

    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;

    FILE *f = state.fds[fd];
    if (fgets((char *)buf, size, f) == NULL) {
        if (feof(f))
            return ERR_EOF;
        errno = ferror(f);
        return mapErrNoResult();
    }
    return 0;
}

int SDCardVFS::write(int fd, size_t size, const void *buf) {
    if (basePath.empty())
        return ERR_NO_DISK;

    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)::fwrite(buf, 1, size, f);
    return (result < 0) ? mapErrNoResult() : result;
}

int SDCardVFS::seek(int fd, size_t offset) {
    if (basePath.empty())
        return ERR_NO_DISK;

    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)::fseek(f, (long)offset, SEEK_SET);
    return (result < 0) ? mapErrNoResult() : 0;
}

int SDCardVFS::tell(int fd) {
    if (basePath.empty())
        return ERR_NO_DISK;

    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)::ftell(f);
    return (result < 0) ? mapErrNoResult() : result;
}

DirEnumCtx SDCardVFS::direnum(const std::string &path, uint8_t flags) {
    if (basePath.empty())
        return nullptr;

    bool mode83     = (flags & DE_FLAG_MODE83) != 0;
    bool showHidden = (flags & DE_FLAG_HIDDEN) != 0;

    auto fullPath = getFullPath(path);

#    ifndef _WIN32
    DIR *dir = ::opendir(fullPath.c_str());
    if (dir == nullptr) {
        return nullptr;
    }
#    else
    struct _finddata_t fileinfo;
    intptr_t           handle = _findfirst((fullPath + "/*.*").c_str(), &fileinfo);
    if (handle < 0) {
        return nullptr;
    }
    bool first = true;
#    endif

    auto result = std::make_shared<std::vector<DirEnumEntry>>();

    // Read directory contents
    while (1) {
        DirEnumEntry dee;
#    ifndef _WIN32
        // Read directory entry
        struct dirent *de = ::readdir(dir);
        if (de == NULL) {
            break;
        }

        // Read additional file stats
        std::string filePath = fullPath + "/" + de->d_name;
        struct stat st;
        if (::stat(filePath.c_str(), &st) < 0) {
            continue;
        }

        // Return file entry
        dee.filename = de->d_name;
        dee.size     = (de->d_type == DT_DIR) ? 0 : st.st_size;
        dee.attr     = (de->d_type == DT_DIR) ? DE_ATTR_DIR : 0;

#        ifdef __APPLE__
        time_t t = st.st_mtimespec.tv_sec;
#        else
        time_t t = st.st_mtim.tv_sec;
#        endif

#    else
        if (!first) {
            if (_findnext(handle, &fileinfo) != 0)
                break;
        }
        first = false;

        // Skip hidden and system files
        if (fileinfo.attrib & (_A_HIDDEN | _A_SYSTEM))
            continue;

        // Return file entry
        dee.filename = fileinfo.name;
        dee.size     = (fileinfo.attrib & _A_SUBDIR) ? 0 : fileinfo.size;
        dee.attr     = (fileinfo.attrib & _A_SUBDIR) ? DE_ATTR_DIR : 0;
        time_t t     = fileinfo.time_write;
#    endif

        struct tm *tm = ::localtime(&t);
        dee.ftime     = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
        dee.fdate     = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

        // Skip files starting with a dot
        if (dee.filename.size() == 0 || dee.filename == "." || dee.filename == ".." || (dee.filename[0] == '.' && !showHidden))
            continue;

        if (mode83) {
            // Skip file if it does not conform to 8.3

            // Filename length > 12 characters?
            if (dee.filename.size() > 12)
                continue;

            auto extPos = dee.filename.find_first_of('.');
            if (extPos == std::string::npos) {
                // No extension and length > 8?
                if (dee.filename.size() > 8)
                    continue;

            } else {
                // Base filename > 8 characters
                if (extPos > 8) {
                    continue;
                }
                // Multiple dots?
                if (dee.filename.find_first_of('.', extPos + 1) != std::string::npos)
                    continue;
            }

            // Disallowed characters?
            bool ok = true;
            for (unsigned i = 0; i < dee.filename.size(); i++) {
                dee.filename[i] = std::toupper(dee.filename[i]);
                if (dee.filename[i] <= ' ' || dee.filename[i] >= '~') {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;
        }

        result->push_back(dee);
    }

#    ifndef _WIN32
    ::closedir(dir);
#    else
    ::_findclose(handle);
#    endif

    return result;
}

int SDCardVFS::delete_(const std::string &path) {
    if (basePath.empty())
        return ERR_NO_DISK;

    auto fullPath = getFullPath(path);

    int result = ::unlink(fullPath.c_str());
    if (result < 0) {
        result = ::rmdir(fullPath.c_str());
    }
    return (result < 0) ? mapErrNoResult() : 0;
}

int SDCardVFS::rename(const std::string &pathOld, const std::string &pathNew) {
    if (basePath.empty())
        return ERR_NO_DISK;

    auto fullOld = getFullPath(pathOld);
    auto fullNew = getFullPath(pathNew);

    int result = ::rename(fullOld.c_str(), fullNew.c_str());
    return (result < 0) ? mapErrNoResult() : 0;
}

int SDCardVFS::mkdir(const std::string &path) {
    if (basePath.empty())
        return ERR_NO_DISK;

    auto fullPath = getFullPath(path);

#    if _WIN32
    int result = ::mkdir(fullPath.c_str());
#    else
    int result = ::mkdir(fullPath.c_str(), 0775);
#    endif
    return (result < 0) ? mapErrNoResult() : 0;
}

int SDCardVFS::stat(const std::string &path, struct stat *st) {
    if (basePath.empty())
        return ERR_NO_DISK;

    auto fullPath = getFullPath(path);
    int  result   = ::stat(fullPath.c_str(), st);
    return (result < 0) ? mapErrNoResult() : 0;
}
#endif
