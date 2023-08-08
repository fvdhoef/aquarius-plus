#pragma once

#include "Common.h"
#include "NimBLEDevice.h"
#include "HIDReportHandler.h"

class BLE : public NimBLEClientCallbacks, NimBLEAdvertisedDeviceCallbacks {
    BLE();

public:
    static BLE &instance();

    void init();

private:
    static void _bleTask(void *arg);
    void        bleTask();

    // NimBLEClientCallbacks
    void onConnect(NimBLEClient *client) override;
    void onDisconnect(NimBLEClient *client) override;
    bool onConnParamsUpdateRequest(NimBLEClient *client, const ble_gap_upd_params *params) override;
    void onAuthenticationComplete(ble_gap_conn_desc *desc) override;

    // NimBLEAdvertisedDeviceCallbacks
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) override;

    static void _notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    void        notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    static void _scanEndedCB(NimBLEScanResults results);
    void        scanEndedCB(NimBLEScanResults &results);

    bool connectToServer(NimBLEAdvertisedDevice *advDevice);

    NimBLEAdvertisedDevice *advDevice;
    bool                    scanning;
    bool                    connected;

    HIDReportHandler *reportHandlers = nullptr;
};
