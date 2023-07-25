#include "ble.h"
#include "NimBLEDevice.h"
#include "HID.h"

static const char *TAG = "ble";

static NimBLEAdvertisedDevice *advDevice;
static bool                    scanning  = false;
static bool                    connected = false;

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient *pClient) {
        ESP_LOGI(TAG, "Connected");
        connected = true;
        // pClient->updateConnParams(120,120,0,60);
    };

    void onDisconnect(NimBLEClient *pClient) {
        ESP_LOGI(TAG, "%s Disconnected", pClient->getPeerAddress().toString().c_str());
        connected = false;
    };

    // Called when the peripheral requests a change to the connection parameters.
    // Return true to accept and apply them or false to reject and keep
    // the currently used parameters. Default will return true.
    bool onConnParamsUpdateRequest(NimBLEClient *pClient, const ble_gap_upd_params *params) {
        ESP_LOGI(TAG, "onConnParamsUpdateRequest");
        if (params->itvl_min < 24) {                    // 1.25ms units
            return false;
        } else if (params->itvl_max > 40) {             // 1.25ms units
            return false;
        } else if (params->latency > 2) {               // Number of intervals allowed to skip
            return false;
        } else if (params->supervision_timeout > 100) { // 10ms units
            return false;
        }
        return true;
    };

    // Pairing process complete, we can check the results in ble_gap_conn_desc
    void onAuthenticationComplete(ble_gap_conn_desc *desc) {
        ESP_LOGI(TAG, "onAuthenticationComplete");
        if (!desc->sec_state.encrypted) {
            ESP_LOGE(TAG, "Encrypt connection failed - disconnecting");

            // Find the client with the connection handle provided in desc
            NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
            return;
        }
    };
};

static ClientCallbacks clientCB;

// Define a class to handle the callbacks when advertisements are received
class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
        if (advertisedDevice->getAppearance() == 0x03C4) {
            ESP_LOGI(TAG, "Found Our Service");

            // Stop scan before connecting
            NimBLEDevice::getScan()->stop();

            // Save the device reference in a global for the client to use
            advDevice = advertisedDevice;
        }
    };
};

// Notification / Indication receiving handler callback
static void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    if (length == 16) {
        handle_xbox_data(pData);
    }
}

static void scanEndedCB(NimBLEScanResults results) {
    ESP_LOGI(TAG, "Scan Ended");
    scanning = false;
}

// Handles the provisioning of clients and connects / interfaces with the server
static bool connectToServer(NimBLEAdvertisedDevice *advDevice) {
    NimBLEClient *pClient = nullptr;

    // Check if we have a client we should reuse first
    if (NimBLEDevice::getClientListSize()) {
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            pClient->connect();
        }
    }

    // No client to reuse? Create a new one.
    if (!pClient) {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
            ESP_LOGE(TAG, "Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        ESP_LOGI(TAG, "New client created");
        pClient->setClientCallbacks(&clientCB, false);
        pClient->setConnectionParams(12, 12, 0, 51);
        pClient->setConnectTimeout(5);
        pClient->connect(advDevice, false);
    }

    int retryCount = 5;
    while (!pClient->isConnected()) {
        if (retryCount <= 0) {
            return false;
        } else {
            ESP_LOGW(TAG, "try connection again");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        NimBLEDevice::getScan()->stop();
        pClient->disconnect();
        vTaskDelay(pdMS_TO_TICKS(500));
        pClient->connect(true);
        --retryCount;
    }

    ESP_LOGI(TAG, "Connected to: %s (RSSI: %d dBm)\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    auto hidSvc = pClient->getService("1812");
    if (!hidSvc) {
        pClient->disconnect();
        return false;
    }
    for (auto chr : *hidSvc->getCharacteristics(true)) {
        if (chr->canRead()) {
            auto str = chr->readValue();
            if (str.size() == 0) {
                str = chr->readValue();
            }
        }
        if (chr->canNotify()) {
            if (chr->subscribe(true, notifyCB, true)) {
                ESP_LOGI(TAG, "Subscribed!");
            } else {
                ESP_LOGE(TAG, "Failed to subscribe!");
                pClient->disconnect();
                return false;
            }
        }
    }

    ESP_LOGI(TAG, "Done with this device!");
    return true;
}

static void bleTask(void *parameter) {
    ESP_LOGI(TAG, "Starting NimBLE Client");
    NimBLEDevice::init("");
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

    while (1) {
        if (!connected) {
            if (!scanning && advDevice == nullptr) {
                scanning   = true;
                auto pScan = NimBLEDevice::getScan();
                pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
                // pScan->setInterval(45);
                // pScan->setWindow(15);
                ESP_LOGI(TAG, "Start scan");
                pScan->start(0, scanEndedCB);
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

void ble_init(void) {
    xTaskCreate(bleTask, "ble", 5000, nullptr, 3, nullptr);
}
