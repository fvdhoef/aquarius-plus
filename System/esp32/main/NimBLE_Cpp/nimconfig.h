#pragma once

#define CONFIG_NIMBLE_CPP_IDF 1

#define CONFIG_BTDM_SCAN_DUPL_CACHE_SIZE CONFIG_BT_CTRL_SCAN_DUPL_CACHE_SIZE
#define CONFIG_BTDM_SCAN_DUPL_TYPE CONFIG_BT_CTRL_SCAN_DUPL_TYPE

#define CONFIG_NIMBLE_CPP_LOG_LEVEL ESP_LOG_INFO

#define CONFIG_BT_ENABLE 1
#define CONFIG_BT_NIMBLE_ROLE_OBSERVER 1
#define CONFIG_BT_NIMBLE_ROLE_CENTRAL 1

#define CONFIG_NIMBLE_CPP_ENABLE_RETURN_CODE_TEXT 1
#define CONFIG_NIMBLE_CPP_ENABLE_ADVERTISEMENT_TYPE_TEXT 1
#define CONFIG_NIMBLE_CPP_ENABLE_GAP_EVENT_CODE_TEXT 1

#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
