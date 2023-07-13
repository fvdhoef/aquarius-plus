#include "fpga.h"
#include <driver/spi_master.h>
#include <freertos/task.h>

static const char *TAG = "fpga";

static SemaphoreHandle_t mutex;

extern const uint8_t fpga_image_start[] asm("_binary_top_bit_start");
extern const uint8_t fpga_image_end[] asm("_binary_top_bit_end");

static spi_device_handle_t fpga_spidev;
static spi_device_handle_t fpga_spidev_regs;

static uint8_t saved_banks[4];
static uint8_t cur_banks[4];

#define SPIBUS SPI3_HOST // SPI2 host is used by SD card interface
#define IOPIN_FPGA_INIT_B IOPIN_SPI_CS_N

#define MAX_TRANSFER_SIZE (1024)

enum {
    CMD_RESET           = 0x01,
    CMD_SET_KEYB_MATRIX = 0x10,
    CMD_SET_HCTRL       = 0x11,
    CMD_BUS_ACQUIRE     = 0x20,
    CMD_BUS_RELEASE     = 0x21,
    CMD_MEM_WRITE       = 0x22,
    CMD_MEM_READ        = 0x23,
    CMD_IO_WRITE        = 0x24,
    CMD_IO_READ         = 0x25,
};

void fpga_init(void) {
    // IOPIN_ESP_NOTIFY  = 12,
    // IOPIN_FPGA_PROG_N = 34,
    mutex = xSemaphoreCreateRecursiveMutex();
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

    {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << IOPIN_FPGA_DONE) | (1ULL << IOPIN_FPGA_INIT_B),
            .mode         = GPIO_MODE_INPUT,
        };
        gpio_config(&io_conf);
    }
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << IOPIN_FPGA_PROG_N),
            .mode         = GPIO_MODE_OUTPUT,
        };
        gpio_config(&io_conf);
        gpio_set_level(IOPIN_FPGA_PROG_N, 1);
    }

    spi_bus_config_t bus_config = {
        .mosi_io_num     = IOPIN_SPI_MOSI,
        .miso_io_num     = IOPIN_SPI_MISO,
        .sclk_io_num     = IOPIN_SPI_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = MAX_TRANSFER_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPIBUS, &bus_config, SPI_DMA_CH_AUTO));

    {
        spi_device_interface_config_t dev_config = {
            .clock_speed_hz = 20000000,
            .mode           = 0,
            .spics_io_num   = -1,
            .queue_size     = 7,

            // .flags = SPI_DEVICE_TXBIT_LSBFIRST,
        };
        ESP_ERROR_CHECK(spi_bus_add_device(SPIBUS, &dev_config, &fpga_spidev));
    }
    {
        spi_device_interface_config_t dev_config = {
            .clock_speed_hz = 1000000,
            .mode           = 0,
            .spics_io_num   = -1,
            .queue_size     = 7,

            // .flags = SPI_DEVICE_TXBIT_LSBFIRST,
        };
        ESP_ERROR_CHECK(spi_bus_add_device(SPIBUS, &dev_config, &fpga_spidev_regs));
    }

    ESP_LOGI(TAG, "Starting configuration");

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
        return;
    }

    ESP_LOGI(TAG, "Sending bitstream");

    // Send bitstream
    unsigned       image_len = fpga_image_end - fpga_image_start;
    const uint8_t *image_p   = fpga_image_start;
    unsigned       remaining = image_len;

    while (remaining > 0) {
        unsigned txsize = remaining;
        if (txsize > MAX_TRANSFER_SIZE)
            txsize = MAX_TRANSFER_SIZE;

        spi_transaction_t t = {
            .length    = txsize * 8,
            .tx_buffer = image_p,
        };
        ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev, &t));

        image_p += txsize;
        remaining -= txsize;
    }

    ESP_LOGI(TAG, "Sending extra clock cycles");

    // Keep sending clock pulses until DONE becomes high
    for (int i = 0; i < 1000; i++) {
        if (gpio_get_level(IOPIN_FPGA_DONE))
            break;

        spi_transaction_t t = {
            .length = 8,
            .flags  = SPI_TRANS_USE_TXDATA,
        };
        ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev, &t));
    }
    if (!gpio_get_level(IOPIN_FPGA_DONE)) {
        ESP_LOGE(TAG, "Error: DONE didn't become high, aborting!");
        return;
    }
    ESP_LOGI(TAG, "Configuration completed");

    {
        gpio_set_level(IOPIN_SPI_CS_N, 1);
        gpio_set_direction(IOPIN_SPI_CS_N, GPIO_MODE_OUTPUT);
    }
    xSemaphoreGiveRecursive(mutex);
}

void fpga_reset_req(void) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    spi_transaction_t t = {.length = 8, .tx_data[0] = CMD_RESET, .flags = SPI_TRANS_USE_TXDATA};
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

void fpga_update_keyb_matrix(uint8_t *keyb_matrix) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    uint8_t buf[9];
    buf[0] = CMD_SET_KEYB_MATRIX;
    memcpy(&buf[1], keyb_matrix, 8);

    // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, sizeof(buf), ESP_LOG_INFO);

    spi_transaction_t t = {.length = sizeof(buf) * 8, .tx_buffer = buf};
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

void fpga_update_handctrl(uint8_t hctrl1, uint8_t hctrl2) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    uint8_t buf[3];
    buf[0] = CMD_SET_HCTRL;
    buf[1] = hctrl1;
    buf[2] = hctrl2;

    // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, sizeof(buf), ESP_LOG_INFO);

    spi_transaction_t t = {.length = sizeof(buf) * 8, .tx_buffer = buf};
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

void fpga_bus_acquire(void) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    spi_transaction_t t = {.length = 8, .tx_data[0] = CMD_BUS_ACQUIRE, .flags = SPI_TRANS_USE_TXDATA};
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

void fpga_bus_release(void) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    spi_transaction_t t = {.length = 8, .tx_data[0] = CMD_BUS_RELEASE, .flags = SPI_TRANS_USE_TXDATA};
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

void fpga_mem_write(uint16_t addr, uint8_t data) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    spi_transaction_t t = {
        .length  = 4 * 8,
        .tx_data = {
            CMD_MEM_WRITE,
            (addr >> 0) & 0xFF,
            (addr >> 8) & 0xFF,
            data,
        },
        .flags = SPI_TRANS_USE_TXDATA,
    };
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

uint8_t fpga_mem_read(uint16_t addr) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    {
        spi_transaction_t t = {
            .length  = 4 * 8,
            .tx_data = {
                CMD_MEM_READ,
                (addr >> 0) & 0xFF,
                (addr >> 8) & 0xFF,
                0x00,
            },
            .flags = SPI_TRANS_USE_TXDATA,
        };
        ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));
    }

    uint8_t result = 0;
    {
        spi_transaction_t t = {
            .length   = 8,
            .rxlength = 8,
            .flags    = SPI_TRANS_USE_RXDATA,
        };
        ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

        result = t.rx_data[0];
    }

    gpio_set_level(IOPIN_SPI_CS_N, 1);

    xSemaphoreGiveRecursive(mutex);
    return result;
}

void fpga_io_write(uint16_t addr, uint8_t data) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    spi_transaction_t t = {
        .length  = 4 * 8,
        .tx_data = {
            CMD_IO_WRITE,
            (addr >> 0) & 0xFF,
            (addr >> 8) & 0xFF,
            data,
        },
        .flags = SPI_TRANS_USE_TXDATA,
    };
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
}

uint8_t fpga_io_read(uint16_t addr) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    gpio_set_level(IOPIN_SPI_CS_N, 0);

    {
        spi_transaction_t t = {
            .length  = 4 * 8,
            .tx_data = {
                CMD_IO_READ,
                (addr >> 0) & 0xFF,
                (addr >> 8) & 0xFF,
                0x00,
            },
            .flags = SPI_TRANS_USE_TXDATA,
        };
        ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));
    }

    uint8_t result = 0;
    {
        spi_transaction_t t = {
            .length   = 8,
            .rxlength = 8,
            .flags    = SPI_TRANS_USE_RXDATA,
        };
        ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));

        result = t.rx_data[0];
    }

    gpio_set_level(IOPIN_SPI_CS_N, 1);
    xSemaphoreGiveRecursive(mutex);
    return result;
}

void fpga_save_banks(void) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    // Save bank registers
    for (int i = 0; i < 4; i++) {
        cur_banks[i] = saved_banks[i] = fpga_io_read(IO_BANK0 + i);
    }
    xSemaphoreGiveRecursive(mutex);
}

void fpga_restore_banks(void) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    // Restore bank registers
    for (int i = 0; i < 4; i++) {
        fpga_io_write(IO_BANK0 + i, saved_banks[i]);
    }
    xSemaphoreGiveRecursive(mutex);
}

void fpga_set_bank(unsigned bank, uint8_t val) {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    if (cur_banks[bank] != val) {
        fpga_io_write(IO_BANK0 + bank, val);
        cur_banks[bank] = val;
    }
    xSemaphoreGiveRecursive(mutex);
}
