#include "ca_store.h"

#include <esp_tls.h>

extern const uint8_t certificate_start[] asm("_binary_letsencrypt_root_certificate_pem_start");
extern const uint8_t certificate_end[] asm("_binary_letsencrypt_root_certificate_pem_end");

void ca_store_init(void) {
    ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store(certificate_start, certificate_end - certificate_start));
}
