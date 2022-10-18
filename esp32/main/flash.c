#include "flash.h"
#include "fpga.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "screen.h"

static const char *TAG = "flash";

#if 1
extern const uint8_t rom_image_start[] asm("_binary_aquarius_rom_start");
extern const uint8_t rom_image_end[] asm("_binary_aquarius_rom_end");
#else
extern const uint8_t rom_image_start[] asm("_binary_aquarius_s2_rom_start");
extern const uint8_t rom_image_end[] asm("_binary_aquarius_s2_rom_end");
#endif

void flash_prepare(void) {
    fpga_save_banks();
}

void flash_finish(void) {
    fpga_restore_banks();
}

void flash_program(unsigned addr, uint8_t val) {
    if (addr >= 256 * 1024) {
        ESP_LOGE(TAG, "flash_program - addr out of range! (0x%X)", addr);
        return;
    }
    if (val == 0xFF) {
        // Erased byte is already 0xFF
        return;
    }

    // Remap memory for flash write
    fpga_set_bank(0, 0);
    fpga_set_bank(1, 1);
    fpga_set_bank(2, addr >> 14);

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
    fpga_set_bank(0, 0);
    fpga_set_bank(1, 1);
    fpga_set_bank(2, addr >> 14);

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
    fpga_set_bank(0, 0);
    fpga_set_bank(1, 1);

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

bool verify_sysrom(void) {
    bool ok = true;

    fpga_bus_acquire();
    fpga_save_banks();

    screen_save();
    screen_show_msg("Verifying system ROM");
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "* Verifying boot ROM *");
    {
        uint8_t saved_bank = fpga_io_read(IO_BANK0);

        unsigned       addr = 0;
        const uint8_t *p    = rom_image_start;
        while (p != rom_image_end) {
            fpga_set_bank(0, addr >> 14);
            uint8_t flash_val = fpga_mem_read(addr & 0x3FFF);

            if (flash_val != *p) {
                char str[50];
                snprintf(str, sizeof(str), "Verify error @ $%05X   (%02X != %02X)", addr, flash_val, *p);
                screen_show_msg(str);

                ESP_LOGE(TAG, "Verify error @ 0x%X   (%02X != %02X)", addr, flash_val, *p);
                ok = false;
            }
            p++;
            addr++;
        }
        fpga_io_write(IO_BANK0, saved_bank);
    }
    ESP_LOGI(TAG, "Done.");

    if (ok) {
        screen_show_msg("Verifying system ROM OK!");
    } else {
        vTaskDelay(pdMS_TO_TICKS(2000));
        screen_show_msg("Verifying system ROM failed!");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    screen_restore();
    fpga_restore_banks();
    fpga_bus_release();

    return ok;
}

void flash_sysrom(void) {
    fpga_bus_acquire();

    screen_save();
    screen_show_msg("Programming system ROM");

    ESP_LOGI(TAG, "* Programming system ROM *");

    bool led = false;

    {
        flash_prepare();

        // char str[40];
        // snprintf(str, sizeof(str), "Erasing system ROM");
        // screen_show_msg(str);
        // flash_erase_chip();

        unsigned       addr = 0;
        const uint8_t *p    = rom_image_start;
        while (p != rom_image_end) {
            if ((addr & 4095) == 0) {
                flash_erase_4kb_sector(addr);
            }

            if ((addr & 1023) == 0) {
                gpio_set_level(IOPIN_LED, led ? 1 : 0);
                led = !led;

                char str[40];
                snprintf(str, sizeof(str), "Programming system ROM @ $%05X", addr);
                screen_show_msg(str);

                ESP_LOGI(TAG, "Programming @ 0x%X", addr);
            }

            flash_program(addr, *p);

            p++;
            addr++;
        }
        flash_finish();
    }
    ESP_LOGI(TAG, "Done.");
    gpio_set_level(IOPIN_LED, 1);

    screen_show_msg("Programming system ROM done.");
    vTaskDelay(pdMS_TO_TICKS(1000));

    screen_restore();
    fpga_bus_release();
}
