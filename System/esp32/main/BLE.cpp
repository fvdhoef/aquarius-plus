#include "BLE.h"
#include "NimBLEDevice.h"
#include "HIDReportDescriptor.h"

static const char *TAG = "BLE";

BLE::BLE() {
    advDevice = nullptr;
    scanning  = false;
    connected = false;
}

BLE &BLE::instance() {
    static BLE obj;
    return obj;
}

void BLE::init() {
    xTaskCreate(_bleTask, "BLE", 5000, this, 3, nullptr);
}

void BLE::_bleTask(void *arg) {
    static_cast<BLE *>(arg)->bleTask();
}

void BLE::bleTask() {
    ESP_LOGI(TAG, "Starting NimBLE Client");
    NimBLEDevice::init("");
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

    while (1) {
        if (!connected) {
            if (!scanning && advDevice == nullptr) {
                scanning   = true;
                auto pScan = NimBLEDevice::getScan();
                pScan->setAdvertisedDeviceCallbacks(this);
                // pScan->setInterval(45);
                // pScan->setWindow(15);
                ESP_LOGI(TAG, "Start scan");
                pScan->start(0, _scanEndedCB);
            }
            if (advDevice != nullptr) {
                if (connectToServer(advDevice)) {
                    ESP_LOGI(TAG, "Success! we should now be getting notifications");
                } else {
                    ESP_LOGE(TAG, "Failed to connect");
                }
                advDevice = nullptr;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void BLE::onConnect(NimBLEClient *client) {
    ESP_LOGI(TAG, "Connected");
    connected = true;
    // client->updateConnParams(120,120,0,60);
};

void BLE::onDisconnect(NimBLEClient *client) {
    ESP_LOGI(TAG, "%s Disconnected", client->getPeerAddress().toString().c_str());
    connected = false;

    if (reportHandlers) {
        delete reportHandlers;
        reportHandlers = nullptr;
    }
};

// Called when the peripheral requests a change to the connection parameters.
// Return true to accept and apply them or false to reject and keep
// the currently used parameters. Default will return true.
bool BLE::onConnParamsUpdateRequest(NimBLEClient *client, const ble_gap_upd_params *params) {
    ESP_LOGI(TAG, "onConnParamsUpdateRequest");
    if (params->itvl_min < 24) { // 1.25ms units
        return false;
    } else if (params->itvl_max > 40) { // 1.25ms units
        return false;
    } else if (params->latency > 2) { // Number of intervals allowed to skip
        return false;
    } else if (params->supervision_timeout > 100) { // 10ms units
        return false;
    }
    return true;
};

// Pairing process complete, we can check the results in ble_gap_conn_desc
void BLE::onAuthenticationComplete(ble_gap_conn_desc *desc) {
    ESP_LOGI(TAG, "onAuthenticationComplete");
    if (!desc->sec_state.encrypted) {
        ESP_LOGE(TAG, "Encrypt connection failed - disconnecting");

        // Find the client with the connection handle provided in desc
        NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
        return;
    }
};

void BLE::onResult(NimBLEAdvertisedDevice *advertisedDevice) {
    if (advertisedDevice->getAppearance() == 0x03C4) {
        ESP_LOGI(TAG, "Found Our Service");

        // Stop scan before connecting
        NimBLEDevice::getScan()->stop();

        // Save the device reference in a global for the client to use
        advDevice = advertisedDevice;
    }
}

void BLE::_notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    instance().notifyCB(pRemoteCharacteristic, pData, length, isNotify);
}

// Notification / Indication receiving handler callback
void BLE::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    HIDReportHandler *reportHandler = reportHandlers;
    while (reportHandler) {
        // FIXME: How do report Ids work in Bluetooth?
        if (reportHandler->hasReportId)
            reportHandler->hasReportId = false;

        reportHandler->inputReport(pData, length);
        reportHandler = reportHandler->next;
    }
}

void BLE::_scanEndedCB(NimBLEScanResults results) {
    instance().scanEndedCB(results);
}

void BLE::scanEndedCB(NimBLEScanResults &results) {
    ESP_LOGI(TAG, "Scan Ended");
    scanning = false;
}

// Handles the provisioning of clients and connects / interfaces with the server
bool BLE::connectToServer(NimBLEAdvertisedDevice *advDevice) {
    NimBLEClient *client = nullptr;

    // Check if we have a client we should reuse first
    if (NimBLEDevice::getClientListSize()) {
        client = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (client) {
            client->connect();
        }
    }

    // No client to reuse? Create a new one.
    if (!client) {
        if (NimBLEDevice::getClientListSize() >= CONFIG_BT_NIMBLE_MAX_CONNECTIONS) {
            ESP_LOGE(TAG, "Max clients reached - no more connections available");
            return false;
        }

        client = NimBLEDevice::createClient();

        ESP_LOGI(TAG, "New client created");
        client->setClientCallbacks(this, false);
        client->setConnectionParams(12, 12, 0, 51);
        client->setConnectTimeout(5);
        client->connect(advDevice, false);
    }

    int retryCount = 5;
    while (!client->isConnected()) {
        if (retryCount <= 0) {
            return false;
        } else {
            ESP_LOGW(TAG, "try connection again");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        NimBLEDevice::getScan()->stop();
        client->disconnect();
        vTaskDelay(pdMS_TO_TICKS(500));
        client->connect(true);
        --retryCount;
    }

    ESP_LOGI(TAG, "Connected to: %s (RSSI: %d dBm)\n", client->getPeerAddress().toString().c_str(), client->getRssi());

    auto hidSvc = client->getService("1812");
    if (!hidSvc) {
        client->disconnect();
        return false;
    }
    auto reportMapChr = hidSvc->getCharacteristic("2A4B");
    if (reportMapChr) {
        auto str       = reportMapChr->readValue();
        reportHandlers = HIDReportHandler::getReportHandlersForDescriptor(str.data(), str.size());
    }

    for (auto chr : *hidSvc->getCharacteristics(true)) {
        if (chr->canRead()) {
            auto str = chr->readValue();
            if (str.size() == 0) {
                str = chr->readValue();
            }
        }
        if (chr->canNotify()) {
            if (chr->subscribe(true, _notifyCB, true)) {
                ESP_LOGI(TAG, "Subscribed!");
            } else {
                ESP_LOGE(TAG, "Failed to subscribe!");
                client->disconnect();
                return false;
            }
        }
    }

    ESP_LOGI(TAG, "Done with this device!");
    return true;
}
