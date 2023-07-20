#include "ble_util.h"

void print_uuid2(const ble_uuid_t *uuid) {
    char buf[BLE_UUID_STR_LEN];

    printf("%s", ble_uuid_to_str(uuid, buf));
}

void print_bytes2(const uint8_t *bytes, int len) {
    int i;

    for (i = 0; i < len; i++) {
        printf("%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

void print_adv_fields2(const struct ble_hs_adv_fields *fields) {
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

void print_peer_svcs(const struct peer *peer) {
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
