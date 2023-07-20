#pragma once

#include "common.h"
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
#include <esp_central.h>

void print_uuid2(const ble_uuid_t *uuid);
void print_bytes2(const uint8_t *bytes, int len);
void print_adv_fields2(const struct ble_hs_adv_fields *fields);
void print_mbuf2(const struct os_mbuf *om);
void print_peer_svcs(const struct peer *peer);

void ble_store_config_init(void);
