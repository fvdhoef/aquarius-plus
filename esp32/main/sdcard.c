#include "sdcard.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <dirent.h>

static const char *TAG = "sdcard";

#if CONFIG_IDF_TARGET_ESP32S2
#    define SPI_DMA_CHAN host.slot
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
    slot_config.gpio_wp               = SDSPI_SLOT_NO_WP;
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
