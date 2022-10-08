#include "flash.h"
#include "fpga.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "flash";

static uint8_t saved_banks[4];
static uint8_t cur_banks[4];

void flash_prepare(void) {
    // Save bank registers
    for (int i = 0; i < 4; i++) {
        cur_banks[i] = saved_banks[i] = fpga_io_read(IO_BANK0 + i);
    }
}

void flash_finish(void) {
    // Restore bank registers
    for (int i = 0; i < 4; i++) {
        fpga_io_write(IO_BANK0 + i, saved_banks[i]);
    }
}

static void set_bank(unsigned bank, uint8_t val) {
    if (cur_banks[bank] == val)
        return;
    fpga_io_write(IO_BANK0 + bank, val);
    cur_banks[bank] = val;
}

void flash_program(unsigned addr, uint8_t val) {
    if (addr >= 256 * 1024) {
        ESP_LOGE(TAG, "flash_program - addr out of range! (0x%X)", addr);
        return;
    }

    // Remap memory for flash write
    set_bank(0, 0);
    set_bank(1, 1);
    set_bank(2, addr >> 14);

    // Byte-program
    fpga_mem_write(0x5555, 0xAA);
    fpga_mem_write(0x2AAA, 0x55);
    fpga_mem_write(0x5555, 0xA0);
    fpga_mem_write(0x8000 + (addr & 0x3FFF), val);

    ets_delay_us(50);
}

void flash_erase_4kb_sector(unsigned addr) {
    if (addr >= 256 * 1024 || (addr & 4095) != 0) {
        ESP_LOGE(TAG, "flash_erase_4kb_sector - invalid addr! (0x%X)", addr);
        return;
    }

    ESP_LOGI(TAG, "Erasing sector @ 0x%X", addr);

    // Remap memory for flash write
    set_bank(0, 0);
    set_bank(1, 1);
    set_bank(2, addr >> 14);

    // Sector erase
    fpga_mem_write(0x5555, 0xAA);
    fpga_mem_write(0x2AAA, 0x55);
    fpga_mem_write(0x5555, 0x80);
    fpga_mem_write(0x5555, 0xAA);
    fpga_mem_write(0x2AAA, 0x55);
    fpga_mem_write(0x8000 + (addr & 0x3FFF), 0x30);

    // Wait for operation to complete
    vTaskDelay(pdMS_TO_TICKS(50));
}

void flash_erase_chip(void) {
    ESP_LOGI(TAG, "Erasing chip");

    // Remap memory for flash write
    set_bank(0, 0);
    set_bank(1, 1);

    // Sector erase
    fpga_mem_write(0x5555, 0xAA);
    fpga_mem_write(0x2AAA, 0x55);
    fpga_mem_write(0x5555, 0x80);
    fpga_mem_write(0x5555, 0xAA);
    fpga_mem_write(0x2AAA, 0x55);
    fpga_mem_write(0x5555, 0x10);

    // Wait for operation to complete
    vTaskDelay(pdMS_TO_TICKS(200));
}
