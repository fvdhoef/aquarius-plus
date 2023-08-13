#include "FPGA.h"

static const char *TAG = "FPGA";

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
    CMD_ROM_WRITE       = 0x30,
};

FPGA::FPGA() {
}

FPGA &FPGA::instance() {
    static FPGA fpga;
    return fpga;
}

void FPGA::init() {
    // IOPIN_ESP_NOTIFY  = 12,
    // IOPIN_FPGA_PROG_N = 34,

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
    spi_device_interface_config_t fpgaDevCfg = {
        .mode           = 0,
        .clock_speed_hz = 20000000,
        .spics_io_num   = -1,
        .queue_size     = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPIBUS, &fpgaDevCfg, &fpgaSpiDev));

    // SPI configuration when communicating with AQ+ FPGA core
    spi_device_interface_config_t aqpDevCfg = {
        .mode           = 0,
        .clock_speed_hz = 1000000,
        .spics_io_num   = -1,
        .queue_size     = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPIBUS, &aqpDevCfg, &aqpSpiDev));

    // Configure FPGA PROG# line
    {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = (1ULL << IOPIN_FPGA_PROG_N),
            .mode         = GPIO_MODE_OUTPUT,
        };
        gpio_config(&gpioCfg);
        gpio_set_level((gpio_num_t)IOPIN_FPGA_PROG_N, 1);
    }

    // Configure FPGA DONE and INIT# lines
    {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = (1ULL << IOPIN_FPGA_DONE) | (1ULL << IOPIN_FPGA_INIT_B),
            .mode         = GPIO_MODE_INPUT,
        };
        gpio_config(&gpioCfg);
    }
}

bool FPGA::loadBitstream(const void *data, size_t length) {
    RecursiveMutexLock lock(mutex);

    ESP_LOGI(TAG, "Starting configuration");

    // Set FPGA INIT# as input
    gpio_set_direction((gpio_num_t)IOPIN_FPGA_INIT_B, GPIO_MODE_INPUT);

    // Pulse PROG_B to start configuration process
    gpio_set_level((gpio_num_t)IOPIN_FPGA_PROG_N, 0);
    esp_rom_delay_us(10);
    gpio_set_level((gpio_num_t)IOPIN_FPGA_PROG_N, 1);

    // Wait for INIT_B to become high
    for (int i = 0; i < 100; i++) {
        vTaskDelay(pdMS_TO_TICKS(1));
        if (gpio_get_level((gpio_num_t)IOPIN_FPGA_INIT_B))
            break;
    }
    if (!gpio_get_level((gpio_num_t)IOPIN_FPGA_INIT_B)) {
        ESP_LOGE(TAG, "Error: INIT_B didn't become high, aborting!");
        return false;
    }

    // Send bitstream
    ESP_LOGI(TAG, "Sending bitstream");
    fpgaTx(data, length);

    // Keep sending clock pulses until DONE becomes high
    ESP_LOGI(TAG, "Sending extra clock cycles");
    for (int i = 0; i < 1000; i++) {
        if (gpio_get_level((gpio_num_t)IOPIN_FPGA_DONE))
            break;

        uint8_t val = 0;
        fpgaTx(&val, 1);
    }
    if (!gpio_get_level((gpio_num_t)IOPIN_FPGA_DONE)) {
        ESP_LOGE(TAG, "Error: DONE didn't become high, aborting!");
        return false;
    }
    ESP_LOGI(TAG, "Configuration completed");

    // Configure INITB# (same as CS#) as output
    gpio_set_level((gpio_num_t)IOPIN_SPI_CS_N, 1);
    gpio_set_direction((gpio_num_t)IOPIN_SPI_CS_N, GPIO_MODE_OUTPUT);

    return true;
}

void FPGA::fpgaTx(const void *data, size_t length) {
    unsigned       remaining = length;
    const uint8_t *p         = (const uint8_t *)data;
    while (remaining > 0) {
        unsigned txsize = remaining;
        if (txsize > MAX_TRANSFER_SIZE)
            txsize = MAX_TRANSFER_SIZE;

        spi_transaction_t t = {0};
        t.length            = txsize * 8;
        t.tx_buffer         = p;
        ESP_ERROR_CHECK(spi_device_transmit(fpgaSpiDev, &t));

        p += txsize;
        remaining -= txsize;
    }
}

void FPGA::aqpSel(bool enable) {
    gpio_set_level((gpio_num_t)IOPIN_SPI_CS_N, !enable);
}

void FPGA::aqpTx(int data0, int data1, int data2, int data3) {
    spi_transaction_t t = {0};
    t.flags             = SPI_TRANS_USE_TXDATA;
    if (data0 >= 0) {
        t.tx_data[0] = data0;
        t.length += 8;

        if (data1 >= 0) {
            t.tx_data[1] = data1;
            t.length += 8;

            if (data2 >= 0) {
                t.tx_data[2] = data2;
                t.length += 8;

                if (data3 >= 0) {
                    t.tx_data[3] = data3;
                    t.length += 8;
                }
            }
        }
    }
    ESP_ERROR_CHECK(spi_device_transmit(aqpSpiDev, &t));
}

void FPGA::aqpTx(const void *data, size_t length) {
    spi_transaction_t t = {0};
    t.length            = length * 8;
    t.tx_buffer         = data;
    ESP_ERROR_CHECK(spi_device_transmit(aqpSpiDev, &t));
}

void FPGA::aqpRx(void *buf, size_t length) {
    spi_transaction_t t = {0};
    t.length            = length * 8;
    t.rxlength          = length * 8;
    t.rx_buffer         = buf;
    ESP_ERROR_CHECK(spi_device_transmit(aqpSpiDev, &t));
}

void FPGA::aqpReset() {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_RESET);
    aqpSel(false);
}

void FPGA::aqpUpdateKeybMatrix(uint8_t *keyb_matrix) {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    uint8_t buf[9];
    buf[0] = CMD_SET_KEYB_MATRIX;
    memcpy(&buf[1], keyb_matrix, 8);
    aqpTx(buf, sizeof(buf));
    aqpSel(false);
}

void FPGA::aqpUpdateHandCtrl(uint8_t hctrl1, uint8_t hctrl2, TickType_t ticks_to_wait) {
    if (xSemaphoreTakeRecursive(mutex, ticks_to_wait) != pdTRUE)
        return;
    aqpSel(true);
    aqpTx(CMD_SET_HCTRL, hctrl1, hctrl2);
    aqpSel(false);
    xSemaphoreGiveRecursive(mutex);
}

void FPGA::aqpAqcuireBus() {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_BUS_ACQUIRE);
    aqpSel(false);
}

void FPGA::aqpReleaseBus() {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_BUS_RELEASE);
    aqpSel(false);
}

void FPGA::aqpWriteMem(uint16_t addr, uint8_t data) {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_MEM_WRITE, addr & 0xFF, addr >> 8, data);
    aqpSel(false);
}

uint8_t FPGA::aqpReadMem(uint16_t addr) {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_MEM_READ, addr & 0xFF, addr >> 8);

    uint8_t result[2];
    aqpRx(result, 2);
    aqpSel(false);
    return result[1];
}

void FPGA::aqpWriteIO(uint16_t addr, uint8_t data) {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_IO_WRITE, addr & 0xFF, addr >> 8, data);
    aqpSel(false);
}

uint8_t FPGA::aqpReadIO(uint16_t addr) {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_IO_READ, addr & 0xFF, addr >> 8);

    uint8_t result[2];
    aqpRx(result, 2);
    aqpSel(false);
    return result[1];
}

void FPGA::aqpWriteROM(uint16_t addr, uint8_t data) {
    RecursiveMutexLock lock(mutex);
    aqpSel(true);
    aqpTx(CMD_ROM_WRITE, addr & 0xFF, addr >> 8, data);
    aqpSel(false);
}

void FPGA::aqpSaveMemBanks() {
    RecursiveMutexLock lock(mutex);
    for (int i = 0; i < 4; i++) {
        cur_banks[i] = saved_banks[i] = aqpReadIO(IO_BANK0 + i);
    }
}

void FPGA::aqpRestoreMemBanks() {
    RecursiveMutexLock lock(mutex);
    for (int i = 0; i < 4; i++) {
        aqpWriteIO(IO_BANK0 + i, saved_banks[i]);
    }
}

void FPGA::aqpSetMemBank(unsigned bank, uint8_t val) {
    RecursiveMutexLock lock(mutex);
    if (cur_banks[bank] != val) {
        aqpWriteIO(IO_BANK0 + bank, val);
        cur_banks[bank] = val;
    }
}
