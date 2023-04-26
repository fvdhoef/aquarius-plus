#pragma once

#include "common.h"

#define WIFI_MIN_RSSI (-100)
#define WIFI_MAX_RSSI (-55)

void        wifi_init(void);
bool        wifi_is_connected(void);
const char *wifi_get_status_str(void);
