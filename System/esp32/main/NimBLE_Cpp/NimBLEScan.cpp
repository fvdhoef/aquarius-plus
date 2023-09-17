/*
 * NimBLEScan.cpp
 *
 *  Created: on Jan 24 2020
 *      Author H2zero
 *
 * Originally:
 *
 * BLEScan.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */

#include "NimBLEScan.h"
#include "NimBLEDevice.h"

#include <string>
#include <climits>

static const char *LOG_TAG = "NimBLEScan";

NimBLEScan &NimBLEScan::instance() {
    static NimBLEScan obj;
    return obj;
}

/**
 * @brief Handle GAP events related to scans.
 * @param [in] event The event type for this event.
 * @param [in] param Parameter data for this event.
 */
/*STATIC*/ int NimBLEScan::handleGapEvent(ble_gap_event *event, void *arg) {
    (void)arg;
    auto scan = NimBLEScan::instance();

    switch (event->type) {

        case BLE_GAP_EVENT_EXT_DISC:
        case BLE_GAP_EVENT_DISC: {
            if (scan.m_ignoreResults) {
                NIMBLE_LOGI(LOG_TAG, "Scan op in progress - ignoring results");
                return 0;
            }
            const auto   &disc       = event->disc;
            const auto    event_type = disc.event_type;
            NimBLEAddress advertisedAddress(disc.addr);

            // Examine our list of ignored addresses and stop processing if we don't want to see it or are already connected
            if (NimBLEDevice::isIgnored(advertisedAddress)) {
                NIMBLE_LOGI(LOG_TAG, "Ignoring device: address: %s", advertisedAddress.toString().c_str());
                return 0;
            }

            NimBLEAdvertisedDevice *advertisedDevice = nullptr;

            // If we've seen this device before get a pointer to it from the vector
            for (auto &it : scan.m_scanResults.m_advertisedDevicesVector) {
                if (it->getAddress() == advertisedAddress) {
                    advertisedDevice = it;
                    break;
                }
            }

            // If we haven't seen this device before; create a new instance and insert it in the vector.
            // Otherwise just update the relevant parameters of the already known device.
            if (advertisedDevice == nullptr && event_type != BLE_HCI_ADV_RPT_EVTYPE_SCAN_RSP) {
                // Check if we have reach the scan results limit, ignore this one if so.
                // We still need to store each device when maxResults is 0 to be able to append the scan results
                if (scan.m_maxResults > 0 && scan.m_maxResults < 0xFF &&
                    (scan.m_scanResults.m_advertisedDevicesVector.size() >= scan.m_maxResults)) {
                    return 0;
                }

                advertisedDevice = new NimBLEAdvertisedDevice();
                advertisedDevice->setAddress(advertisedAddress);
                advertisedDevice->setAdvType(event_type);
                scan.m_scanResults.m_advertisedDevicesVector.push_back(advertisedDevice);
                NIMBLE_LOGI(LOG_TAG, "New advertiser: %s", advertisedAddress.toString().c_str());
            } else if (advertisedDevice != nullptr) {
                NIMBLE_LOGI(LOG_TAG, "Updated advertiser: %s", advertisedAddress.toString().c_str());
            } else {
                // Scan response from unknown device
                return 0;
            }

            advertisedDevice->setRSSI(disc.rssi);
            advertisedDevice->setPayload(disc.data, disc.length_data, event_type == BLE_HCI_ADV_RPT_EVTYPE_SCAN_RSP);

            if (scan.m_pAdvertisedDeviceCallbacks) {
                if (scan.m_scan_params.filter_duplicates && advertisedDevice->m_callbackSent) {
                    return 0;
                }

                // If not active scanning or scan response is not available
                // or extended advertisement scanning, report the result to the callback now.
                if (scan.m_scan_params.passive || (advertisedDevice->getAdvType() != BLE_HCI_ADV_TYPE_ADV_IND && advertisedDevice->getAdvType() != BLE_HCI_ADV_TYPE_ADV_SCAN_IND)) {
                    advertisedDevice->m_callbackSent = true;
                    scan.m_pAdvertisedDeviceCallbacks->onResult(advertisedDevice);

                    // Otherwise, wait for the scan response so we can report the complete data.
                } else if (event_type == BLE_HCI_ADV_RPT_EVTYPE_SCAN_RSP) {
                    advertisedDevice->m_callbackSent = true;
                    scan.m_pAdvertisedDeviceCallbacks->onResult(advertisedDevice);
                }
                // If not storing results and we have invoked the callback, delete the device.
                if (scan.m_maxResults == 0 && advertisedDevice->m_callbackSent) {
                    scan.erase(advertisedAddress);
                }
            }

            return 0;
        }
        case BLE_GAP_EVENT_DISC_COMPLETE: {
            NIMBLE_LOGD(LOG_TAG, "discovery complete; reason=%d", event->disc_complete.reason);

            // If a device advertised with scan response available and it was not received
            // the callback would not have been invoked, so do it here.
            if (scan.m_pAdvertisedDeviceCallbacks) {
                for (auto &it : scan.m_scanResults.m_advertisedDevicesVector) {
                    if (!it->m_callbackSent) {
                        scan.m_pAdvertisedDeviceCallbacks->onResult(it);
                    }
                }
            }

            if (scan.m_maxResults == 0) {
                scan.clearResults();
            }

            if (scan.m_scanCompleteCB != nullptr) {
                scan.m_scanCompleteCB(scan.m_scanResults);
            }

            if (scan.m_pTaskData != nullptr) {
                scan.m_pTaskData->rc = event->disc_complete.reason;
                xTaskNotifyGive(scan.m_pTaskData->task);
            }

            return 0;
        }

        default:
            return 0;
    }
}

/**
 * @brief Start scanning.
 * @param [in] duration The duration in seconds for which to scan.
 * @param [in] scanCompleteCB A function to be called when scanning has completed.
 * @param [in] is_continue Set to true to save previous scan results, false to clear them.
 * @return True if scan started or false if there was an error.
 */
bool NimBLEScan::start(uint32_t duration, void (*scanCompleteCB)(NimBLEScanResults), bool is_continue) {
    NIMBLE_LOGD(LOG_TAG, ">> start: duration=%" PRIu32, duration);

    // Save the callback to be invoked when the scan completes.
    m_scanCompleteCB = scanCompleteCB;
    // Save the duration in the case that the host is reset so we can reuse it.
    m_duration = duration;

    // If 0 duration specified then we assume a continuous scan is desired.
    if (duration == 0) {
        duration = BLE_HS_FOREVER;
    } else {
        // convert duration to milliseconds
        duration = duration * 1000;
    }

    // Set the flag to ignore the results while we are deleting the vector
    if (!is_continue) {
        m_ignoreResults = true;
    }

    int rc = ble_gap_disc(NimBLEDevice::m_own_addr_type, duration, &m_scan_params, NimBLEScan::handleGapEvent, NULL);
    switch (rc) {
        case 0:
            if (!is_continue) {
                clearResults();
            }
            break;

        case BLE_HS_EALREADY:
            break;

        case BLE_HS_EBUSY:
            NIMBLE_LOGE(LOG_TAG, "Unable to scan - connection in progress.");
            break;

        case BLE_HS_ETIMEOUT_HCI:
        case BLE_HS_EOS:
        case BLE_HS_ECONTROLLER:
        case BLE_HS_ENOTSYNCED:
            NIMBLE_LOGC(LOG_TAG, "Unable to scan - Host Reset");
            break;

        default:
            NIMBLE_LOGE(LOG_TAG, "Error initiating GAP discovery procedure; rc=%d, %s", rc, NimBLEUtils::returnCodeToString(rc));
            break;
    }

    m_ignoreResults = false;
    NIMBLE_LOGD(LOG_TAG, "<< start()");

    if (rc != 0 && rc != BLE_HS_EALREADY) {
        return false;
    }
    return true;
}

/**
 * @brief Start scanning and block until scanning has been completed.
 * @param [in] duration The duration in seconds for which to scan.
 * @param [in] is_continue Set to true to save previous scan results, false to clear them.
 * @return The NimBLEScanResults.
 */
NimBLEScanResults NimBLEScan::start(uint32_t duration, bool is_continue) {
    if (duration == 0) {
        NIMBLE_LOGW(LOG_TAG, "Blocking scan called with duration = forever");
    }

    TaskHandle_t    cur_task = xTaskGetCurrentTaskHandle();
    ble_task_data_t taskData = {nullptr, cur_task, 0, nullptr};
    m_pTaskData              = &taskData;

    if (start(duration, nullptr, is_continue)) {
        // Clear the task notification value to ensure we block
        ulTaskNotifyValueClear(cur_task, ULONG_MAX);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    m_pTaskData = nullptr;
    return m_scanResults;
}

/**
 * @brief Stop an in progress scan.
 * @return True if successful.
 */
bool NimBLEScan::stop() {
    NIMBLE_LOGD(LOG_TAG, ">> stop()");

    int rc = ble_gap_disc_cancel();
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        NIMBLE_LOGE(LOG_TAG, "Failed to cancel scan; rc=%d", rc);
        return false;
    }

    if (m_maxResults == 0) {
        clearResults();
    }

    if (rc != BLE_HS_EALREADY && m_scanCompleteCB != nullptr) {
        m_scanCompleteCB(m_scanResults);
    }

    if (m_pTaskData != nullptr) {
        xTaskNotifyGive(m_pTaskData->task);
    }

    NIMBLE_LOGD(LOG_TAG, "<< stop()");
    return true;
}

/**
 * @brief Delete peer device from the scan results vector.
 * @param [in] address The address of the device to delete from the results.
 * @details After disconnecting, it may be required in the case we were connected to a device without a public address.
 */
void NimBLEScan::erase(const NimBLEAddress &address) {
    NIMBLE_LOGD(LOG_TAG, "erase device: %s", address.toString().c_str());

    for (auto it = m_scanResults.m_advertisedDevicesVector.begin(); it != m_scanResults.m_advertisedDevicesVector.end(); ++it) {
        if ((*it)->getAddress() == address) {
            delete *it;
            m_scanResults.m_advertisedDevicesVector.erase(it);
            break;
        }
    }
}

/**
 * @brief If the host reset and re-synced this is called.
 * If the application was scanning indefinitely with a callback, restart it.
 */
void NimBLEScan::onHostSync() {
    m_ignoreResults = false;

    if (m_duration == 0 && m_pAdvertisedDeviceCallbacks != nullptr) {
        start(m_duration, m_scanCompleteCB);
    }
}

/**
 * @brief Clear the results of the scan.
 */
void NimBLEScan::clearResults() {
    for (auto &it : m_scanResults.m_advertisedDevicesVector) {
        delete it;
    }
    m_scanResults.m_advertisedDevicesVector.clear();
}

/**
 * @brief Dump the scan results to the log.
 */
void NimBLEScanResults::dump() {
    NIMBLE_LOGD(LOG_TAG, ">> Dump scan results:");
    for (int i = 0; i < getCount(); i++) {
        NIMBLE_LOGI(LOG_TAG, "- %s", getDevice(i)->toString().c_str());
    }
}

/**
 * @brief Get a pointer to the specified device at the given address.
 * If the address is not found a nullptr is returned.
 * @param [in] address The address of the device.
 * @return A pointer to the device at the specified address.
 */
NimBLEAdvertisedDevice *NimBLEScanResults::getDevice(const NimBLEAddress &address) {
    for (size_t index = 0; index < m_advertisedDevicesVector.size(); index++) {
        if (m_advertisedDevicesVector[index]->getAddress() == address) {
            return m_advertisedDevicesVector[index];
        }
    }

    return nullptr;
}
