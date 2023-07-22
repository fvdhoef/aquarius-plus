#include "ble.h"
#include "ble_util.h"
#include "hid.h"
#include <host/ble_hs_pvcy.h>

#if 0
static const char *TAG = "ble";

static void blecent_scan(void);
static int  blecent_gap_event(struct ble_gap_event *event, void *arg);

// static int blecent_on_subscribe2(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
//     ESP_LOGI(TAG, "Subscribe complete; status=%d conn_handle=%d attr_handle=%d", error->status, conn_handle, attr->handle);
//     return 0;
// }

// static int blecent_on_subscribe1(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
//     ESP_LOGI(TAG, "Subscribe complete; status=%d conn_handle=%d attr_handle=%d", error->status, conn_handle, attr->handle);

//     struct peer *peer;
//     peer = peer_find(conn_handle);
//     if (peer == NULL) {
//         ESP_LOGE(TAG, "Error in finding peer, aborting...");
//         ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
//         return 0;
//     }

//     const struct peer_dsc *dsc;
//     dsc = peer_dsc_find_uuid(peer, BLE_UUID16_DECLARE(0x180f), BLE_UUID16_DECLARE(0x2a19), BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
//     if (dsc == NULL) {
//         ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
//         return 0;
//     }

//     uint16_t value = 1;
//     int      rc    = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, &value, sizeof(value), blecent_on_subscribe2, NULL);
//     if (rc != 0) {
//         ESP_LOGE(TAG, "Failed to subscribe to characteristic; rc=%d", rc);
//         ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
//         return 0;
//     }

//     return 0;
// }

static int blecent_on_subscribe(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
    ESP_LOGI(TAG, "Subscribe complete; status=%d conn_handle=%d attr_handle=%d", error->status, conn_handle, attr->handle);
    return 0;
}

static int blecent_subscribe(const struct peer *peer, const ble_uuid_t *svc_uuid, const ble_uuid_t *chr_uuid) {
    // Find HID report descriptor
    const struct peer_dsc *dsc = peer_dsc_find_uuid(peer, svc_uuid, chr_uuid, BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if (dsc == NULL) {
        ESP_LOGE(TAG, "Failed to find characteristic");
        return -1;
    }

    // Enable notifications
    uint16_t value = 1;
    int      rc    = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, &value, sizeof(value), blecent_on_subscribe, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to subscribe to characteristic; rc=%d", rc);
        return -1;
    }
    return 0;
}

static int blecent_on_read(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
    ESP_LOGI(TAG, "Read complete; status=%d conn_handle=%d", error->status, conn_handle);
    if (error->status != 0) {
        goto fail;
    }

    struct peer *peer;
    if ((peer = peer_find(conn_handle)) == NULL) {
        ESP_LOGE(TAG, "Error in finding peer, aborting...");
        goto fail;
    }

    if (blecent_subscribe(peer, BLE_UUID16_DECLARE(0x1812), BLE_UUID16_DECLARE(0x2a4d)) != 0) {
        goto fail;
    }
    return 0;

fail:
    // Failed, disconnect device.
    return ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
}

static void blecent_on_disc_complete(const struct peer *peer, int status, void *arg) {
    if (status != 0) {
        // Service discovery failed. Terminate the connection.
        ESP_LOGE(TAG, "Service discovery failed; status=%d conn_handle=%d", status, peer->conn_handle);
        goto fail;
    }

    // Service discovery has completed successfully. Now we have a complete list of services, characteristics, and descriptors that the peer supports.
    ESP_LOGI(TAG, "Service discovery complete; status=%d conn_handle=%d", status, peer->conn_handle);

    // Print discovered services
    print_peer_svcs(peer);

    // Subscribe
    const struct peer_chr *chr = peer_chr_find_uuid(peer, BLE_UUID16_DECLARE(0x1812), BLE_UUID16_DECLARE(0x2a4d));
    if (chr == NULL) {
        goto fail;
    }

    if (ble_gattc_read(peer->conn_handle, chr->chr.val_handle, blecent_on_read, NULL) != 0) {
        goto fail;
    }

    // if (blecent_subscribe(peer, BLE_UUID16_DECLARE(0x1812), BLE_UUID16_DECLARE(0x2a4d)) != 0) {
    //     goto fail;
    // }
    return;

fail:
    // Failed, disconnect device.
    ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
}

static void blecent_scan(void) {
    // Start BLE scan
    ESP_LOGI(TAG, "Start BLE scan");

    struct ble_gap_disc_params disc_params = {.passive = 1};
    int                        rc;
    if ((rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params, blecent_gap_event, NULL)) != 0) {
        ESP_LOGE(TAG, "Error initiating GAP discovery procedure; rc=%d", rc);
    }
}

static int blecent_should_connect(const struct ble_gap_disc_desc *disc) {
    struct ble_hs_adv_fields fields;

    int rc = ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);
    if (rc != 0) {
        return 0;
    }

    if (fields.appearance_is_present && fields.appearance == 0x03C4) {
        // Connect if this is a game pad
        return 1;
    }

    return 0;
}

static void blecent_connect_if_interesting(const struct ble_gap_disc_desc *disc) {
    // Don't do anything if we don't care about this advertiser
    if (!blecent_should_connect(disc)) {
        return;
    }

    // Scanning must be stopped before a connection can be initiated.
    ble_gap_disc_cancel();

    // Figure out address to use for connect (no privacy for now)
    uint8_t own_addr_type;
    ble_hs_id_infer_auto(0, &own_addr_type);

    // Try to connect the the advertiser
    ESP_LOGI(TAG, "Connecting");
    ble_gap_connect(own_addr_type, &disc->addr, 30000, NULL, blecent_gap_event, NULL);
}

static int blecent_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_DISC: {
#    if 0
            ESP_LOGW(TAG, "BLE_GAP_EVENT_DISC");
            struct ble_hs_adv_fields fields;

            int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
            if (rc != 0) {
                return 0;
            }

            /* An advertisement report was received during GAP discovery. */
            print_adv_fields2(&fields);
#    endif
            blecent_connect_if_interesting(&event->disc);
            return 0;
        }

        case BLE_GAP_EVENT_CONNECT: {
            int rc;

            if (event->connect.status != 0) {
                // Connection attempt failed, resume scanning
                ESP_LOGE(TAG, "Connection failed");
                blecent_scan();
                return 0;
            }

            ESP_LOGI(TAG, "Connection established");

            // struct ble_gap_conn_desc desc;
            // int                      rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            // assert(rc == 0);
            // print_conn_desc(&desc);

            // Remember peer
            rc = peer_add(event->connect.conn_handle);
            if (rc != 0) {
                ESP_LOGE(TAG, "Failed to add peer; rc=%d", rc);
                return 0;
            }

            // Initiate security - It will perform:
            // - Pairing (Exchange keys)
            // - Bonding (Store keys)
            // - Encryption (Enable encryption)
            // - Will invoke event BLE_GAP_EVENT_ENC_CHANGE
            rc = ble_gap_security_initiate(event->connect.conn_handle);
            if (rc != 0) {
                ESP_LOGI(TAG, "Security could not be initiated, rc = %d", rc);
                return ble_gap_terminate(event->connect.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
            } else {
                ESP_LOGI(TAG, "Connection secured");
            }
            return 0;
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
                handle_xbox_data(event->notify_rx.om->om_data);
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
#endif

void ble_init(void) {
#if 0
    int       rc;
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NimBLE %d", ret);
        return;
    }

    // Configure the host
    ble_hs_cfg.reset_cb        = blecent_on_reset;
    ble_hs_cfg.sync_cb         = blecent_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Set initial security capabilities
    ble_hs_cfg.sm_io_cap         = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding        = 0;
    ble_hs_cfg.sm_mitm           = 0;
    ble_hs_cfg.sm_sc             = 1;
    ble_hs_cfg.sm_our_key_dist   = 1;
    ble_hs_cfg.sm_their_key_dist = 3;

    // Initialize data structures to track connected peers
    rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(blecent_host_task);

    // {
    //     // ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    //     rc = ble_hs_pvcy_rpa_config(NIMBLE_HOST_ENABLE_RPA);
    //     if (rc != 0) {
    //         ESP_LOGE(TAG, "ble_hs_pvcy_rpa_config rc=%d", rc);
    //     }
    // }
#endif
}
