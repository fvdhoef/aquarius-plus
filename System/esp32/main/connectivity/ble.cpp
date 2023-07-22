#include "ble.h"
#include "NimBLEDevice.h"
#include "hid.h"

static const char *TAG = "ble";

static NimBLEAdvertisedDevice *advDevice;
static volatile bool           doConnect = false;

static NimBLEUUID uuidServiceHid((uint16_t)0x1812);

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient *pClient) {
        ESP_LOGI(TAG, "Connected\n");
        // pClient->updateConnParams(120,120,0,60);
    };

    void onDisconnect(NimBLEClient *pClient, int reason) {
        ESP_LOGI(TAG, "%s Disconnected, reason = %d - Starting scan", pClient->getPeerAddress().toString().c_str(), reason);
        NimBLEDevice::getScan()->start(0);
    };

    bool onConnParamsUpdateRequest(NimBLEClient *pClient, const ble_gap_upd_params *params) {
        ESP_LOGI(TAG, "onConnParamsUpdateRequest");
        if (params->itvl_min < 24) {                    /** 1.25ms units */
            return false;
        } else if (params->itvl_max > 40) {             /** 1.25ms units */
            return false;
        } else if (params->latency > 2) {               /** Number of intervals allowed to skip */
            return false;
        } else if (params->supervision_timeout > 100) { /** 10ms units */
            return false;
        }
        return true;
    };

    // Pairing process complete, we can check the results in connInfo
    void onAuthenticationComplete(NimBLEConnInfo &connInfo) {
        if (!connInfo.isEncrypted()) {
            ESP_LOGE(TAG, "Encrypt connection failed - disconnecting\n");
            // Find the client with the connection handle provided in desc
            NimBLEDevice::getClientByID(connInfo.getConnHandle())->disconnect();
            return;
        }
    };
};

/** Define a class to handle the callbacks when advertisements are received */
class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
        if (advertisedDevice->getAppearance() == 0x03C4) {
            ESP_LOGI(TAG, "Found Our Service");
            NimBLEDevice::getScan()->stop();
            advDevice = advertisedDevice;
            doConnect = true;
        }
    };
};

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    if (length == 16 && isNotify) {
        handle_xbox_data(pData);
    }

    // std::string str = (isNotify == true) ? "Notification" : "Indication";
    // str += " from ";
    // str += pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress().toString();
    // str += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    // str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
    // str += ", Value = " + std::string((char *)pData, length);
    // ESP_LOGI(TAG, "%s", str.c_str());
}

static ClientCallbacks clientCB;

bool afterConnect(NimBLEClient *pClient) {
    for (auto pService : *pClient->getServices(true)) {
        auto sUuid = pService->getUUID();
        if (!sUuid.equals(uuidServiceHid)) {
            continue; // skip
        }
        for (auto chr : *pService->getCharacteristics(true)) {
            if (chr->canRead()) {
                puts(" canRead");
                auto str = chr->readValue();
                if (str.size() == 0) {
                    str = chr->readValue();
                }

                printf("str: %s\n", str.c_str());
                printf("hex:");
                for (auto v : str) {
                    printf(" %02x", v);
                }
                puts("");
            }

            if (chr->canNotify()) {
                puts(" canNotify ");
                if (chr->subscribe(true, notifyCB, true)) {
                    puts("set notifyCb");
                    // return true;
                } else {
                    puts("failed to subscribe");
                }
            }
        }
    }

    return true;
}

// Handles the provisioning of clients and connects / interfaces with the server
bool connectToServer() {
    if (!advDevice)
        return false;

    NimBLEClient *pClient = nullptr;

    // Check if we have a client we should reuse first
    if (NimBLEDevice::getClientListSize()) {
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(advDevice, false)) {
                ESP_LOGE(TAG, "Reconnect failed");
                return false;
            }
            ESP_LOGI(TAG, "Reconnected client");
        }
    }
    // We don't already have a client that knows this device, we will check for a client that is disconnected that we can use.
    else {
        pClient = NimBLEDevice::getDisconnectedClient();
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
        pClient->setConnectionParams(6, 6, 0, 15);
        // pClient->setConnectionParams(12, 12, 0, 51);
        pClient->setConnectTimeout(30);

        if (!pClient->connect(advDevice)) {
            // Created a client but failed to connect, don't need to keep it as it has no data
            NimBLEDevice::deleteClient(pClient);
            ESP_LOGE(TAG, "Failed to connect, deleted client");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            ESP_LOGE(TAG, "Failed to connect");
            return false;
        }
    }

    ESP_LOGI(TAG, "Connected to: %s RSSI: %d", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    pClient->discoverAttributes();

    auto svc = pClient->getService("1812");
    if (svc) {
        for (auto chr : *svc->getCharacteristics(true)) {
            if (chr->canRead()) {
                chr->readValue();
            }
            if (chr->canNotify()) {
                chr->subscribe(true, notifyCB, true);
            }
        }
    }

    puts("Done with this device!");
    return true;
}

static void connectTask(void *parameter) {
    NimBLEDevice::init("");
    // NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    auto pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new ScanCallbacks());
    pScan->setInterval(45);
    pScan->setWindow(15);
    pScan->setActiveScan(true);
    pScan->start(0);

    // Loop here until we find a device we want to connect to
    for (;;) {
        if (doConnect) {
            doConnect = false;

            ESP_LOGI(TAG, "Connecting to found device!");

            // Found a device we want to connect to, do it now
            if (connectToServer()) {
                ESP_LOGI(TAG, "Success! we should now be getting notifications!"); //, scanning for more!");
            } else {
                ESP_LOGE(TAG, "Failed to connect, starting scan");
                NimBLEDevice::getScan()->start(0);
            }
        }

        // ESP_LOGI(TAG, "Waiting");
        vTaskDelay(pdTICKS_TO_MS(500));
    }
    vTaskDelete(NULL);
}

void ble_init(void) {
    xTaskCreate(connectTask, "connectTask", 5000, NULL, 3, NULL);
}
