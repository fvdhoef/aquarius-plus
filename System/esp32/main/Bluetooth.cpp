#include "Bluetooth.h"

#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
#include <nvs_flash.h>

#include "HIDReportHandler.h"

static const char *TAG = "Bluetooth";

extern "C" void ble_store_config_init(void);

#define MAX_KNOWN_DEVICES (5)
struct KnownDevice {
    ble_addr_t addr;
    char       name[32];

    bool isUnused() {
        return (
            addr.type == 0 &&
            addr.val[0] == 0 &&
            addr.val[1] == 0 &&
            addr.val[2] == 0 &&
            addr.val[3] == 0 &&
            addr.val[4] == 0 &&
            addr.val[5] == 0);
    }
};

struct GattDsc {
    ble_gatt_dsc dsc;
};

struct GattChr {
    ble_gatt_chr                chr;
    std::map<uint16_t, GattDsc> dscs;
    bool                        dscsDiscovered = false;
};

struct GattSvc {
    ble_gatt_svc                svc;
    std::map<uint16_t, GattChr> chrs;
    bool                        chrsDiscovered = false;
};

struct BluetoothDevInfo {
    ~BluetoothDevInfo() {
        if (reportHandlers) {
            delete reportHandlers;
            reportHandlers = nullptr;
        }
    }

    ble_addr_t  addr;
    uint16_t    appearance = 0;
    std::string name;
    int8_t      rssi = -128;

    uint8_t  flags         = 0;
    bool     isHID         = false;
    int      connHandle    = -1; // >=0: connected, <0: not connected
    uint32_t ticksLastSeen = 0;
    bool     discoveryDone = false;

    std::map<uint16_t, GattSvc> services;
    std::vector<uint8_t>        hidReportDesc;

    HIDReportHandler *reportHandlers = nullptr;
    uint16_t          hidDataHandle  = 0;
};

static bool operator<(const ble_addr_t &lhs, const ble_addr_t &rhs) {
    return memcmp(&lhs, &rhs, sizeof(ble_addr_t)) < 0;
}

class BluetoothInt : public Bluetooth {

    bool enabled  = false;
    bool scanning = false;

    KnownDevice                            knownDevices[MAX_KNOWN_DEVICES];
    SemaphoreHandle_t                      mutex;
    std::map<ble_addr_t, BluetoothDevInfo> devices;

    std::vector<ble_addr_t> tmpAddrList;

public:
    BluetoothInt() {
        mutex = xSemaphoreCreateRecursiveMutex();
    }

    void init() override {
        RecursiveMutexLock lock(mutex);

        // Restore config from NVS
        loadEnabled();
        loadKnownDevices();

        // Add known devices to devices list
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (knownDevices[i].isUnused())
                break;

            BluetoothDevInfo bdi;
            bdi.addr = knownDevices[i].addr;
            bdi.name = knownDevices[i].name;

            devices.insert(std::make_pair(knownDevices[i].addr, bdi));
        }

        // Start timer
        auto timer = xTimerCreate("bt", pdMS_TO_TICKS(2000), pdTRUE, this, _onTimer);
        xTimerStart(timer, 0);

        // Start Bluetooth if previously enabled
        if (enabled) {
            start();
        }
    }

    static void _onTimer(TimerHandle_t xTimer) { static_cast<BluetoothInt *>(pvTimerGetTimerID(xTimer))->onTimer(); }

    void onTimer() {
        RecursiveMutexLock lock(mutex);
        if (!enabled)
            return;

        if (!scanning)
            startScan();

        auto now = xTaskGetTickCount();

        for (auto it = devices.begin(); it != devices.end();) {
            bool remove = false;
            auto bdi    = &it->second;

            bool offline = bdi->connHandle < 0;

            if (offline) {
                if (findKnownDevice(bdi->addr) < 0) {
                    auto age = now - bdi->ticksLastSeen;
                    if (bdi->connHandle < 0 && age > pdMS_TO_TICKS(5000)) {
                        remove = true;
                    }
                }

            } else {
                // Update RSSI
                ble_gap_conn_rssi(bdi->connHandle, &bdi->rssi);
            }

            if (remove) {
                it = devices.erase(it);
            } else {
                ++it;
            }
        }
    }

    void start() {
        RecursiveMutexLock lock(mutex);
        auto               ret = nimble_port_init();
        if (ret != ESP_OK) {
            enabled = false;
            return;
        }

        ble_hs_cfg.sm_io_cap         = BLE_HS_IO_NO_INPUT_OUTPUT;
        ble_hs_cfg.sm_bonding        = 1;
        ble_hs_cfg.sm_mitm           = 1;
        ble_hs_cfg.sm_sc             = 1;
        ble_hs_cfg.sm_our_key_dist   = BLE_SM_PAIR_KEY_DIST_ENC;
        ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
        ble_hs_cfg.reset_cb          = _onReset;
        ble_hs_cfg.sync_cb           = _onSync;
        ble_hs_cfg.store_status_cb   = ble_store_util_status_rr;
        ble_store_config_init();

        // Retrieve list of bonded peers
        int count;
        if (ble_store_util_count(BLE_STORE_OBJ_TYPE_OUR_SEC, &count) == 0) {
            ESP_LOGW(TAG, "- BLE_STORE_OBJ_TYPE_OUR_SEC: %d", count);
        }
        if (ble_store_util_count(BLE_STORE_OBJ_TYPE_PEER_SEC, &count) == 0) {
            ESP_LOGW(TAG, "- BLE_STORE_OBJ_TYPE_PEER_SEC: %d", count);
        }
        if (ble_store_util_count(BLE_STORE_OBJ_TYPE_CCCD, &count) == 0) {
            ESP_LOGW(TAG, "- BLE_STORE_OBJ_TYPE_CCCD: %d", count);
        }
        if (ble_store_util_count(BLE_STORE_OBJ_TYPE_PEER_DEV_REC, &count) == 0) {
            ESP_LOGW(TAG, "- BLE_STORE_OBJ_TYPE_PEER_DEV_REC: %d", count);
        }
        if (ble_store_util_count(BLE_STORE_OBJ_TYPE_PEER_ADDR, &count) == 0) {
            ESP_LOGW(TAG, "- BLE_STORE_OBJ_TYPE_PEER_ADDR: %d", count);
        }
        if (ble_store_util_count(BLE_STORE_OBJ_TYPE_LOCAL_IRK, &count) == 0) {
            ESP_LOGW(TAG, "- BLE_STORE_OBJ_TYPE_LOCAL_IRK: %d", count);
        }

        // Get RSSI of connected device
        // esp_ble_gap_read_rssi()
        // ble_gap_conn_rssi

        // Remove bonds for devices not in our known devices list
        {
            tmpAddrList.clear();
            ble_store_iterate(BLE_STORE_OBJ_TYPE_PEER_SEC, getStoreAddrIter, nullptr);
            for (auto &addr : tmpAddrList) {
                if (findKnownDevice(addr) < 0) {
                    ESP_LOGW(TAG, "Removing bond for %s", toString(addr).c_str());
                    auto ret = ble_store_util_delete_peer(&addr);
                    if (ret != 0) {
                        ESP_LOGE(TAG, "ble_store_util_delete_peer: %d", ret);
                    }
                }
            }
            tmpAddrList.clear();
        }

        enabled = true;

        nimble_port_freertos_init(_hostTask);
    }

    static int getStoreAddrIter(int obj_type, union ble_store_value *val, void *) {
        auto bt = static_cast<BluetoothInt *>(getBluetooth());
        bt->tmpAddrList.push_back(val->sec.peer_addr);
        return 0;
    }

    static void _onReset(int reason) { static_cast<BluetoothInt *>(getBluetooth())->onReset(reason); }
    static void _onSync() { static_cast<BluetoothInt *>(getBluetooth())->onSync(); }
    static void _hostTask(void *) { static_cast<BluetoothInt *>(getBluetooth())->hostTask(); }
    static int  _onGapEvent(struct ble_gap_event *event, void *) { return static_cast<BluetoothInt *>(getBluetooth())->onGapEvent(event); }
    static int  _onGattSvcDisc(uint16_t connHandle, const struct ble_gatt_error *error, const struct ble_gatt_svc *service, void *arg) { return static_cast<BluetoothInt *>(getBluetooth())->onGattSvcDisc(connHandle, error, service); }
    static int  _onGattChrDisc(uint16_t connHandle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg) { return static_cast<BluetoothInt *>(getBluetooth())->onGattChrDisc(connHandle, error, chr, arg); }
    static int  _onGattDscDisc(uint16_t connHandle, const struct ble_gatt_error *error, uint16_t chrValHandle, const struct ble_gatt_dsc *dsc, void *arg) { return static_cast<BluetoothInt *>(getBluetooth())->onGattDscDisc(connHandle, error, chrValHandle, dsc, arg); }

    void hostTask() {
        ESP_LOGI(TAG, "Host task started");
        nimble_port_run();
        ESP_LOGI(TAG, "Host task returned");
        nimble_port_freertos_deinit();
    }

    void onReset(int reason) {
        ESP_LOGW(TAG, "onReset: %d", reason);
    }

    void onSync() {
        ESP_LOGI(TAG, "onSync");
        int rc = ble_hs_util_ensure_addr(0);
        assert(rc == 0);

        startScan();
    }

    void startScan() {
        RecursiveMutexLock lock(mutex);
        if (scanning) {
            // Already scanning
            return;
        }

        int rc;

        // Figure out address to use while advertising
        uint8_t ownAddrType;
        if ((rc = ble_hs_id_infer_auto(0, &ownAddrType)) != 0) {
            ESP_LOGE(TAG, "error determining address type; rc=%d", rc);
            return;
        }

        struct ble_gap_disc_params discParams = {
            .itvl              = 0,
            .window            = 0,
            .filter_policy     = 0,
            .limited           = 0,
            .passive           = 0,
            .filter_duplicates = 0,
        };

        if ((rc = ble_gap_disc(ownAddrType, BLE_HS_FOREVER, &discParams, _onGapEvent, nullptr)) == 0) {
            scanning = true;
        } else {
            ESP_LOGE(TAG, "Error initiating GAP discovery procedure; rc=%d", rc);
        }
    }

    static std::string toString(const ble_addr_t &addr) {
        char str[64];

        const char *typeStr = "";
        switch (addr.type) {
            case BLE_OWN_ADDR_PUBLIC: typeStr = "public"; break;
            case BLE_OWN_ADDR_RANDOM: typeStr = "random"; break;
            case BLE_OWN_ADDR_RPA_PUBLIC_DEFAULT: typeStr = "rpa_public"; break;
            case BLE_OWN_ADDR_RPA_RANDOM_DEFAULT: typeStr = "rpa_random"; break;
            default: typeStr = "?"; break;
        }

        snprintf(str, sizeof(str), "<%s>[%02x:%02x:%02x:%02x:%02x:%02x]", typeStr, addr.val[5], addr.val[4], addr.val[3], addr.val[2], addr.val[1], addr.val[0]);
        return str;
    }

    static std::string toString(const ble_uuid_any_t &uuid) {
        char tmp[40];
        ble_uuid_to_str(&uuid.u, tmp);
        return tmp;
    }

    void onGapExtDiscovery(const struct ble_gap_ext_disc_desc &eventInfo) {
        if ((eventInfo.props & BLE_HCI_ADV_CONN_MASK) == 0)
            // Not connectable
            return;

        // if (eventInfo.rssi > -40)
        //     ESP_LOGI(
        //         TAG, "onGapExtDiscovery props:%02X data_status:%02X addr:%s rssi:%d tx_power:%d sid:%u prim_phy:%u sec_phy:%u periodic_adv_itvl:%u directAddr:%s",
        //         eventInfo.props,
        //         eventInfo.data_status,
        //         toString(eventInfo.addr).c_str(),
        //         eventInfo.rssi,
        //         eventInfo.tx_power,
        //         eventInfo.sid,
        //         eventInfo.prim_phy,
        //         eventInfo.sec_phy,
        //         eventInfo.periodic_adv_itvl,
        //         toString(eventInfo.direct_addr).c_str());

        // ESP_LOG_BUFFER_HEXDUMP(TAG, eventInfo.data, eventInfo.length_data, ESP_LOG_INFO);

        RecursiveMutexLock lock(mutex);

        auto              devIt = devices.find(eventInfo.addr);
        BluetoothDevInfo *bdi   = (devIt != devices.end()) ? &devIt->second : nullptr;

        int  knownDeviceIdx = findKnownDevice(eventInfo.addr);
        bool isKnown        = knownDeviceIdx >= 0;

        bool             newDevice = (bdi == nullptr);
        BluetoothDevInfo newBdi;
        if (newDevice) {
            bdi       = &newBdi;
            bdi->addr = eventInfo.addr;
        }

        bdi->rssi          = eventInfo.rssi;
        bdi->ticksLastSeen = xTaskGetTickCount();

        // Parse advertisement data
        if (!bdi->discoveryDone) {
            const uint8_t *p         = eventInfo.data;
            int            remaining = eventInfo.length_data;

            while (remaining >= 2) {
                unsigned fieldLen = p[0];
                remaining--;
                p++;

                if (fieldLen > remaining)
                    break;

                uint8_t type = p[0];

                switch (type) {
                    case 0x01: { // Flags
                        if (fieldLen == 2)
                            bdi->flags = p[1];
                        break;
                    }

                    case 0x02:   // Incomplete List of 16-bit Service or Service Class UUIDs
                    case 0x03: { // Complete List of 16-bit Service or Service Class UUIDs
                        int numUUIDs = (fieldLen - 1) / 2;
                        for (int i = 0; i < numUUIDs; i++) {
                            uint16_t uuid = (p[i * 2 + 2] << 8) | p[i * 2 + 1];
                            if (uuid == 0x1812)
                                bdi->isHID = true;
                        }
                        break;
                    }

                    case 0x08: { // Shortened Local Name
                        if (!isKnown && bdi->name.empty())
                            bdi->name = std::string(p + 1, p + fieldLen);
                        break;
                    }

                    case 0x09: { // Complete Local Name
                        if (!isKnown)
                            bdi->name = std::string(p + 1, p + fieldLen);
                        break;
                    }

                    case 0x19: { // Appearance
                        if (fieldLen == 3)
                            bdi->appearance = (p[2] << 8) | p[1];
                        break;
                    }
                }
                p += fieldLen;
                remaining -= fieldLen;
            }

            // printf("flags: %02X\n", bdi->flags);

            if (!isKnown) {
                if ((bdi->flags & 3) == 0) {
                    // Device not in discoverable mode
                    return;
                }

                if (!bdi->isHID) {
                    // We only support HID devices (for now)
                    return;
                }
            }

            if (bdi->name.empty()) {
                // Use mac address instead
                char str[32];
                snprintf(str, sizeof(str), "%02x:%02x:%02x:%02x:%02x:%02x", bdi->addr.val[5], bdi->addr.val[4], bdi->addr.val[3], bdi->addr.val[2], bdi->addr.val[1], bdi->addr.val[0]);
                bdi->name = str;
            }

            if ((eventInfo.props & BLE_HCI_ADV_SCAN_RSP_MASK) || (eventInfo.props & BLE_HCI_ADV_SCAN_MASK) == 0) {
                bdi->discoveryDone = true;
            }
        }

        if (newDevice) {
            auto [it, result] = devices.insert(std::make_pair(eventInfo.addr, *bdi));
            bdi               = &it->second;
        }

        // Connect to the device if it is known
        if (isKnown && bdi->connHandle < 0) {
            connectDevice(eventInfo.addr);
        }
    }

    int findKnownDevice(ble_addr_t addr) {
        // Check if this is a known device
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (knownDevices[i].isUnused())
                break;
            if (memcmp(&addr, &knownDevices[i].addr, sizeof(addr)) == 0)
                return i;
        }
        return -1;
    }

    void connectDevice(ble_addr_t addr) {
        RecursiveMutexLock lock(mutex);

        // Stop scanning
        if (scanning) {
            auto ret = ble_gap_disc_cancel();
            if (ret != 0) {
                ESP_LOGE(TAG, "Failed to cancel scan: %d", ret);
                return;
            }
            scanning = false;
        }

        // Connect
        uint8_t own_addr_type;
        auto    ret = ble_hs_id_infer_auto(0, &own_addr_type);
        if (ret != 0) {
            ESP_LOGE(TAG, "Error determining address type: %d", ret);
            return;
        }
        ret = ble_gap_connect(own_addr_type, &addr, 5000, nullptr, _onGapEvent, this);
        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to connect to device: %d", ret);
            return;
        }
    }

    void onGapDiscoveryComplete(int reason) {
        ESP_LOGI(TAG, "onGapDiscoveryComplete %d", reason);
        RecursiveMutexLock lock(mutex);
        scanning = false;
    }
    void onGapConnect(int status, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapConnect status=%d connHandle=%u", status, connHandle);
        RecursiveMutexLock lock(mutex);

        startScan();

        // Resume scanning
        if (status != 0) {
            // Connection attempt failed; resume scanning.
            ESP_LOGE(TAG, "Connection failed, status=%d", status);
            return;
        }

        ble_gap_conn_desc connDesc;
        if (ble_gap_conn_find(connHandle, &connDesc) == 0) {
            auto it = devices.find(connDesc.peer_ota_addr);
            if (it == devices.end()) {
                // Device not found. This should not happen normally.
                ESP_LOGE(TAG, "Device not found!");
                ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
                return;
            }

            if (it->second.connHandle >= 0) {
                ESP_LOGW(TAG, "Connection handle already set!");
            }

            it->second.connHandle = connHandle;
        }

        // Secure connection
        int ret;
        if ((ret = ble_gap_security_initiate(connHandle)) != 0) {
            ESP_LOGE(TAG, "ble_gap_security_initiate: %d", ret);
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);

        } else {
            ESP_LOGI(TAG, "Connection security initiated");
        }
    }
    void onGapDisconnect(int reason, const struct ble_gap_conn_desc &connDesc) {
        // Connection terminated
        ESP_LOGI(TAG, "onGapDisconnect reason=%d", reason);
        RecursiveMutexLock lock(mutex);

        auto it = devices.find(connDesc.peer_ota_addr);
        if (it != devices.end()) {
            auto bdi = &it->second;

            bdi->ticksLastSeen = xTaskGetTickCount();
            bdi->connHandle    = -1;
            bdi->rssi          = -128;
            bdi->hidDataHandle = 0;

            // Delete report handlers
            if (bdi->reportHandlers) {
                delete bdi->reportHandlers;
                bdi->reportHandlers = nullptr;
            }
        }

        // Resume scanning
        startScan();
    }
    void onGapEncryptionChange(int status, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapEncryptionChange status=%d connHandle=%u", status, connHandle);
        if (status != 0) {
            ESP_LOGE(TAG, "onGapEncryptionChange: %d", status);
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
        }

        RecursiveMutexLock lock(mutex);

        auto bdi = getBdiFromConnHandle(connHandle);
        if (!bdi) {
            // Device not found. This should not happen normally.
            ESP_LOGE(TAG, "Device not found!");
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return;
        }
        discoverSvcs(bdi);
    }
    void onGapNotifyReceived(uint16_t connHandle, uint16_t attrHandle, bool isIndication, struct os_mbuf *om) {
        // ESP_LOGI(TAG, "onGapNotifyReceived connHandle=%u attrHandle=%u", connHandle, attrHandle);
        RecursiveMutexLock lock(mutex);

        auto bdi = getBdiFromConnHandle(connHandle);
        if (!bdi) {
            // Device not found. This should not happen normally.
            ESP_LOGE(TAG, "Device not found!");
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return;
        }

        std::vector<uint8_t> buf;
        auto                 size = OS_MBUF_PKTLEN(om);
        buf.resize(size);
        os_mbuf_copydata(om, 0, size, buf.data());

        if (attrHandle == bdi->hidDataHandle) {
            HIDReportHandler *reportHandler = bdi->reportHandlers;
            while (reportHandler) {
                // FIXME: How do report Ids work in Bluetooth?
                if (reportHandler->hasReportId)
                    reportHandler->hasReportId = false;

                reportHandler->inputReport(buf.data(), buf.size());
                reportHandler = reportHandler->next;
            }
        }

        // ESP_LOG_BUFFER_HEX(TAG, buf.data(), buf.size());
    }
    int onGapRepeatPairing(const struct ble_gap_repeat_pairing &eventInfo) {
        ESP_LOGI(TAG, "onGapRepeatPairing");
        return BLE_GAP_REPEAT_PAIRING_IGNORE; // BLE_GAP_REPEAT_PAIRING_RETRY;
    }
    void onGapConnUpdate(int status, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapConnUpdate status=%d connHandle=%u", status, connHandle);
    }
    void onGapConnUpdateReq(const struct ble_gap_upd_params *peerParams, struct ble_gap_upd_params *selfParams, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapConnUpdateReq");
        if (selfParams->itvl_min < 24) { // 1.25ms units
            selfParams->itvl_min = 24;
        } else if (selfParams->itvl_max > 40) { // 1.25ms units
            selfParams->itvl_min = 40;
        } else if (selfParams->latency > 2) { // Number of intervals allowed to skip
            selfParams->latency = 2;
        } else if (selfParams->supervision_timeout > 100) { // 10ms units
            selfParams->supervision_timeout = 100;
        }
    }
    void onGapL2CapUpdateReq(const struct ble_gap_upd_params *peerParams, struct ble_gap_upd_params *selfParams, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapL2CapUpdateReq");
    }
    void onGapPairingComplete(int status, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapPairingComplete status=%d connHandle=%u", status, connHandle);
    }
#if 0
    void onGapPassKeyAction(const ble_gap_passkey_params *params, uint16_t connHandle) {
        ESP_LOGI(TAG, "onGapPassKeyAction action=%u numcmp=%u connHandle=%u", params->action, (unsigned)params->numcmp, connHandle);

        if (params->action == BLE_SM_IOACT_DISP) {
            ble_sm_io pkey = {0};
            pkey.passkey   = 123456;
            ble_sm_inject_io(connHandle, &pkey);

            ESP_LOGI(TAG, "Please enter 123456 on your device");
            // TODO: show this on-screen
        }
    }
#endif

    BluetoothDevInfo *getBdiFromConnHandle(uint16_t connHandle) {
        ble_gap_conn_desc connDesc;
        if (ble_gap_conn_find(connHandle, &connDesc) != 0) {
            return nullptr;
        }

        auto it = devices.find(connDesc.peer_ota_addr);
        if (it == devices.end()) {
            return nullptr;
        }
        return &it->second;
    }

    static int _onGattAttrWritten(uint16_t connHandle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
        ESP_LOGI(TAG, "_onGattAttrWritten connHandle=%u status=%u att_handle=%u", connHandle, error->status, error->att_handle);
        return 0;
    }

    static int _onReadHidDsc(uint16_t connHandle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) { return static_cast<BluetoothInt *>(getBluetooth())->onReadHidDsc(connHandle, error, attr, arg); }

    int onReadHidDsc(uint16_t connHandle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
        // ESP_LOGI(TAG, "_onReadHidDsc connHandle=%u status=%u att_handle=%u", connHandle, error->status, error->att_handle);
        RecursiveMutexLock lock(mutex);

        auto bdi = getBdiFromConnHandle(connHandle);
        if (!bdi) {
            // Device not found. This should not happen normally.
            ESP_LOGE(TAG, "Device not found!");
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        if (error->status == 0) {
            if (bdi->hidReportDesc.size() != attr->offset) {
                ESP_LOGE(TAG, "Incorrect offset");
                return 0;
            }

            auto size = OS_MBUF_PKTLEN(attr->om);
            bdi->hidReportDesc.resize(attr->offset + size);
            if (os_mbuf_copydata(attr->om, 0, size, bdi->hidReportDesc.data() + attr->offset) != 0) {
                ESP_LOGE(TAG, "Error copying data");
                return 0;
            }
            return 0;

        } else if (error->status == BLE_HS_EDONE) {
            // Full descriptor received
            ESP_LOGI(TAG, "HID Report descriptor:");
            ESP_LOG_BUFFER_HEX(TAG, bdi->hidReportDesc.data(), bdi->hidReportDesc.size());

            bdi->reportHandlers = HIDReportHandler::getReportHandlersForDescriptor(bdi->hidReportDesc.data(), bdi->hidReportDesc.size());

            writeCCCs(bdi);
        }
        return 0;
    }

    void writeCCCs(BluetoothDevInfo *bdi) {
        for (auto &[key, svc] : bdi->services) {
            for (auto &[key, chr] : svc.chrs) {
                for (auto &[key, dsc] : chr.dscs) {
                    if (chr.chr.uuid.u.type == BLE_UUID_TYPE_16 && chr.chr.uuid.u16.value == 0x2a4d && // Report
                        (chr.chr.properties & 0x10) != 0 &&                                            // Indication?
                        dsc.dsc.uuid.u.type == BLE_UUID_TYPE_16 && dsc.dsc.uuid.u16.value == 0x2902) { // CCC descriptor

                        if (bdi->hidDataHandle == 0) {
                            bdi->hidDataHandle = chr.chr.val_handle;

                            uint16_t value = 1; // Enable indication
                            int      ret   = ble_gattc_write_flat(bdi->connHandle, dsc.dsc.handle, &value, sizeof(value), _onGattAttrWritten, nullptr);
                            if (ret != 0) {
                                ESP_LOGE(TAG, "ble_gattc_write_flat: %d", ret);
                            }
                        }
                    }
                }
            }
        }
    }

    void readHidReportDesc(BluetoothDevInfo *bdi) {
        for (auto &[key, svc] : bdi->services) {
            for (auto &[key, chr] : svc.chrs) {
                if (chr.chr.uuid.u.type == BLE_UUID_TYPE_16 && chr.chr.uuid.u16.value == 0x2a4b) {
                    bdi->hidReportDesc.clear();
                    int ret = ble_gattc_read_long(bdi->connHandle, chr.chr.val_handle, 0, _onReadHidDsc, nullptr);
                    if (ret != 0) {
                        ESP_LOGE(TAG, "ble_gattc_read_long: %d", ret);
                    }
                    return;
                }
            }
        }
    }

    void onDiscoveryDone(BluetoothDevInfo *bdi) {
        RecursiveMutexLock lock(mutex);

        readHidReportDesc(bdi);

        // Done discovery
        ESP_LOGI(TAG, "Discovery done for connHandle=%d", bdi->connHandle);
        for (auto &[key, svc] : bdi->services) {
            ESP_LOGI(TAG, "- %04x-%04x uuid=%s", svc.svc.start_handle, svc.svc.end_handle, toString(svc.svc.uuid).c_str());

            for (auto &[key, chr] : svc.chrs) {
                ESP_LOGI(TAG, "  - def_handle=%04x val_handle=%04x properties=%02x uuid=%s", chr.chr.def_handle, chr.chr.val_handle, chr.chr.properties, toString(chr.chr.uuid).c_str());
            }
        }
    }

    void discoverDscs(BluetoothDevInfo *bdi) {
        for (auto &[svcKey, svc] : bdi->services) {
            for (auto it = svc.chrs.begin(); it != svc.chrs.end(); ++it) {
                auto &chr = it->second;
                if (chr.dscsDiscovered)
                    continue;

                chr.dscsDiscovered = true;

                uint16_t endHandle;
                {
                    auto nextIt = it;
                    ++nextIt;
                    endHandle = (nextIt != svc.chrs.end()) ? (nextIt->second.chr.def_handle - 1) : svc.svc.end_handle;
                }
                if (endHandle <= chr.chr.val_handle)
                    continue;

                // ESP_LOGI(
                //     TAG, "ble_gattc_disc_all_dscs  conn_handle=%u start_handle=%u end_handle=%u",
                //     bdi->connHandle, chr.chr.val_handle, endHandle);

                auto ret = ble_gattc_disc_all_dscs(bdi->connHandle, chr.chr.val_handle, endHandle, _onGattDscDisc, &chr);
                if (ret != 0) {
                    ESP_LOGE(TAG, "ble_gattc_disc_all_dscs: %d", ret);
                }
                return;
            }
        }

        onDiscoveryDone(bdi);
    }

    void discoverChars(BluetoothDevInfo *bdi) {
        if (!bdi)
            return;

        // Search for services with undiscovered characteristics
        for (auto &[key, svc] : bdi->services) {
            if (svc.chrsDiscovered)
                continue;

            svc.chrsDiscovered = true;
            auto ret           = ble_gattc_disc_all_chrs(bdi->connHandle, svc.svc.start_handle, svc.svc.end_handle, _onGattChrDisc, &svc);
            if (ret != 0) {
                ESP_LOGE(TAG, "ble_gattc_disc_all_chrs: %d", ret);
            }
            return;
        }

        discoverDscs(bdi);
    }

    void discoverSvcs(BluetoothDevInfo *bdi) {
        if (!bdi)
            return;

        if (bdi->services.empty()) {
            auto ret = ble_gattc_disc_all_svcs(bdi->connHandle, _onGattSvcDisc, nullptr);
            if (ret != 0) {
                ESP_LOGE(TAG, "ble_gattc_disc_all_svcs: %d", ret);
            }
            return;
        }

        onDiscoveryDone(bdi);
    }

    int onGattDscDisc(uint16_t connHandle, const struct ble_gatt_error *error, uint16_t chrValHandle, const struct ble_gatt_dsc *dsc, void *arg) {
        // ESP_LOGI(TAG, "onGattDscDisc  status:%u  att_handle:%u", error->status, error->att_handle);
        RecursiveMutexLock lock(mutex);

        auto bdi = getBdiFromConnHandle(connHandle);
        if (!bdi) {
            // Device not found. This should not happen normally.
            ESP_LOGE(TAG, "Device not found!");
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return -1;
        }

        if (error->status == 0) {
            // ESP_LOGI(TAG, "dsc handle: %u", dsc->handle);

            GattDsc gattDsc;
            gattDsc.dsc = *dsc;

            auto chr = static_cast<GattChr *>(arg);
            assert(chr != nullptr);

            chr->dscs.insert(std::make_pair(dsc->handle, gattDsc));

        } else if (error->status == BLE_HS_EDONE) {
            discoverDscs(bdi);

        } else {
            ESP_LOGE(TAG, "onGattDscDisc - Error during discovery! %d", error->status);
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return -1;
        }

        return 0;
    }

    int onGattChrDisc(uint16_t connHandle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg) {
        // ESP_LOGI(TAG, "onGattChrDisc  status:%u  att_handle:%u", error->status, error->att_handle);
        RecursiveMutexLock lock(mutex);

        auto bdi = getBdiFromConnHandle(connHandle);
        if (!bdi) {
            // Device not found. This should not happen normally.
            ESP_LOGE(TAG, "Device not found!");
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return -1;
        }

        if (error->status == 0) {
            // ESP_LOGI(TAG, "chr handle: %u", chr->def_handle);

            GattChr gattChr;
            gattChr.chr = *chr;

            auto svc = static_cast<GattSvc *>(arg);
            assert(svc != nullptr);

            svc->chrs.insert(std::make_pair(chr->def_handle, gattChr));

        } else if (error->status == BLE_HS_EDONE) {
            discoverChars(bdi);

        } else {
            ESP_LOGE(TAG, "onGattChrDisc - Error during discovery! %d", error->status);
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return -1;
        }

        return 0;
    }

    int onGattSvcDisc(uint16_t connHandle, const struct ble_gatt_error *error, const struct ble_gatt_svc *svc) {
        // ESP_LOGI(TAG, "onGattSvcDisc  status:%u  att_handle:%u", error->status, error->att_handle);
        RecursiveMutexLock lock(mutex);

        auto bdi = getBdiFromConnHandle(connHandle);
        if (!bdi) {
            // Device not found. This should not happen normally.
            ESP_LOGE(TAG, "Device not found!");
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return -1;
        }

        if (error->status == 0) {
            // ESP_LOGI(TAG, "svc handle: %u-%u", svc->start_handle, svc->end_handle);

            GattSvc gattSvc;
            gattSvc.svc = *svc;

            // Only add services we are interested in
            if (svc->uuid.u.type == BLE_UUID_TYPE_16 &&
                (svc->uuid.u16.value == 0x180f ||  // Battery service
                 svc->uuid.u16.value == 0x1812)) { // HID service
                bdi->services.insert(std::make_pair(svc->start_handle, gattSvc));
            }

        } else if (error->status == BLE_HS_EDONE) {
            discoverChars(bdi);

        } else {
            ESP_LOGE(TAG, "onGattSvcDisc - Error during discovery! %d", error->status);
            ble_gap_terminate(connHandle, BLE_ERR_REM_USER_CONN_TERM);
            return -1;
        }
        return 0;
    }

    int onGapEvent(struct ble_gap_event *event) {
        switch (event->type) {
            case BLE_GAP_EVENT_DISC: ESP_LOGE(TAG, "BLE_GAP_EVENT_DISC should not happen"); return 0;
            case BLE_GAP_EVENT_DISC_COMPLETE: onGapDiscoveryComplete(event->disc_complete.reason); return 0;
            case BLE_GAP_EVENT_EXT_DISC: onGapExtDiscovery(event->ext_disc); return 0;
            case BLE_GAP_EVENT_CONNECT: onGapConnect(event->connect.status, event->connect.conn_handle); return 0;
            case BLE_GAP_EVENT_DISCONNECT: onGapDisconnect(event->disconnect.reason, event->disconnect.conn); return 0;
            case BLE_GAP_EVENT_ENC_CHANGE: onGapEncryptionChange(event->enc_change.status, event->enc_change.conn_handle); return 0;
            case BLE_GAP_EVENT_NOTIFY_RX: onGapNotifyReceived(event->notify_rx.conn_handle, event->notify_rx.attr_handle, event->notify_rx.indication != 0, event->notify_rx.om); return 0;
            case BLE_GAP_EVENT_REPEAT_PAIRING: return onGapRepeatPairing(event->repeat_pairing);
            case BLE_GAP_EVENT_CONN_UPDATE: onGapConnUpdate(event->conn_update.status, event->conn_update.conn_handle); return 0;
            case BLE_GAP_EVENT_CONN_UPDATE_REQ: onGapConnUpdateReq(event->conn_update_req.peer_params, event->conn_update_req.self_params, event->conn_update_req.conn_handle); return 0;
            case BLE_GAP_EVENT_L2CAP_UPDATE_REQ: onGapL2CapUpdateReq(event->conn_update_req.peer_params, event->conn_update_req.self_params, event->conn_update_req.conn_handle); return 0;
            case BLE_GAP_EVENT_PARING_COMPLETE: onGapPairingComplete(event->pairing_complete.status, event->pairing_complete.conn_handle); return 0;
            // case BLE_GAP_EVENT_PASSKEY_ACTION: onGapPassKeyAction(&event->passkey.params, event->passkey.conn_handle); return 0;
            default: {
                ESP_LOGW(TAG, "Unhandled gap event: %d", event->type);
                return 0;
            }
        }
    }

    void stop() {
        auto ret = nimble_port_stop();
        if (ret == ESP_OK) {
            nimble_port_deinit();

            {
                RecursiveMutexLock lock(mutex);
                enabled = false;
            }
        } else {
            ESP_LOGE(TAG, "nimble_port_stop failed: %d", ret);
        }
    }

    bool getEnabled() override {
        RecursiveMutexLock lock(mutex);
        return enabled;
    }

    void setEnabled(bool enable) override {
        bool newState = false;

        {
            RecursiveMutexLock lock(mutex);
            if (enable == enabled)
                return;
            newState = enable;
        }

        if (newState)
            start();
        else
            stop();

        {
            RecursiveMutexLock lock(mutex);
            if (enable == enabled)
                saveEnabled();
        }
    }

    void loadEnabled() {
        RecursiveMutexLock lock(mutex);
        enabled = false;

        nvs_handle_t h;
        if (nvs_open("bt_settings", NVS_READONLY, &h) == ESP_OK) {
            uint8_t val;
            if (nvs_get_u8(h, "enabled", &val) == ESP_OK) {
                enabled = val != 0;
            }
            nvs_close(h);
        }
    }

    void saveEnabled() {
        RecursiveMutexLock lock(mutex);

        nvs_handle_t h;
        if (nvs_open("bt_settings", NVS_READWRITE, &h) == ESP_OK) {
            if (nvs_set_u8(h, "enabled", enabled ? 1 : 0) == ESP_OK) {
                nvs_commit(h);
            }
            nvs_close(h);
        }
    }

    void loadKnownDevices() {
        RecursiveMutexLock lock(mutex);
        memset(knownDevices, 0, sizeof(knownDevices));

        nvs_handle_t h;
        if (nvs_open("bt_settings", NVS_READONLY, &h) == ESP_OK) {
            size_t len = sizeof(knownDevices);
            if (nvs_get_blob(h, "known_devs", knownDevices, &len) != ESP_OK || len != sizeof(knownDevices)) {
                memset(knownDevices, 0, sizeof(knownDevices));
            }
            nvs_close(h);
        }
    }

    void saveKnownDevices() {
        RecursiveMutexLock lock(mutex);

        nvs_handle_t h;
        if (nvs_open("bt_settings", NVS_READWRITE, &h) == ESP_OK) {
            if (nvs_set_blob(h, "known_devs", knownDevices, sizeof(knownDevices)) == ESP_OK) {
                nvs_commit(h);
            }
            nvs_close(h);
        }
    }

    bool addDevice(const BleAddr &_addr, const std::string &name) override {
        RecursiveMutexLock lock(mutex);
        ble_addr_t         addr = *reinterpret_cast<const ble_addr_t *>(&_addr);

        auto it = devices.find(addr);
        if (it != devices.end()) {
            if (it->second.connHandle >= 0)
                // Already connected
                return true;
        }

        // Find free known device slot
        int idx = -1;
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (knownDevices[i].isUnused()) {
                idx = i;
                break;
            }
        }
        if (idx < 0) {
            // No free spot found, not going to add
            return false;
        }

        auto &dev = knownDevices[idx];
        dev.addr  = addr;
        snprintf(dev.name, sizeof(dev.name), name.c_str());

        saveKnownDevices();

        // Update name in devices list
        if (it != devices.end()) {
            it->second.name = (const char *)dev.name;
        }

        // Connect to device
        connectDevice(addr);

        return true;
    }

    void forgetDevice(const BleAddr &_addr) override {
        RecursiveMutexLock lock(mutex);
        ble_addr_t         addr = *reinterpret_cast<const ble_addr_t *>(&_addr);

        // Disconnect and remove device from devices map
        auto it = devices.find(addr);
        if (it != devices.end()) {
            if (it->second.connHandle >= 0) {
                if (enabled) {
                    // Disconnect device
                    auto ret = ble_gap_terminate(it->second.connHandle, BLE_ERR_REM_USER_CONN_TERM);
                    if (ret != ESP_OK) {
                        ESP_LOGE(TAG, "Error disconnecting device: %d", ret);
                    }
                }
                it->second.connHandle = -1;
            }
            devices.erase(it);
        }

        // Remove from BLE store
        if (enabled) {
            auto ret = ble_store_util_delete_peer(&addr);
            if (ret != 0) {
                ESP_LOGE(TAG, "ble_store_util_delete_peer: %d", ret);
            }
        }

        // Remove from known devices
        int idx = -1;
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (memcmp(&knownDevices[i].addr, &addr, sizeof(addr)) == 0) {
                idx = i;
                break;
            }
        }
        if (idx < 0) {
            // Device not found
            return;
        }

        while (idx < MAX_KNOWN_DEVICES - 1) {
            knownDevices[idx] = knownDevices[idx + 1];
            idx++;
        }
        memset(&knownDevices[idx], 0, sizeof(knownDevices[idx]));
        saveKnownDevices();
    }

    BluetoothInfo getBluetoothInfo() override {
        RecursiveMutexLock lock(mutex);

        BluetoothInfo result;

        for (auto &[addr, dev] : devices) {
            BtDevInfo btdi;
            btdi.addr       = *reinterpret_cast<BleAddr *>(&dev.addr);
            btdi.name       = dev.name;
            btdi.appearance = dev.appearance;
            btdi.rssi       = dev.rssi;

            if (dev.connHandle >= 0) {
                result.connectedDevices.push_back(btdi);
            } else {
                if (findKnownDevice(addr) >= 0)
                    result.knownDevices.push_back(btdi);
                else
                    result.otherDevices.push_back(btdi);
            }
        }
        return result;
    }
};

Bluetooth *getBluetooth() {
    static BluetoothInt obj;
    return &obj;
}
