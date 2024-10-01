#include "FPGA.h"
#include <driver/spi_master.h>
#include "Keyboard.h"

static const char *TAG = "FPGA";

#define SPIBUS            SPI3_HOST // SPI2 host is used by SD card interface
#define IOPIN_FPGA_INIT_B IOPIN_SPI_SSEL_N

#define MAX_TRANSFER_SIZE (1024)

enum {
    // Aq+ command
    CMD_RESET           = 0x01,
    CMD_SET_KEYB_MATRIX = 0x10,
    CMD_SET_HCTRL       = 0x11,
    CMD_WRITE_KBBUF     = 0x12,
    CMD_BUS_ACQUIRE     = 0x20,
    CMD_BUS_RELEASE     = 0x21,
    CMD_MEM_WRITE       = 0x22,
    CMD_MEM_READ        = 0x23,
    CMD_IO_WRITE        = 0x24,
    CMD_IO_READ         = 0x25,
    CMD_ROM_WRITE       = 0x30,
    CMD_SET_VIDMODE     = 0x40,

    // General commands
    CMD_GET_KEYS    = 0xF1,
    CMD_OVL_TEXT    = 0xF4,
    CMD_OVL_FONT    = 0xF5,
    CMD_OVL_PALETTE = 0xF6,
    CMD_GET_STATUS  = 0xF7,
};

enum {
    STATUS_KEYB = (1 << 0),
};

class FPGAInt : public FPGA {
public:
    SemaphoreHandle_t   mutex;
    spi_device_handle_t cfgSpiDev;
    spi_device_handle_t cmdSpiDev;

    FPGAInt() {
    }

    void init() override {
        // Create semaphore for allowing to call FPGA function from different threads
        mutex = xSemaphoreCreateRecursiveMutex();

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
                .clock_speed_hz = 1000000,
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

        // Configure FPGA DONE
        {
            gpio_config_t cfg = {
                .pin_bit_mask = (1ULL << IOPIN_FPGA_DONE),
                .mode         = GPIO_MODE_INPUT,
            };
            gpio_config(&cfg);
        }

        // Configure SPI_SSEL_N line (also used as INIT# during configuration)
        {
            gpio_config_t cfg = {
                .pin_bit_mask = (1ULL << IOPIN_SPI_SSEL_N),
                .mode         = GPIO_MODE_INPUT,
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
    }

    bool loadBitstream(const void *data, size_t length) override {
        RecursiveMutexLock lock(mutex);
        ESP_LOGI(TAG, "Starting configuration");

        // Set FPGA INIT# as input
        gpio_set_direction(IOPIN_FPGA_INIT_B, GPIO_MODE_INPUT);

        // Pulse PROG_B to start configuration process
        gpio_set_level(IOPIN_FPGA_PROG_N, 0);
        esp_rom_delay_us(10);
        gpio_set_level(IOPIN_FPGA_PROG_N, 1);

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

        // Configure INITB# (same as CS#) as output
        gpio_set_level(IOPIN_SPI_SSEL_N, 1);
        gpio_set_direction(IOPIN_SPI_SSEL_N, GPIO_MODE_OUTPUT);

        return true;
    }

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

    void spiSel(bool enable) override {
        gpio_set_level(IOPIN_SPI_SSEL_N, !enable);
    }

    void spiTx(const void *data, size_t length) override {
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
    }

    void spiRx(void *buf, size_t length) override {
        spi_transaction_t t = {0};
        t.length            = length * 8;
        t.rxlength          = length * 8;
        t.rx_buffer         = buf;
        ESP_ERROR_CHECK(spi_device_transmit(cmdSpiDev, &t));
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
            //     uint8_t keys[14];
            //     getKeys(keys);
            //     getKeyboard()->updateKeys(keys);
        }
    }

    SemaphoreHandle_t getMutex() override {
        return mutex;
    }
};

FPGA *getFPGA() {
    static FPGAInt obj;
    return &obj;
}
