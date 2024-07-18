#include "BLE.h"
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
#include <esp_central.h>

static const char *TAG = "BLE";

// Xbox One Series S/X controller services/characteristics:
// - 0x1800             Generic Access service
//   - 0x2a00               Device Name
//   - 0x2a01               Appearance
//   - 0x2a04               Peripheral Preferred Connection Parameters
// - 0x1801             Generic Attribute service
// - 0x180a             Device Information service
//   - 0x2a29               Manufacturer Name String
//   - 0x2a50               PnP ID
//   - 0x2a26               Firmware Revision String
//   - 0x2a25               Serial Number String
// - 0x180f             Battery service
//   - 0x2a19               Battery Level
// - 0x1812             Human Interface Device service
//   - 0x2a4a               HID Information
//   - 0x2a4c               HID Control Point
//   - 0x2a4b               Report Map
//   - 0x2a4d               Report
//   - 0x2a4d               Report
// - 00000001-5f60-4c4f-9c83-a7953298d40d
//   - 00000002-5f60-4c4f-9c83-a7953298d40d
//   - 00000003-5f60-4c4f-9c83-a7953298d40d
//   - 00000004-5f60-4c4f-9c83-a7953298d40d

// #define PEER_ADDR "a8:8c:3e:34:03:39"

extern "C" void ble_store_config_init(void);

BLE::BLE() {
}

BLE &BLE::instance() {
    static BLE obj;
    return obj;
}

void BLE::init() {
    ESP_ERROR_CHECK(nimble_port_init());

    ble_hs_cfg.reset_cb        = onReset;
    ble_hs_cfg.sync_cb         = onSync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Set initial security capabilities
    ble_hs_cfg.sm_io_cap         = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding        = 1;
    ble_hs_cfg.sm_mitm           = 1;
    ble_hs_cfg.sm_sc             = 1;
    ble_hs_cfg.sm_our_key_dist   = 1;
    ble_hs_cfg.sm_their_key_dist = 3;

    int rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
    assert(rc == 0);

    ble_store_config_init();
    ble_store_clear();

    nimble_port_freertos_init(hostTask);
}

void BLE::onReset(int reason) {
    ESP_LOGE(TAG, "Resetting state; reason=%d", reason);
}

void BLE::onSync() {
    // Make sure we have proper identity address set (public preferred)
    int rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    // Begin scanning for a peripheral to connect to
    instance().startScan();
}

void BLE::hostTask(void *param) {
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void BLE::startScan() {
    uint8_t ownAddrType;
    int     rc;

    // Figure out address to use while advertising (no privacy for now)
    if ((rc = ble_hs_id_infer_auto(0, &ownAddrType)) != 0) {
        ESP_LOGE(TAG, "error determining address type; rc=%d", rc);
        return;
    }

    struct ble_gap_disc_params disc_params = {
        .itvl              = 0,
        .window            = 0,
        .filter_policy     = 0,
        .limited           = 0,
        .passive           = 1, // Perform a passive scan.  I.e., don't send follow-up scan requests to each advertiser.
        .filter_duplicates = 1, // Tell the controller to filter duplicates, we don't want to process repeated advertisements from the same device.
    };

    if ((rc = ble_gap_disc(ownAddrType, BLE_HS_FOREVER, &disc_params, _onGapEvent, this)) != 0) {
        ESP_LOGE(TAG, "Error initiating GAP discovery procedure; rc=%d", rc);
    }
}

void BLE::onGapConnect(int status, uint16_t connHandle) {
    if (status != 0) {
        // Connection attempt failed; resume scanning.
        ESP_LOGE(TAG, "Error: Connection failed, status=%d", status);
        startScan();
        return;
    }

    ESP_LOGI(TAG, "Connection established");

    struct ble_gap_conn_desc desc;
    int                      rc = ble_gap_conn_find(connHandle, &desc);
    assert(rc == 0);

    // Remember peer.
    rc = peer_add(connHandle);
    if (rc == BLE_HS_EALREADY) {
        peer_delete(connHandle);

        if ((rc = peer_add(connHandle)) != 0) {
            ESP_LOGE(TAG, "Failed to add peer; rc=%d", rc);
            return;
        }
    }

    // Secure connection
    if ((rc = ble_gap_security_initiate(connHandle)) != 0) {
        ESP_LOGI(TAG, "Security could not be initiated, rc = %d", rc);
        ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);

    } else {
        ESP_LOGI(TAG, "Connection security initiated");
    }
}

void BLE::onGapDisconnect(int reason, const struct ble_gap_conn_desc &connDesc) {
    // Connection terminated
    ESP_LOGI(TAG, "Disconnected; reason=%d", reason);

    // Forget about peer
    peer_delete(connDesc.conn_handle);
    if (reportHandlers) {
        delete reportHandlers;
        reportHandlers = nullptr;
    }

    // Resume scanning
    startScan();
}

void BLE::onGapEncryptionChange(int status, uint16_t connHandle) {
    ESP_LOGI(TAG, "encryption change event; status=%d ", status);

    struct ble_gap_conn_desc desc;
    int                      rc = ble_gap_conn_find(connHandle, &desc);
    assert(rc == 0);

    // Perform service discovery now encryption has been successfully enabled
    if ((rc = peer_disc_all(connHandle, _onDiscoveryComplete, this)) != 0) {
        ESP_LOGE(TAG, "Failed to discover services; rc=%d", rc);
        return;
    }
}

void BLE::onGapNotifyReceived(uint16_t connHandle, uint16_t attrHandle, bool isIndication, struct os_mbuf *om) {
    // ESP_LOGI(TAG, "received %s; conn_handle=%d attr_handle=%d attr_len=%d", isIndication ? "indication" : "notification", connHandle, attrHandle, OS_MBUF_PKTLEN(om));
    // Attribute data is contained in event->notify_rx.om. Use `os_mbuf_copydata` to copy the data received in notification mbuf

    uint8_t tmpbuf[64];
    auto    len = OS_MBUF_PKTLEN(om);

    if (attrHandle != hidDataHandle || len > sizeof(tmpbuf) || os_mbuf_copydata(om, 0, len, tmpbuf) != 0)
        return;

    HIDReportHandler *reportHandler = reportHandlers;
    while (reportHandler) {
        // FIXME: How do report Ids work in Bluetooth?
        if (reportHandler->hasReportId)
            reportHandler->hasReportId = false;

        reportHandler->inputReport(tmpbuf, len);
        reportHandler = reportHandler->next;
    }

    // printf("- HID data:");
    // for (int i = 0; i < len; i++) {
    //     printf(" %02X", tmpbuf[i]);
    // }
    // printf("\n");
}

int BLE::onGapRepeatPairing(const struct ble_gap_repeat_pairing &eventInfo) {
    // We already have a bond with the peer, but it is attempting to establish a new secure link.
    // This app sacrifices security for convenience: just throw away the old bond and accept the new link.

    // Delete the old bond.
    struct ble_gap_conn_desc desc;
    int                      rc = ble_gap_conn_find(eventInfo.conn_handle, &desc);
    assert(rc == 0);
    ble_store_util_delete_peer(&desc.peer_id_addr);

    // Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should continue with the pairing operation.
    return BLE_GAP_REPEAT_PAIRING_RETRY;
}

void BLE::onGapDiscovery(const struct ble_gap_disc_desc &eventInfo) {
    if (eventInfo.event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
        eventInfo.event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND) {
        // Not connectable
        return;
    }

    struct ble_hs_adv_fields advFields;
    int                      rc;
    if ((rc = ble_hs_adv_parse_fields(&advFields, eventInfo.data, eventInfo.length_data)) != 0)
        return;

    connectIfInteresting(&eventInfo.addr, advFields);
}

void BLE::onGapExtDiscovery(const struct ble_gap_ext_disc_desc &eventInfo) {
    if ((eventInfo.props & BLE_HCI_ADV_LEGACY_MASK) == 0 ||
        (eventInfo.legacy_event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
         eventInfo.legacy_event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND)) {
        // Not connectable
        return;
    }

    struct ble_hs_adv_fields advFields;
    int                      rc;
    if ((rc = ble_hs_adv_parse_fields(&advFields, eventInfo.data, eventInfo.length_data)) != 0)
        return;

    connectIfInteresting(&eventInfo.addr, advFields);
}

int BLE::onGapEvent(struct ble_gap_event *event) {
    switch (event->type) {
        case BLE_GAP_EVENT_DISC: onGapDiscovery(event->disc); return 0;
        case BLE_GAP_EVENT_EXT_DISC: onGapExtDiscovery(event->ext_disc); return 0;
        case BLE_GAP_EVENT_CONNECT: onGapConnect(event->connect.status, event->connect.conn_handle); return 0;
        case BLE_GAP_EVENT_DISCONNECT: onGapDisconnect(event->disconnect.reason, event->disconnect.conn); return 0;
        case BLE_GAP_EVENT_ENC_CHANGE: onGapEncryptionChange(event->enc_change.status, event->enc_change.conn_handle); return 0;
        case BLE_GAP_EVENT_NOTIFY_RX: onGapNotifyReceived(event->notify_rx.conn_handle, event->notify_rx.attr_handle, event->notify_rx.indication != 0, event->notify_rx.om); return 0;
        case BLE_GAP_EVENT_REPEAT_PAIRING: return onGapRepeatPairing(event->repeat_pairing);
        case BLE_GAP_EVENT_CONN_UPDATE_REQ: {
            if (event->conn_update_req.self_params->itvl_min < 24) { // 1.25ms units
                event->conn_update_req.self_params->itvl_min = 24;
            } else if (event->conn_update_req.self_params->itvl_max > 40) { // 1.25ms units
                event->conn_update_req.self_params->itvl_min = 40;
            } else if (event->conn_update_req.self_params->latency > 2) { // Number of intervals allowed to skip
                event->conn_update_req.self_params->latency = 2;
            } else if (event->conn_update_req.self_params->supervision_timeout > 100) { // 10ms units
                event->conn_update_req.self_params->supervision_timeout = 100;
            }
            return 0;
        }
        default: return 0;
    }
}

void BLE::connectIfInteresting(const ble_addr_t *addr, const struct ble_hs_adv_fields &advFields) {
    // Check for HID service
    bool hasHidService = false;
    for (int i = 0; i < advFields.num_uuids16; i++) {
        if (ble_uuid_u16(&advFields.uuids16[i].u) == 0x1812) {
            hasHidService = true;
            break;
        }
    }
    if (!hasHidService)
        return;

    // Check for gamepad appearance
    if (!advFields.appearance_is_present || advFields.appearance != 0x03C4) {
        return;
    }

    // {
    //     uint8_t peer_addr[6];
    //     sscanf(PEER_ADDR, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &peer_addr[5], &peer_addr[4], &peer_addr[3], &peer_addr[2], &peer_addr[1], &peer_addr[0]);
    //     if (memcmp(peer_addr, addr->val, sizeof(addr->val)) != 0) {
    //         return;
    //     }
    // }

    // Scanning must be stopped before a connection can be initiated.
    int rc;
    if ((rc = ble_gap_disc_cancel()) != 0) {
        ESP_LOGE(TAG, "Failed to cancel scan; rc=%d", rc);
        return;
    }

    // Figure out address to use for connect (no privacy for now)
    uint8_t ownAddrType;
    if ((rc = ble_hs_id_infer_auto(0, &ownAddrType)) != 0) {
        ESP_LOGI(TAG, "error determining address type; rc=%d", rc);
        return;
    }

    // Try to connect to the advertiser
    ble_gap_conn_params connParams = {
        .scan_itvl           = 16,
        .scan_window         = 16,
        .itvl_min            = 12,
        .itvl_max            = 12,
        .latency             = 0,
        .supervision_timeout = 51,
        .min_ce_len          = 0,
        .max_ce_len          = 0,
    };
    if ((rc = ble_gap_connect(ownAddrType, addr, 5000, &connParams, _onGapEvent, this)) != 0) {
        ESP_LOGE(TAG, "Error: Failed to connect to device; rc=%d", rc);
        return;
    }
}

#if 0
static void uuid_to_str(const ble_uuid_t *uuid, char *dst, size_t maxLen) {
    switch (uuid->type) {
        case BLE_UUID_TYPE_16: snprintf(dst, maxLen, "0x%04" PRIx16, BLE_UUID16(uuid)->value); break;
        case BLE_UUID_TYPE_32: snprintf(dst, maxLen, "0x%08" PRIx32, BLE_UUID32(uuid)->value); break;
        case BLE_UUID_TYPE_128: {
            const uint8_t *u8p = BLE_UUID128(uuid)->value;
            snprintf(dst, maxLen, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", u8p[15], u8p[14], u8p[13], u8p[12], u8p[11], u8p[10], u8p[9], u8p[8], u8p[7], u8p[6], u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
            break;
        }
        default: dst[0] = '\0'; break;
    }
}
#endif

void BLE::onDiscoveryComplete(const struct peer *peer, int status) {
    if (status != 0) {
        // Service discovery failed. Terminate the connection.
        ESP_LOGE(TAG, "Error: Service discovery failed; status=%d conn_handle=%d", status, peer->conn_handle);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

#if 0
    struct peer_svc *svc;
    SLIST_FOREACH(svc, &peer->svcs, next) {
        char uuidStr[64];
        uuid_to_str(reinterpret_cast<const ble_uuid_t *>(&svc->svc.uuid), uuidStr, sizeof(uuidStr));
        printf("- %s\n", uuidStr);

        struct peer_chr *chr;
        SLIST_FOREACH(chr, &svc->chrs, next) {
            uuid_to_str(reinterpret_cast<const ble_uuid_t *>(&chr->chr.uuid), uuidStr, sizeof(uuidStr));
            printf("  - %s  val_handle:%d\n", uuidStr, chr->chr.val_handle);
        }
    }
#endif

    // Get HID data handle
    {
        ble_uuid16_t svc_uuid = {.u = {.type = BLE_UUID_TYPE_16}, .value = 0x1812};
        ble_uuid16_t chr_uuid = {.u = {.type = BLE_UUID_TYPE_16}, .value = 0x2A4D};

        const struct peer_chr *chr = peer_chr_find_uuid(peer, reinterpret_cast<const ble_uuid_t *>(&svc_uuid), reinterpret_cast<const ble_uuid_t *>(&chr_uuid));
        if (chr) {
            hidDataHandle = chr->chr.val_handle;
        }
    }

    // ESP_LOGI(TAG, "Service discovery complete; status=%d conn_handle=%d", status, peer->conn_handle);

    // Get HID report descriptor
    {
        ble_uuid16_t svc_uuid = {.u = {.type = BLE_UUID_TYPE_16}, .value = 0x1812};
        ble_uuid16_t chr_uuid = {.u = {.type = BLE_UUID_TYPE_16}, .value = 0x2A4B};

        const struct peer_chr *chr = peer_chr_find_uuid(
            peer,
            reinterpret_cast<const ble_uuid_t *>(&svc_uuid),
            reinterpret_cast<const ble_uuid_t *>(&chr_uuid));
        if (chr) {
            reportDescOffset = 0;
            ble_gattc_read_long(peer->conn_handle, chr->chr.val_handle, 0, _onRead, this);
        }
    }
}

int BLE::onRead(uint16_t connHandle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr) {
    if (error->status == 0) {
        // ESP_LOGI(TAG, "onRead connHandle=%d attrHandle=%d attrOffset=%d size=%d", connHandle, attr->handle, attr->offset, OS_MBUF_PKTLEN(attr->om));

        auto len = OS_MBUF_PKTLEN(attr->om);
        if (reportDescOffset + len >= sizeof(reportDesc)) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        if (reportDescOffset + len < sizeof(reportDesc)) {
            if (os_mbuf_copydata(attr->om, 0, len, &reportDesc[reportDescOffset]) != 0)
                return 0;
            reportDescOffset += len;
        }
        return 0;
    }

    ESP_LOGI(TAG, "HID Report descriptor read: %d bytes\n", reportDescOffset);

    // printf("HID report descriptor\n");
    // ESP_LOG_BUFFER_HEXDUMP(TAG, reportDesc, reportDescOffset, ESP_LOG_INFO);

    reportHandlers = HIDReportHandler::getReportHandlersForDescriptor(reportDesc, reportDescOffset);
    return 0;
}
