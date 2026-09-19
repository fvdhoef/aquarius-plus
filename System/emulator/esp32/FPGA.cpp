#include "FPGA.h"
#include "Keyboard.h"
#ifdef EMULATOR
#include "EmuState.h"
#else

#include <driver/spi_master.h>

static const char *TAG = "FPGA";

#define SPIBUS SPI3_HOST

#ifdef CONFIG_MACHINE_TYPE_AQPLUS
#define IOPIN_FPGA_INIT_B IOPIN_SPI_SSEL_N
#endif

#define MAX_TRANSFER_SIZE (1024)

#endif

enum {
    STATUS_KEYB = (1 << 0),
};

class FPGAInt : public FPGA {
public:
    SemaphoreHandle_t mutex;
#ifndef EMULATOR
    spi_device_handle_t cfgSpiDev;
    spi_device_handle_t cmdSpiDev;
#endif

    FPGAInt() {
    }

    void init() override {
        // Create semaphore for allowing to call FPGA function from different threads
        mutex = xSemaphoreCreateRecursiveMutex();

#ifndef EMULATOR
        // Configure SPI bus
        spi_bus_config_t bus_config = {
            .mosi_io_num     = IOPIN_SPI_MOSI,
            .miso_io_num     = IOPIN_SPI_MISO,
            .sclk_io_num     = IOPIN_SPI_SCLK,
            .quadwp_io_num   = -1,
            .quadhd_io_num   = -1,
            .max_transfer_sz = MAX_TRANSFER_SIZE,
        };
        ESP_ERROR_CHECK(spi_bus_initialize(SPIBUS, &bus_config, SPI_DMA_CH_AUTO));

        // SPI configuration during sending bitstream
        {
            spi_device_interface_config_t cfg = {
                .mode           = 0,
                .clock_speed_hz = 20000000,
                .spics_io_num   = -1,
                .queue_size     = 7,
            };
            ESP_ERROR_CHECK(spi_bus_add_device(SPIBUS, &cfg, &cfgSpiDev));
        }

        // SPI configuration when communicating with FPGA core
        {
            spi_device_interface_config_t cfg = {
                .mode           = 0,
                .clock_speed_hz = CONFIG_FPGA_SPI_SPEED_HZ,
                .spics_io_num   = -1,
                .queue_size     = 7,
            };
            ESP_ERROR_CHECK(spi_bus_add_device(SPIBUS, &cfg, &cmdSpiDev));
        }

        // Configure FPGA PROG# line
        {
            gpio_config_t cfg = {
                .pin_bit_mask = (1ULL << IOPIN_FPGA_PROG_N),
                .mode         = GPIO_MODE_OUTPUT,
            };
            gpio_config(&cfg);
            gpio_set_level(IOPIN_FPGA_PROG_N, 1);
        }

        // Configure FPGA DONE line
        {
            gpio_config_t cfg = {
                .pin_bit_mask = (1ULL << IOPIN_FPGA_DONE),
                .mode         = GPIO_MODE_INPUT,
                .pull_up_en   = GPIO_PULLUP_ENABLE,
            };
            gpio_config(&cfg);
        }

        // Configure SPI_SSEL_N line
        {
            gpio_config_t cfg = {
                .pin_bit_mask = (1ULL << IOPIN_SPI_SSEL_N),
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
                .mode = GPIO_MODE_INPUT,
#else
                .mode = GPIO_MODE_OUTPUT,
#endif
            };
            gpio_config(&cfg);
            gpio_set_level(IOPIN_SPI_SSEL_N, 1);
        }

        // Configure SPI_NOTIFY line
        {
            gpio_config_t cfg = {
                .pin_bit_mask = (1ULL << IOPIN_SPI_NOTIFY),
                .mode         = GPIO_MODE_INPUT,
                .intr_type    = GPIO_INTR_POSEDGE,
            };
            gpio_config(&cfg);

            gpio_install_isr_service(0);
            gpio_isr_handler_add(IOPIN_SPI_NOTIFY, spiNotifyIrqHandler, this);
        }
#endif
    }

    bool loadBitstream(const void *data, size_t length) override {
        RecursiveMutexLock lock(mutex);
        ESP_LOGI(TAG, "Starting configuration");

#ifdef EMULATOR
        EmuState::loadCore((const char *)data);
        if (EmuState::get())
            return true;
        return false;
#else
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        // Set FPGA INIT# as input
        gpio_set_direction(IOPIN_FPGA_INIT_B, GPIO_MODE_INPUT);
#endif

        // Pulse PROG_N to start configuration process
        gpio_set_level(IOPIN_FPGA_PROG_N, 0);
        esp_rom_delay_us(10);
        gpio_set_level(IOPIN_FPGA_PROG_N, 1);
        esp_rom_delay_us(10);

#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        // Wait for INIT_B to become high
        for (int i = 0; i < 100; i++) {
            vTaskDelay(pdMS_TO_TICKS(1));
            if (gpio_get_level(IOPIN_FPGA_INIT_B))
                break;
        }
        if (!gpio_get_level(IOPIN_FPGA_INIT_B)) {
            ESP_LOGE(TAG, "Error: INIT_B didn't become high, aborting!");
            return false;
        }
#endif

        // Send bitstream
        ESP_LOGI(TAG, "Sending bitstream");
        cfgTx(data, length);

        // Keep sending clock pulses until DONE becomes high
        ESP_LOGI(TAG, "Sending extra clock cycles");
        for (int i = 0; i < 1000; i++) {
            if (gpio_get_level(IOPIN_FPGA_DONE))
                break;

            uint8_t val = 0;
            cfgTx(&val, 1);
        }
        if (!gpio_get_level(IOPIN_FPGA_DONE)) {
            ESP_LOGE(TAG, "Error: DONE didn't become high, aborting!");
            return false;
        }
        ESP_LOGI(TAG, "Configuration completed");

#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        // Configure INITB# (same as SSEL#) as output
        gpio_set_level(IOPIN_SPI_SSEL_N, 1);
        gpio_set_direction(IOPIN_SPI_SSEL_N, GPIO_MODE_OUTPUT);
#endif

        return true;
#endif
    }

#ifndef EMULATOR
    void cfgTx(const void *data, size_t length) {
        unsigned       remaining = length;
        const uint8_t *p         = (const uint8_t *)data;
        while (remaining > 0) {
            unsigned txsize = remaining;
            if (txsize > MAX_TRANSFER_SIZE)
                txsize = MAX_TRANSFER_SIZE;

            spi_transaction_t t = {0};
            t.length            = txsize * 8;
            t.tx_buffer         = p;
            ESP_ERROR_CHECK(spi_device_transmit(cfgSpiDev, &t));

            p += txsize;
            remaining -= txsize;
        }
    }
#endif

    void spiSel(bool enable) override {
#ifndef EMULATOR
        gpio_set_level(IOPIN_SPI_SSEL_N, !enable);
#else
        auto emuState = EmuState::get();
        if (emuState) {
            emuState->spiSel(enable);
        }
#endif
    }

    void spiTx(const void *data, size_t length) override {
#ifndef EMULATOR
        unsigned       remaining = length;
        const uint8_t *p         = (const uint8_t *)data;
        while (remaining > 0) {
            unsigned txsize = remaining;
            if (txsize > MAX_TRANSFER_SIZE)
                txsize = MAX_TRANSFER_SIZE;

            spi_transaction_t t = {0};
            t.length            = txsize * 8;
            t.tx_buffer         = p;
            ESP_ERROR_CHECK(spi_device_transmit(cmdSpiDev, &t));

            p += txsize;
            remaining -= txsize;
        }
#else
        auto emuState = EmuState::get();
        if (emuState) {
            emuState->spiTx(data, length);
        }
#endif
    }

    void spiRx(void *buf, size_t length) override {
#ifndef EMULATOR
        spi_transaction_t t = {0};
        t.length            = length * 8;
        t.rxlength          = length * 8;
        t.rx_buffer         = buf;
        ESP_ERROR_CHECK(spi_device_transmit(cmdSpiDev, &t));
#else
        auto emuState = EmuState::get();
        if (emuState) {
            emuState->spiRx(buf, length);
        } else {
            memset(buf, 0, length);
        }
#endif
    }

#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
    uint64_t getKeys() override {
        uint64_t result;

        RecursiveMutexLock lock(mutex);
        spiSel(true);

        uint8_t cmd[] = {CMD_GET_KEYS, 0};
        spiTx(cmd, sizeof(cmd));
        spiRx(&result, sizeof(result));

        spiSel(false);

        return result;
    }

    void setVolume(uint16_t volume, bool spkEn) override {
        RecursiveMutexLock lock(mutex);
        spiSel(true);
        uint8_t cmd[] = {CMD_SET_VOLUME, (uint8_t)(volume & 0xFF), (uint8_t)(volume >> 8), (uint8_t)(spkEn ? 1 : 0)};
        spiTx(cmd, sizeof(cmd));
        spiSel(false);
    }
#endif

    bool getCoreInfo(CoreInfo *info) override {
        RecursiveMutexLock lock(mutex);

        bool ok = false;

        for (int i = 0; i < 10 && !ok; i++) {
            // Sysinfo
            {
                uint8_t rxData[8];
                uint8_t cmd[] = {CMD_GET_SYSINFO, 0};
                spiSel(true);
                spiTx(cmd, sizeof(cmd));
                spiRx(rxData, 8);
                spiSel(false);

                info->coreType     = rxData[0];
                info->flags        = rxData[1];
                info->versionMajor = rxData[2];
                info->versionMinor = rxData[3];

                if (info->coreType == 0) {
                    // FPGA not yet ready, wait a moment and try again
                    vTaskDelay(pdMS_TO_TICKS(50));
                    continue;
                }
                ok = true;
            }

            // Name1
            {
                uint8_t cmd[] = {CMD_GET_NAME1, 0};

                spiSel(true);
                spiTx(cmd, sizeof(cmd));
                spiRx(info->name, 8);
                spiSel(false);
            }

            // Name2
            {
                uint8_t cmd[] = {CMD_GET_NAME2, 0};

                spiSel(true);
                spiTx(cmd, sizeof(cmd));
                spiRx(info->name + 8, 8);
                spiSel(false);
            }
            info->name[16] = 0;

            snprintf(info->name, sizeof(info->name), "%s", trim(info->name).c_str());
        }

        if (ok)
            ESP_LOGI(TAG, "CoreInfo type=%02X flags=%02X version=%d.%02d name=%s", info->coreType, info->flags, info->versionMajor, info->versionMinor, info->name);

        return ok;
    }

    void setOverlayText(const uint16_t buf[1024]) override {
        RecursiveMutexLock lock(mutex);
        spiSel(true);
        uint8_t cmd[] = {CMD_OVL_TEXT};
        spiTx(cmd, sizeof(cmd));
        spiTx(buf, 2 * 1024);
        spiSel(false);
    }

    void setOverlayFont(const uint8_t buf[2048]) override {
        RecursiveMutexLock lock(mutex);
        spiSel(true);
        uint8_t cmd[] = {CMD_OVL_FONT};
        spiTx(cmd, sizeof(cmd));
        spiTx(buf, 2048);
        spiSel(false);
    }

    void setOverlayPalette(const uint16_t buf[16]) override {
        RecursiveMutexLock lock(mutex);
        spiSel(true);
        uint8_t cmd[] = {CMD_OVL_PALETTE};
        spiTx(cmd, sizeof(cmd));
        spiTx(buf, 2 * 16);
        spiSel(false);
    }

#ifndef EMULATOR
    uint8_t getStatus() {
        RecursiveMutexLock lock(mutex);
        spiSel(true);

        uint8_t cmd[] = {CMD_GET_STATUS, 0};
        spiTx(cmd, sizeof(cmd));

        uint8_t result;
        spiRx(&result, 1);

        spiSel(false);

        return result;
    }

    static void spiNotifyIrqHandler(void *arg) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerPendFunctionCallFromISR(spiNotifyHandler, arg, 0, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    static void spiNotifyHandler(void *arg1, uint32_t) { static_cast<FPGAInt *>(arg1)->spiNotified(); }

    void spiNotified() {
        uint8_t status = getStatus();

        if (status & STATUS_KEYB) {
#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
            Keyboard::instance()->updateKeys(getKeys());
#endif
        }
    }
#endif

    SemaphoreHandle_t getMutex() override {
        return mutex;
    }
};

FPGA *FPGA::instance() {
    static FPGAInt obj;
    return &obj;
}
