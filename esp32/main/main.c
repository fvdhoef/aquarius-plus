#include "common.h"
#include "usbhost.h"
#include "sdcard.h"
#include "fpga.h"

static const char *TAG = "main";

static void init(void) {
    // Enable power LED
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << IOPIN_LED),
            .mode         = GPIO_MODE_OUTPUT,
        };
        gpio_config(&io_conf);
        gpio_set_level(IOPIN_LED, 1);
    }

    fpga_init();

    // sdcard_init();
    // usbhost_init();
}

void app_main(void) {
    init();
}
