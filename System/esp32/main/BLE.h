#pragma once

#include "Common.h"
#include <host/ble_hs.h>

#include "HIDReportHandler.h"

class BLE {
    BLE();

public:
    static BLE &instance();

    void init();

private:
    static void onReset(int reason);
    static void onSync();
    static void hostTask(void *param);

    void startScan();

    int  onGapEvent(struct ble_gap_event *event);
    void onGapConnect(int status, uint16_t connHandle);
    void onGapDisconnect(int reason, const struct ble_gap_conn_desc &connDesc);
    void onGapEncryptionChange(int status, uint16_t connHandle);
    void onGapNotifyReceived(uint16_t connHandle, uint16_t attrHandle, bool isIndication, struct os_mbuf *om);
    int  onGapRepeatPairing(const struct ble_gap_repeat_pairing &eventInfo);
    void onGapDiscovery(const struct ble_gap_disc_desc &eventInfo);
    void onGapExtDiscovery(const struct ble_gap_ext_disc_desc &eventInfo);

    void onDiscoveryComplete(const struct peer *peer, int status);

    void connectIfInteresting(const ble_addr_t *addr, const struct ble_hs_adv_fields &advFields);

    int onRead(uint16_t connHandle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr);

    static int _onGapEvent(struct ble_gap_event *event, void *arg) {
        return static_cast<BLE *>(arg)->onGapEvent(event);
    }
    static void _onDiscoveryComplete(const struct peer *peer, int status, void *arg) {
        static_cast<BLE *>(arg)->onDiscoveryComplete(peer, status);
    }
    static int _onRead(uint16_t connHandle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
        return static_cast<BLE *>(arg)->onRead(connHandle, error, attr);
    }

    uint16_t          hidDataHandle  = 0;
    HIDReportHandler *reportHandlers = nullptr;

    uint8_t reportDesc[512];
    int     reportDescOffset = 0;
};
