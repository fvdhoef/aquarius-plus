#include "bluetooth.h"
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
#include <esp_central.h>
#include <math.h>
#include "fpga.h"

static const char *TAG = "bluetooth";

void ble_store_config_init(void);

static void blecent_scan(void);

static void print_uuid2(const ble_uuid_t *uuid) {
    char buf[BLE_UUID_STR_LEN];

    printf("%s", ble_uuid_to_str(uuid, buf));
}

#if 0
static void print_bytes2(const uint8_t *bytes, int len) {
    int i;

    for (i = 0; i < len; i++) {
        printf("%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

static void print_adv_fields2(const struct ble_hs_adv_fields *fields) {
    char           s[BLE_HS_ADV_MAX_SZ];
    const uint8_t *u8p;
    int            i;

    if (fields->uuids16 == NULL &&
        fields->uuids32 == NULL &&
        fields->uuids128 == NULL &&
        fields->name == NULL &&
        fields->svc_data_uuid16 == NULL &&
        fields->svc_data_uuid32 == NULL &&
        fields->svc_data_uuid128 == NULL &&
        fields->uri == NULL)
        return;

    printf("adv_fields:\n");

    if (fields->flags != 0) {
        printf("    flags=0x%02x\n", fields->flags);
    }

    if (fields->uuids16 != NULL) {
        printf("    uuids16(%scomplete)=", fields->uuids16_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids16; i++) {
            print_uuid2(&fields->uuids16[i].u);
            printf(" ");
        }
        printf("\n");
    }

    if (fields->uuids32 != NULL) {
        printf("    uuids32(%scomplete)=", fields->uuids32_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids32; i++) {
            print_uuid2(&fields->uuids32[i].u);
            printf(" ");
        }
        printf("\n");
    }

    if (fields->uuids128 != NULL) {
        printf("    uuids128(%scomplete)=", fields->uuids128_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids128; i++) {
            print_uuid2(&fields->uuids128[i].u);
            printf(" ");
        }
        printf("\n");
    }

    if (fields->name != NULL) {
        assert(fields->name_len < sizeof s - 1);
        memcpy(s, fields->name, fields->name_len);
        s[fields->name_len] = '\0';
        printf("    name(%scomplete)=%s\n", fields->name_is_complete ? "" : "in", s);
    }

    if (fields->tx_pwr_lvl_is_present) {
        printf("    tx_pwr_lvl=%d\n", fields->tx_pwr_lvl);
    }

    if (fields->slave_itvl_range != NULL) {
        printf("    slave_itvl_range=");
        print_bytes2(fields->slave_itvl_range, BLE_HS_ADV_SLAVE_ITVL_RANGE_LEN);
        printf("\n");
    }

    if (fields->svc_data_uuid16 != NULL) {
        printf("    svc_data_uuid16=");
        print_bytes2(fields->svc_data_uuid16, fields->svc_data_uuid16_len);
        printf("\n");
    }

    if (fields->public_tgt_addr != NULL) {
        printf("    public_tgt_addr=");
        u8p = fields->public_tgt_addr;
        for (i = 0; i < fields->num_public_tgt_addrs; i++) {
            printf("public_tgt_addr=%s ", addr_str(u8p));
            u8p += BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN;
        }
        printf("\n");
    }

    if (fields->appearance_is_present) {
        printf("    appearance=0x%04x\n", fields->appearance);
    }

    if (fields->adv_itvl_is_present) {
        printf("    adv_itvl=0x%04x\n", fields->adv_itvl);
    }

    if (fields->svc_data_uuid32 != NULL) {
        printf("    svc_data_uuid32=");
        print_bytes2(fields->svc_data_uuid32, fields->svc_data_uuid32_len);
        printf("\n");
    }

    if (fields->svc_data_uuid128 != NULL) {
        printf("    svc_data_uuid128=");
        print_bytes2(fields->svc_data_uuid128, fields->svc_data_uuid128_len);
        printf("\n");
    }

    if (fields->uri != NULL) {
        printf("    uri=");
        print_bytes2(fields->uri, fields->uri_len);
        printf("\n");
    }

    if (fields->mfg_data != NULL) {
        printf("    mfg_data=");
        print_bytes2(fields->mfg_data, fields->mfg_data_len);
        printf("\n");
    }
}
#endif

void print_mbuf2(const struct os_mbuf *om) {
    int colon, i;

    colon = 0;
    while (om != NULL) {
        if (colon) {
            printf(":");
        } else {
            colon = 1;
        }
        for (i = 0; i < om->om_len; i++) {
            printf("%s0x%02x", i != 0 ? ":" : "", om->om_data[i]);
        }
        om = SLIST_NEXT(om, om_next);
    }
}

static int blecent_on_subscribe2(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
    ESP_LOGI(TAG, "Subscribe complete; status=%d conn_handle=%d attr_handle=%d", error->status, conn_handle, attr->handle);
    return 0;
}

static int blecent_on_subscribe1(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
    ESP_LOGI(TAG, "Subscribe complete; status=%d conn_handle=%d attr_handle=%d", error->status, conn_handle, attr->handle);

    struct peer *peer;
    peer = peer_find(conn_handle);
    if (peer == NULL) {
        ESP_LOGE(TAG, "Error in finding peer, aborting...");
        ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return 0;
    }

    const struct peer_dsc *dsc;
    dsc = peer_dsc_find_uuid(peer, BLE_UUID16_DECLARE(0x180f), BLE_UUID16_DECLARE(0x2a19), BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if (dsc == NULL) {
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return 0;
    }

    uint16_t value = 1;
    int      rc    = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, &value, sizeof(value), blecent_on_subscribe2, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to subscribe to characteristic; rc=%d", rc);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return 0;
    }

    return 0;
}

static void blecent_on_disc_complete(const struct peer *peer, int status, void *arg) {
    if (status != 0) {
        /* Service discovery failed.  Terminate the connection. */
        ESP_LOGE(TAG, "Service discovery failed; status=%d conn_handle=%d", status, peer->conn_handle);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

    // Service discovery has completed successfully. Now we have a complete list of services, characteristics, and descriptors that the peer supports.
    ESP_LOGI(TAG, "Service discovery complete; status=%d conn_handle=%d", status, peer->conn_handle);
    {
        const struct peer_svc *svc;

        SLIST_FOREACH(svc, &peer->svcs, next) {
            printf("- ");
            print_uuid2((const ble_uuid_t *)&svc->svc.uuid);
            printf("\n");

            const struct peer_chr *chr;
            SLIST_FOREACH(chr, &svc->chrs, next) {
                printf("  - ");
                print_uuid2((const ble_uuid_t *)&chr->chr.uuid);
                printf(" [%02X]", chr->chr.properties);
                printf("\n");

                const struct peer_dsc *dsc;
                SLIST_FOREACH(dsc, &chr->dscs, next) {
                    printf("    - ");
                    print_uuid2((const ble_uuid_t *)&dsc->dsc.uuid);
                    printf("\n");
                }
            }
        }
    }

    const struct peer_dsc *dsc;
    dsc = peer_dsc_find_uuid(peer, BLE_UUID16_DECLARE(0x1812), BLE_UUID16_DECLARE(0x2a4d), BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if (dsc == NULL) {
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

    uint16_t value = 1;
    int      rc    = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, &value, sizeof(value), blecent_on_subscribe1, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to subscribe to characteristic; rc=%d", rc);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }
}

#pragma pack(push, 1)
struct hid_data {
    uint16_t lx;
    uint16_t ly;
    uint16_t rx;
    uint16_t ry;
    uint16_t lt;
    uint16_t rt;
    uint8_t  dpad;

    unsigned btn_a : 1;
    unsigned btn_b : 1;
    unsigned : 1;
    unsigned btn_x : 1;
    unsigned btn_y : 1;
    unsigned : 1;
    unsigned btn_lb : 1;
    unsigned btn_rb : 1;

    unsigned : 2;
    unsigned btn_view : 1;
    unsigned btn_menu : 1;
    unsigned btn_xbox : 1;
    unsigned btn_l3 : 1;
    unsigned btn_r3 : 1;
    unsigned : 1;

    unsigned btn_share : 1;
    unsigned : 7;
};
#pragma pack(pop)

static int blecent_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_DISC: {
            // ESP_LOGW(TAG, "BLE_GAP_EVENT_DISC");

            struct ble_hs_adv_fields fields;

            int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
            if (rc != 0) {
                return 0;
            }

            /* An advertisement report was received during GAP discovery. */
            // print_adv_fields2(&fields);

            if (fields.appearance_is_present && fields.appearance == 0x03C4) {
                ble_gap_disc_cancel();

                uint8_t own_addr_type;
                ble_hs_id_infer_auto(0, &own_addr_type);

                ESP_LOGI(TAG, "Connecting");
                ble_gap_connect(own_addr_type, &event->disc.addr, 5000, NULL, blecent_gap_event, NULL);
            }
            break;
        }
        case BLE_GAP_EVENT_CONNECT: {
            ESP_LOGW(TAG, "BLE_GAP_EVENT_CONNECT");

            if (event->connect.status != 0) {
                // Connection attempt failed, resume scanning
                ESP_LOGE(TAG, "Connect failed");
                blecent_scan();
                return 0;
            }

            ESP_LOGI(TAG, "Connection established");

            struct ble_gap_conn_desc desc;
            int                      rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            print_conn_desc(&desc);
            printf("\n");

            rc = peer_add(event->connect.conn_handle);
            if (rc != 0) {
                ESP_LOGE(TAG, "Failed to add peer; rc=%d", rc);
                return 0;
            }

            /** Initiate security - It will perform
             * Pairing (Exchange keys)
             * Bonding (Store keys)
             * Encryption (Enable encryption)
             * Will invoke event BLE_GAP_EVENT_ENC_CHANGE
             **/
            rc = ble_gap_security_initiate(event->connect.conn_handle);
            if (rc != 0) {
                ESP_LOGI(TAG, "Security could not be initiated, rc = %d", rc);
                return ble_gap_terminate(event->connect.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
            } else {
                ESP_LOGI(TAG, "Connection secured");
            }

            // rc = peer_disc_all(event->connect.conn_handle, blecent_on_disc_complete, NULL);
            // if (rc != 0) {
            //     ESP_LOGE(TAG, "Failed to discover services; rc=%d", rc);
            //     return 0;
            // }
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: {
            ESP_LOGW(TAG, "BLE_GAP_EVENT_DISCONNECT");

            MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
            print_conn_desc(&event->disconnect.conn);
            MODLOG_DFLT(INFO, "\n");

            /* Forget about peer. */
            peer_delete(event->disconnect.conn.conn_handle);

            /* Resume scanning. */
            blecent_scan();
            break;
        }
        case BLE_GAP_EVENT_DISC_COMPLETE: ESP_LOGW(TAG, "BLE_GAP_EVENT_DISC_COMPLETE"); break;
        case BLE_GAP_EVENT_ENC_CHANGE: {
            ESP_LOGW(TAG, "BLE_GAP_EVENT_ENC_CHANGE");

            int rc = peer_disc_all(event->connect.conn_handle, blecent_on_disc_complete, NULL);
            if (rc != 0) {
                ESP_LOGE(TAG, "Failed to discover services; rc=%d", rc);
                return 0;
            }
            break;
        }
        case BLE_GAP_EVENT_NOTIFY_RX: {
            // printf("BLE_GAP_EVENT_NOTIFY_RX\n");
            // ESP_LOGI(TAG, "received %s; conn_handle=%d attr_handle=%d attr_len=%d", event->notify_rx.indication ? "indication" : "notification", event->notify_rx.conn_handle, event->notify_rx.attr_handle, OS_MBUF_PKTLEN(event->notify_rx.om));

            // ESP_LOG_BUFFER_HEX(TAG, event->notify_rx.om->om_data, event->notify_rx.om->om_len);
            if (event->notify_rx.om->om_len == 16) {
                struct hid_data hd;
                memcpy(&hd, event->notify_rx.om->om_data, sizeof(hd));
                // printf("%d\n", sizeof(hd));

                // ESP_LOGI(
                //     TAG, "lx:%6d ly:%6d rx:%6d ru:%6d lt:%4u rt:%4u dpad:%u A:%u B:%u X:%u Y:%u LB:%u RB:%u VIEW:%u MENU:%u XBOX:%u L3:%u R3:%u SHARE:%u",
                //     (int)hd.lx - 0x8000, (int)hd.ly - 0x8000,
                //     (int)hd.rx - 0x8000, (int)hd.ry - 0x8000,
                //     hd.lt, hd.rt, hd.dpad,
                //     hd.btn_a, hd.btn_b, hd.btn_x, hd.btn_y, hd.btn_lb, hd.btn_rb,
                //     hd.btn_view, hd.btn_menu, hd.btn_xbox, hd.btn_l3, hd.btn_r3,
                //     hd.btn_share);

                uint8_t handctrl = 0xFF;
                if (hd.btn_a)
                    handctrl &= ~(1 << 6);
                if (hd.btn_b)
                    handctrl &= ~((1 << 7) | (1 << 2));
                if (hd.btn_x)
                    handctrl &= ~((1 << 7) | (1 << 5));
                if (hd.btn_y)
                    handctrl &= ~((1 << 5));
                if (hd.btn_lb)
                    handctrl &= ~((1 << 7) | (1 << 1));
                if (hd.btn_rb)
                    handctrl &= ~((1 << 7) | (1 << 0));

                unsigned p = 0;
                switch (hd.dpad) {
                    case 1: p = 13; break; // UP
                    case 2: p = 15; break; // UP+RIGHT
                    case 3: p = 1; break;  // RIGHT
                    case 4: p = 3; break;  // DOWN+RIGHT
                    case 5: p = 5; break;  // DOWN
                    case 6: p = 7; break;  // DOWN+LEFT
                    case 7: p = 9; break;  // LEFT
                    case 8: p = 11; break; // UP+LEFT
                    default: break;
                }

                float x     = (hd.lx - 0x8000) / 32768.0f;
                float y     = (hd.ly - 0x8000) / 32768.0f;
                float len   = sqrtf(x * x + y * y);
                float angle = 0;
                if (len > 0.4f) {
                    angle = atan2f(y, x) / M_PI * 180.0f + 180.0f;
                    p     = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
                }

                switch (p) {
                    case 1: handctrl &= ~((1 << 1)); break;
                    case 2: handctrl &= ~((1 << 4) | (1 << 1)); break;
                    case 3: handctrl &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
                    case 4: handctrl &= ~((1 << 1) | (1 << 0)); break;
                    case 5: handctrl &= ~((1 << 0)); break;
                    case 6: handctrl &= ~((1 << 4) | (1 << 0)); break;
                    case 7: handctrl &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
                    case 8: handctrl &= ~((1 << 3) | (1 << 0)); break;
                    case 9: handctrl &= ~((1 << 3)); break;
                    case 10: handctrl &= ~((1 << 4) | (1 << 3)); break;
                    case 11: handctrl &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
                    case 12: handctrl &= ~((1 << 3) | (1 << 2)); break;
                    case 13: handctrl &= ~((1 << 2)); break;
                    case 14: handctrl &= ~((1 << 4) | (1 << 2)); break;
                    case 15: handctrl &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
                    case 16: handctrl &= ~((1 << 2) | (1 << 1)); break;
                    default: break;
                }

                static uint8_t old_handctrl = 0xFF;
                if (old_handctrl != handctrl) {
                    fpga_update_handctrl(handctrl, 0xFF);
                    old_handctrl = handctrl;
                }
            }

            break;
        }
        case BLE_GAP_EVENT_MTU: {
            ESP_LOGW(TAG, "BLE_GAP_EVENT_MTU");
            break;
        }
        case BLE_GAP_EVENT_REPEAT_PAIRING: {
            ESP_LOGW(TAG, "BLE_GAP_EVENT_REPEAT_PAIRING");
            struct ble_gap_conn_desc desc;
            int                      rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            assert(rc == 0);
            ble_store_util_delete_peer(&desc.peer_id_addr);
            return BLE_GAP_REPEAT_PAIRING_RETRY;
        }

        case BLE_GAP_EVENT_CONN_UPDATE: ESP_LOGW(TAG, "BLE_GAP_EVENT_CONN_UPDATE"); break;
        case BLE_GAP_EVENT_L2CAP_UPDATE_REQ: ESP_LOGW(TAG, "BLE_GAP_EVENT_L2CAP_UPDATE_REQ"); break;

        default: {
            ESP_LOGW(TAG, "blecent_gap_event: %u", event->type);
            break;
        }
    }
    return 0;
}

static void blecent_on_reset(int reason) {
    ESP_LOGI(TAG, "blecent_on_reset - Resetting state; reason=%d", reason);
}

static void blecent_scan(void) {
    // Start BLE scan
    ESP_LOGI(TAG, "Start BLE scan");

    struct ble_gap_disc_params disc_params = {}; //.passive = 1};
    int                        rc;
    if ((rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params, blecent_gap_event, NULL)) != 0) {
        ESP_LOGE(TAG, "Error initiating GAP discovery procedure; rc=%d", rc);
    }
}

static void blecent_on_sync(void) {
    // Make sure we have proper identity address set (public preferred)
    int rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    blecent_scan();
}

static void blecent_host_task(void *param) {
    ESP_LOGI(TAG, "BLE Host Task Started");
    // This function will return only when nimble_port_stop() is executed
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void bluetooth_init(void) {
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NimBLE %d", ret);
        return;
    }

    // Configure the host
    ble_hs_cfg.reset_cb        = blecent_on_reset;
    ble_hs_cfg.sync_cb         = blecent_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Initialize data structures to track connected peers
    int rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
    assert(rc == 0);

    // Set the default device name
    rc = ble_svc_gap_device_name_set("nimble-blecent");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(blecent_host_task);
}
