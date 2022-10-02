#include "fpga.h"
#include <driver/spi_master.h>
#include <freertos/task.h>

static const char *TAG = "fpga";

extern const uint8_t fpga_image_start[] asm("_binary_top_bit_start");
extern const uint8_t fpga_image_end[] asm("_binary_top_bit_end");

static spi_device_handle_t fpga_spidev;
static spi_device_handle_t fpga_spidev_regs;

#define SPIBUS SPI2_HOST // SPI3 host is used by SD card interface
#define IOPIN_FPGA_INIT_B IOPIN_SPI_CS_N

#define MAX_TRANSFER_SIZE (1024)

void fpga_init(void) {
    // IOPIN_ESP_NOTIFY  = 12,
    // IOPIN_FPGA_PROG_N = 34,

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
    ets_delay_us(10);
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
}

void fpga_update_keyb_matrix(uint8_t *buf) {
    uint8_t data[9];
    data[0] = 0x10;
    memcpy(&data[1], buf, 8);

    ESP_LOG_BUFFER_HEX(TAG, data, sizeof(data));

    spi_transaction_t t = {.length = sizeof(data) * 8, .tx_buffer = data};

    gpio_set_level(IOPIN_SPI_CS_N, 0);
    ESP_ERROR_CHECK(spi_device_transmit(fpga_spidev_regs, &t));
    gpio_set_level(IOPIN_SPI_CS_N, 1);
}
