#include "usbhost.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <usb/usb_host.h>
#include <esp_intr_alloc.h>
#include "aq_keyb.h"

static const char *TAG = "usbhost";

#define DAEMON_TASK_PRIORITY 2
#define CLASS_TASK_PRIORITY 3

static void task_class_driver(void *arg);

static void task_host_lib_daemon(void *arg) {
    SemaphoreHandle_t signaling_sem = (SemaphoreHandle_t)arg;

    // Install USB host library
    ESP_LOGI(TAG, "Installing USB Host Library");
    usb_host_config_t host_config = {.skip_phy_setup = false, .intr_flags = ESP_INTR_FLAG_LEVEL1};
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    // Signal to the class driver task that the host library is installed
    xSemaphoreGive(signaling_sem);
    vTaskDelay(10); // Short delay to let client task spin up

    bool has_clients = true;
    bool has_devices = true;
    while (has_clients || has_devices) {
        uint32_t event_flags;
        ESP_ERROR_CHECK(usb_host_lib_handle_events(portMAX_DELAY, &event_flags));
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            has_clients = false;
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            has_devices = false;
        }
    }
    ESP_LOGI(TAG, "No more clients and devices");

    // Uninstall the USB Host Library
    ESP_ERROR_CHECK(usb_host_uninstall());
    // Wait to be deleted
    xSemaphoreGive(signaling_sem);
    vTaskSuspend(NULL);
}

void usbhost_init(void) {
    SemaphoreHandle_t signaling_sem = xSemaphoreCreateBinary();

    // Create daemon task
    xTaskCreatePinnedToCore(task_host_lib_daemon, "daemon", 4096, (void *)signaling_sem, DAEMON_TASK_PRIORITY, NULL, 0);

    // Create the class driver task
    xTaskCreatePinnedToCore(task_class_driver, "class", 4096, (void *)signaling_sem, CLASS_TASK_PRIORITY, NULL, 0);
}

#define CLIENT_NUM_EVENT_MSG 5

enum {
    ACTION_OPEN_DEV        = (1 << 0),
    ACTION_GET_DEV_INFO    = (1 << 1),
    ACTION_GET_DEV_DESC    = (1 << 2),
    ACTION_GET_CONFIG_DESC = (1 << 3),
    ACTION_GET_STR_DESC    = (1 << 4),
    ACTION_CLOSE_DEV       = (1 << 5),
};

typedef struct {
    uint32_t                 actions;
    usb_host_client_handle_t client_hdl;
    uint8_t                  dev_addr;
    usb_device_handle_t      dev_hdl;
    usb_device_info_t        dev_info;
    const usb_config_desc_t *cfg_desc;
} class_driver_t;

static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg) {
    class_driver_t *driver_obj = (class_driver_t *)arg;
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            if (driver_obj->dev_addr == 0) {
                driver_obj->dev_addr = event_msg->new_dev.address;
                // Open the device next
                driver_obj->actions |= ACTION_OPEN_DEV;
            }
            break;
        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            if (driver_obj->dev_hdl != NULL) {
                // Cancel any other actions and close the device next
                driver_obj->actions = ACTION_CLOSE_DEV;
            }
            break;
        default:
            // Should never occur
            abort();
    }
}

static void process_keyboard_data(uint8_t *buf) {
    // Don't process during rollover
    if (buf[2] == 1)
        return;

    static uint8_t prev_status[8];

    // Check for modifier key changes
    uint8_t mod_released = prev_status[0] & ~buf[0];
    if (mod_released & (1 << 0))
        keyboard_scancode(0xE0, false); // Left CTRL
    if (mod_released & (1 << 1))
        keyboard_scancode(0xE1, false); // Left SHIFT
    if (mod_released & (1 << 2))
        keyboard_scancode(0xE2, false); // Left ALT
    if (mod_released & (1 << 3))
        keyboard_scancode(0xE3, false); // Left GUI
    if (mod_released & (1 << 4))
        keyboard_scancode(0xE4, false); // Right CTRL
    if (mod_released & (1 << 5))
        keyboard_scancode(0xE5, false); // Right SHIFT
    if (mod_released & (1 << 6))
        keyboard_scancode(0xE6, false); // Right ALT
    if (mod_released & (1 << 7))
        keyboard_scancode(0xE7, false); // Right GUI

    uint8_t mod_pressed = ~prev_status[0] & buf[0];
    if (mod_pressed & (1 << 0))
        keyboard_scancode(0xE0, true); // Left CTRL
    if (mod_pressed & (1 << 1))
        keyboard_scancode(0xE1, true); // Left SHIFT
    if (mod_pressed & (1 << 2))
        keyboard_scancode(0xE2, true); // Left ALT
    if (mod_pressed & (1 << 3))
        keyboard_scancode(0xE3, true); // Left GUI
    if (mod_pressed & (1 << 4))
        keyboard_scancode(0xE4, true); // Right CTRL
    if (mod_pressed & (1 << 5))
        keyboard_scancode(0xE5, true); // Right SHIFT
    if (mod_pressed & (1 << 6))
        keyboard_scancode(0xE6, true); // Right ALT
    if (mod_pressed & (1 << 7))
        keyboard_scancode(0xE7, true); // Right GUI

    uint8_t prev, cur;

    // Check for key releases
    for (int j = 2; j < 8; j++) {
        if ((prev = prev_status[j]) == 0)
            break;

        // Check if this key is in current report
        bool key_released = true;
        for (int i = 2; i < 8; i++) {
            if ((cur = buf[i]) == 0)
                break;

            if (prev == cur) {
                key_released = false;
                break;
            }
        }

        if (key_released)
            keyboard_scancode(prev, false);
    }

    // Check for key presses
    for (int j = 2; j < 8; j++) {
        if ((cur = buf[j]) == 0)
            break;

        // Check if this key is in previous report
        bool key_pressed = true;
        for (int i = 2; i < 8; i++) {
            if ((prev = prev_status[i]) == 0)
                break;

            if (prev == cur) {
                key_pressed = false;
                break;
            }
        }

        if (key_pressed)
            keyboard_scancode(cur, true);
    }

    for (int i = 0; i < 8; i++) {
        prev_status[i] = buf[i];
    }
    
    keyboard_update_matrix();
}

static void notif_xfer_cb(usb_transfer_t *transfer) {
    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        if (transfer->data_buffer_size == 8) {
            // ESP_LOG_BUFFER_HEX(TAG, transfer->data_buffer, transfer->data_buffer_size);
            process_keyboard_data(transfer->data_buffer);
        }
    }
    if (transfer->status != USB_TRANSFER_STATUS_NO_DEVICE) {
        usb_host_transfer_submit(transfer);
    }
}

static void task_class_driver(void *arg) {
    SemaphoreHandle_t signaling_sem = (SemaphoreHandle_t)arg;
    class_driver_t    driver_obj    = {0};

    // Wait until daemon task has installed USB Host Library
    xSemaphoreTake(signaling_sem, portMAX_DELAY);

    ESP_LOGI(TAG, "Registering Client");
    usb_host_client_config_t client_config = {
        .is_synchronous    = false,
        .max_num_event_msg = CLIENT_NUM_EVENT_MSG,
        .async             = {.client_event_callback = client_event_cb, .callback_arg = (void *)&driver_obj},
    };
    ESP_ERROR_CHECK(usb_host_client_register(&client_config, &driver_obj.client_hdl));

    while (1) {
        if (driver_obj.actions == 0) {
            usb_host_client_handle_events(driver_obj.client_hdl, portMAX_DELAY);
        } else {
            if (driver_obj.actions & ACTION_OPEN_DEV) {
                assert(driver_obj.dev_addr != 0);
                ESP_LOGI(TAG, "Opening device at address %d", driver_obj.dev_addr);
                ESP_ERROR_CHECK(usb_host_device_open(driver_obj.client_hdl, driver_obj.dev_addr, &driver_obj.dev_hdl));

                // Get the device's information next
                driver_obj.actions = (driver_obj.actions & ~ACTION_OPEN_DEV) | ACTION_GET_DEV_INFO;
            }

            if (driver_obj.actions & ACTION_GET_DEV_INFO) {
                assert(driver_obj.dev_hdl != NULL);

                ESP_LOGI(TAG, "Getting device information");
                ESP_ERROR_CHECK(usb_host_device_info(driver_obj.dev_hdl, &driver_obj.dev_info));
                ESP_LOGI(TAG, "\t%s speed", (driver_obj.dev_info.speed == USB_SPEED_LOW) ? "Low" : "Full");
                ESP_LOGI(TAG, "\tbConfigurationValue %d", driver_obj.dev_info.bConfigurationValue);
                // Todo: Print string descriptors

                if (driver_obj.dev_info.str_desc_manufacturer) {
                    ESP_LOGI(TAG, "Getting Manufacturer string descriptor");
                    usb_print_string_descriptor(driver_obj.dev_info.str_desc_manufacturer);
                }
                if (driver_obj.dev_info.str_desc_product) {
                    ESP_LOGI(TAG, "Getting Product string descriptor");
                    usb_print_string_descriptor(driver_obj.dev_info.str_desc_product);
                }
                if (driver_obj.dev_info.str_desc_serial_num) {
                    ESP_LOGI(TAG, "Getting Serial Number string descriptor");
                    usb_print_string_descriptor(driver_obj.dev_info.str_desc_serial_num);
                }

                // Get the device descriptor next
                driver_obj.actions = (driver_obj.actions & ~ACTION_GET_DEV_INFO) | ACTION_GET_DEV_DESC;
            }

            if (driver_obj.actions & ACTION_GET_DEV_DESC) {
                assert(driver_obj.dev_hdl != NULL);

                ESP_LOGI(TAG, "Getting device descriptor");
                const usb_device_desc_t *dev_desc;
                ESP_ERROR_CHECK(usb_host_get_device_descriptor(driver_obj.dev_hdl, &dev_desc));
                usb_print_device_descriptor(dev_desc);

                // Get the device's config descriptor next
                driver_obj.actions = (driver_obj.actions & ~ACTION_GET_DEV_DESC) | ACTION_GET_CONFIG_DESC;
            }

            if (driver_obj.actions & ACTION_GET_CONFIG_DESC) {
                assert(driver_obj.dev_hdl != NULL);

                ESP_LOGI(TAG, "Getting config descriptor");
                ESP_ERROR_CHECK(usb_host_get_active_config_descriptor(driver_obj.dev_hdl, &driver_obj.cfg_desc));
                usb_print_config_descriptor(driver_obj.cfg_desc, NULL);

                // Get the device's string descriptors next
                driver_obj.actions = (driver_obj.actions & ~ACTION_GET_CONFIG_DESC) | ACTION_GET_STR_DESC;
            }

            if (driver_obj.actions & ACTION_GET_STR_DESC) {
                // Nothing to do until the device disconnects
                driver_obj.actions &= ~ACTION_GET_STR_DESC;
                ESP_ERROR_CHECK(usb_host_interface_claim(driver_obj.client_hdl, driver_obj.dev_hdl, 0, 0));
                assert(driver_obj.dev_hdl != NULL);

                ESP_LOGI(TAG, "Starting transfer");

                usb_transfer_t *transfer;
                ESP_ERROR_CHECK(usb_host_transfer_alloc(8, 0, &transfer));
                transfer->device_handle    = driver_obj.dev_hdl;
                transfer->bEndpointAddress = 0x81;
                transfer->num_bytes        = 8;
                transfer->callback         = notif_xfer_cb;
                transfer->context          = NULL;
                transfer->timeout_ms       = 1000;

                esp_err_t err = usb_host_transfer_submit(transfer);
                if (err != ESP_OK) {
                    ESP_LOGI(TAG, "usb_host_transfer_submit: %s", esp_err_to_name(err));
                }
            }

            if (driver_obj.actions & ACTION_CLOSE_DEV) {
                usb_host_device_close(driver_obj.client_hdl, driver_obj.dev_hdl);
                driver_obj.dev_addr = 0;
                driver_obj.dev_hdl  = NULL;
                driver_obj.cfg_desc = NULL;
                driver_obj.actions &= ~ACTION_CLOSE_DEV;
            }
        }
    }

    ESP_LOGI(TAG, "Deregistering Client");
    ESP_ERROR_CHECK(usb_host_client_deregister(driver_obj.client_hdl));

    // Wait to be deleted
    xSemaphoreGive(signaling_sem);
    vTaskSuspend(NULL);
}
