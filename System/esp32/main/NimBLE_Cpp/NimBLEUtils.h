/*
 * NimBLEUtils.h
 *
 *  Created: on Jan 25 2020
 *      Author H2zero
 *
 */

#pragma once

#include "nimconfig.h"
#include "host/ble_gap.h"
#include <string>

typedef struct {
    void        *pATT;
    TaskHandle_t task;
    int          rc;
    void        *buf;
} ble_task_data_t;

/**
 * @brief A BLE Utility class with methods for debugging and general purpose use.
 */
class NimBLEUtils {
public:
    static const char *gapEventToString(uint8_t eventType);
    static char       *buildHexData(uint8_t *target, const uint8_t *source, uint8_t length);
    static const char *returnCodeToString(int rc);
    static int         checkConnParams(ble_gap_conn_params *params);
};
