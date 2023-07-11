#include "sdcard.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <dirent.h>
#include "direnum.h"
#include <errno.h>
#include "led.h"

static const char *TAG = "sdcard";

#if CONFIG_IDF_TARGET_ESP32S2
#    define SPI_DMA_CHAN host.slot
#else
#    define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#endif

void sdcard_init(void) {
    ESP_LOGI(TAG, "Initializing SD card");

    esp_err_t        ret;
    sdmmc_host_t     host    = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = IOPIN_SD_MOSI,
        .miso_io_num     = IOPIN_SD_MISO,
        .sclk_io_num     = IOPIN_SD_SCK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    const char *mount_point = MOUNT_POINT;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs               = IOPIN_SD_SSEL_N;
    slot_config.gpio_cd               = IOPIN_SD_CD_N;
    slot_config.gpio_wp               = IOPIN_SD_WP_N;
    slot_config.host_id               = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");

    sdmmc_card_t                    *card;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .max_files = 5,
    };
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount card: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

#define MAX_FDS (10)
#define MAX_DDS (10)

struct state {
    direnum_ctx_t dds[MAX_DDS];
    FILE         *fds[MAX_FDS];
};

static struct state state;

direnum_ctx_t dds[MAX_DDS];
FILE         *fds[MAX_FDS];

static char *get_fullpath(const char *path) {
    // Compose full path
    char *full_path = malloc(strlen(MOUNT_POINT) + 1 + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, "/");
    strcat(full_path, path);
    return full_path;
}

static int sd_open(uint8_t flags, const char *path) {
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
        if (state.fds[i] == NULL) {
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return ERR_TOO_MANY_OPEN;

    char *full_path = get_fullpath(path);
    led_flash_start();
    FILE *f = fopen(full_path, mode);
    led_flash_stop();
    int err_no = errno;
    free(full_path);

    if (f == NULL) {
        uint8_t err = ERR_NOT_FOUND;
        switch (err_no) {
            case EACCES: err = ERR_NOT_FOUND; break;
            case EEXIST: err = ERR_EXISTS; break;
            default: err = ERR_NOT_FOUND; break;
        }
        return err;
    }
    state.fds[fd] = f;
    return fd;
}

static int sd_close(int fd) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    fclose(f);
    state.fds[fd] = NULL;
    return 0;
}

static int sd_read(int fd, uint16_t size, void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    // Use RX buffer as temporary storage
    led_flash_start();
    int result = (int)fread(buf, 1, size, f);
    led_flash_stop();
    return (result < 0) ? ERR_OTHER : result;
}

static int sd_write(int fd, uint16_t size, const void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    led_flash_start();
    int result = (int)fwrite(buf, 1, size, f);
    led_flash_stop();
    return (result < 0) ? ERR_OTHER : result;
}

static int sd_seek(int fd, uint32_t offset) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    led_flash_start();
    int result = fseek(f, offset, SEEK_SET);
    led_flash_stop();
    return (result < 0) ? ERR_OTHER : 0;
}

static int sd_tell(int fd) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    led_flash_start();
    int result = ftell(f);
    led_flash_stop();
    return (result < 0) ? ERR_OTHER : result;
}

static int sd_opendir(const char *path) {
    // Find free directory descriptor
    int dd = -1;
    for (int i = 0; i < MAX_DDS; i++) {
        if (state.dds[i] == NULL) {
            dd = i;
            break;
        }
    }
    if (dd == -1)
        return ERR_TOO_MANY_OPEN;

    char *full_path = get_fullpath(path);

    led_flash_start();
    direnum_ctx_t ctx = direnum_open(full_path);
    led_flash_stop();
    free(full_path);

    if (ctx == NULL)
        return ERR_NOT_FOUND;

    // Return directory descriptor
    state.dds[dd] = ctx;

    return dd + MAX_FDS;
}

static int sd_closedir(int dd) {
    if (dd < MAX_FDS || dd >= MAX_FDS + MAX_DDS || state.dds[dd - MAX_FDS] == NULL)
        return ERR_PARAM;
    dd -= MAX_FDS;

    direnum_ctx_t ctx = state.dds[dd];
    led_flash_start();
    direnum_close(ctx);
    led_flash_stop();
    state.dds[dd] = NULL;
    return 0;
}

static int sd_readdir(int dd, struct direnum_ent *de) {
    if (dd < MAX_FDS || dd >= MAX_FDS + MAX_DDS || state.dds[dd - MAX_FDS] == NULL)
        return ERR_PARAM;
    dd -= MAX_FDS;

    direnum_ctx_t ctx = state.dds[dd];
    led_flash_start();
    int result = direnum_read(ctx, de) ? 0 : ERR_EOF;
    led_flash_stop();
    return result;
}

static int sd_delete(const char *path) {
    char *full_path = get_fullpath(path);

    led_flash_start();
    int result = unlink(full_path);
    if (result < 0) {
        result = rmdir(full_path);
    }
    led_flash_stop();
    int err_no = errno;
    free(full_path);

    if (result < 0) {
        // Error
        if (err_no == ENOTEMPTY) {
            return ERR_NOT_EMPTY;
        } else {
            return ERR_NOT_FOUND;
        }
    }
    return 0;
}

static int sd_rename(const char *path_old, const char *path_new) {
    char *full_old = get_fullpath(path_old);
    char *full_new = get_fullpath(path_new);

    led_flash_start();
    int result = rename(full_old, full_new);
    led_flash_stop();
    free(full_old);
    free(full_new);

    return (result < 0) ? ERR_NOT_FOUND : 0;
}

static int sd_mkdir(const char *path) {
    char *full_path = get_fullpath(path);

    led_flash_start();
#if _WIN32
    int result = mkdir(full_path);
#else
    int result = mkdir(full_path, 0775);
#endif
    led_flash_stop();
    free(full_path);

    return (result < 0) ? ERR_OTHER : 0;
}

static int sd_stat(const char *path, struct stat *st) {
    char *full_path = get_fullpath(path);
    led_flash_start();
    int result = stat(full_path, st);
    led_flash_stop();
    free(full_path);

    return result < 0 ? ERR_NOT_FOUND : 0;
}

struct vfs sdcard_vfs = {
    .open     = sd_open,
    .close    = sd_close,
    .read     = sd_read,
    .write    = sd_write,
    .seek     = sd_seek,
    .tell     = sd_tell,
    .opendir  = sd_opendir,
    .closedir = sd_closedir,
    .readdir  = sd_readdir,
    .delete   = sd_delete,
    .rename   = sd_rename,
    .mkdir    = sd_mkdir,
    .stat     = sd_stat,
};
