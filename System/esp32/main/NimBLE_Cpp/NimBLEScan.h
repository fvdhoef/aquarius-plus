/*
 * NimBLEScan.h
 *
 *  Created: on Jan 24 2020
 *      Author H2zero
 *
 * Originally:
 *
 * BLEScan.h
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */
#pragma once

#include "nimconfig.h"

#include "NimBLEAdvertisedDevice.h"
#include "NimBLEUtils.h"

#include "host/ble_gap.h"

#include <vector>

class NimBLEDevice;
class NimBLEScan;
class NimBLEAdvertisedDevice;
class NimBLEAdvertisedDeviceCallbacks;
class NimBLEAddress;

/**
 * @brief A class that contains and operates on the results of a BLE scan.
 * @details When a scan completes, we have a set of found devices.  Each device is described
 * by a NimBLEAdvertisedDevice object.  The number of items in the set is given by
 * getCount().  We can retrieve a device by calling getDevice() passing in the
 * index (starting at 0) of the desired device.
 */
class NimBLEScanResults {
public:
    void dump();
    int  getCount() {
        return m_advertisedDevicesVector.size();
    }
    NimBLEAdvertisedDevice *getDevice(uint32_t i) {
        return m_advertisedDevicesVector[i];
    }
    std::vector<NimBLEAdvertisedDevice *>::iterator begin() {
        return m_advertisedDevicesVector.begin();
    }
    std::vector<NimBLEAdvertisedDevice *>::iterator end() {
        return m_advertisedDevicesVector.end();
    }
    NimBLEAdvertisedDevice *getDevice(const NimBLEAddress &address);

private:
    friend NimBLEScan;
    std::vector<NimBLEAdvertisedDevice *> m_advertisedDevicesVector;
};

/**
 * @brief Perform and manage %BLE scans.
 *
 * Scanning is associated with a %BLE client that is attempting to locate BLE servers.
 */
class NimBLEScan {
public:
    bool              start(uint32_t duration, void (*scanCompleteCB)(NimBLEScanResults), bool is_continue = false);
    NimBLEScanResults start(uint32_t duration, bool is_continue = false);
    bool              isScanning() {
        return ble_gap_disc_active();
    }
    void setAdvertisedDeviceCallbacks(NimBLEAdvertisedDeviceCallbacks *pAdvertisedDeviceCallbacks, bool wantDuplicates = false) {
        setDuplicateFilter(!wantDuplicates);
        m_pAdvertisedDeviceCallbacks = pAdvertisedDeviceCallbacks;
    }

    // clang-format off
    void setActiveScan(bool active)          { m_scan_params.passive = !active; }
    void setInterval(uint16_t intervalMSecs) { m_scan_params.itvl = intervalMSecs / 0.625; }
    void setWindow(uint16_t windowMSecs)     { m_scan_params.window = windowMSecs / 0.625; }
    void setDuplicateFilter(bool enabled)    { m_scan_params.filter_duplicates = enabled; }
    void setLimitedOnly(bool enabled)        { m_scan_params.limited = enabled; }
    void setFilterPolicy(uint8_t filter)     { m_scan_params.filter_policy = filter; }
    // clang-format on

    bool stop();
    void clearResults();

    NimBLEScanResults getResults() {
        return m_scanResults;
    }

    void setMaxResults(uint8_t maxResults) {
        m_maxResults = maxResults;
    }
    void erase(const NimBLEAddress &address);

private:
    friend class NimBLEDevice;

    NimBLEScan() {
    }
    ~NimBLEScan() {
        clearResults();
    }
    static int handleGapEvent(ble_gap_event *event, void *arg);

    void onHostReset() {
        m_ignoreResults = true;
    }

    void onHostSync();

    NimBLEAdvertisedDeviceCallbacks *m_pAdvertisedDeviceCallbacks = nullptr;
    void (*m_scanCompleteCB)(NimBLEScanResults scanResults);
    ble_gap_disc_params m_scan_params = {
        .itvl              = 0,
        .window            = 0,
        .filter_policy     = BLE_HCI_SCAN_FILT_NO_WL,
        .limited           = 0,
        .passive           = 1,
        .filter_duplicates = 1,
    };
    bool              m_ignoreResults = false;
    NimBLEScanResults m_scanResults;
    uint32_t          m_duration   = BLE_HS_FOREVER;
    ble_task_data_t  *m_pTaskData  = nullptr;
    uint8_t           m_maxResults = 0xFF;
};
