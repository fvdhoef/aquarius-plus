/*
 * NimBLEAdvertisedDevice.cpp
 *
 *  Created: on Jan 24 2020
 *      Author H2zero
 *
 * Originally:
 *
 *  BLEAdvertisedDevice.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: kolban
 */

#include "NimBLEAdvertisedDevice.h"
#include "NimBLEDevice.h"
#include "NimBLEUtils.h"
#include <climits>

static const char *LOG_TAG = "NimBLEAdvertisedDevice";

/**
 * @brief Get the advertisement flags.
 * @return The advertisement flags, a bitmask of:
 * BLE_HS_ADV_F_DISC_LTD (0x01) - limited discoverability
 * BLE_HS_ADV_F_DISC_GEN (0x02) - general discoverability
 * BLE_HS_ADV_F_BREDR_UNSUP - BR/EDR not supported
 */
uint8_t NimBLEAdvertisedDevice::getAdvFlags() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_FLAGS, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length == BLE_HS_ADV_FLAGS_LEN + 1) {
            return *field->value;
        }
    }
    return 0;
}

/**
 * @brief Get the appearance.
 *
 * A %BLE device can declare its own appearance.  The appearance is how it would like to be shown to an end user
 * typically in the form of an icon.
 *
 * @return The appearance of the advertised device.
 */
uint16_t NimBLEAdvertisedDevice::getAppearance() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_APPEARANCE, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length == BLE_HS_ADV_APPEARANCE_LEN + 1) {
            return *field->value | *(field->value + 1) << 8;
        }
    }

    return 0;
}

/**
 * @brief Get the advertisement interval.
 * @return The advertisement interval in 0.625ms units.
 */
uint16_t NimBLEAdvertisedDevice::getAdvInterval() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_ADV_ITVL, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length == BLE_HS_ADV_ADV_ITVL_LEN + 1) {
            return *field->value | *(field->value + 1) << 8;
        }
    }

    return 0;
}

/**
 * @brief Get the preferred min connection interval.
 * @return The preferred min connection interval in 1.25ms units.
 */
uint16_t NimBLEAdvertisedDevice::getMinInterval() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_SLAVE_ITVL_RANGE, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length == BLE_HS_ADV_SLAVE_ITVL_RANGE_LEN + 1) {
            return *field->value | *(field->value + 1) << 8;
        }
    }

    return 0;
}

/**
 * @brief Get the preferred max connection interval.
 * @return The preferred max connection interval in 1.25ms units.
 */
uint16_t NimBLEAdvertisedDevice::getMaxInterval() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_SLAVE_ITVL_RANGE, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length == BLE_HS_ADV_SLAVE_ITVL_RANGE_LEN + 1) {
            return *(field->value + 2) | *(field->value + 3) << 8;
        }
    }

    return 0;
}

/**
 * @brief Get the manufacturer data.
 * @param [in] index The index of the of the manufacturer data set to get.
 * @return The manufacturer data.
 */
std::string NimBLEAdvertisedDevice::getManufacturerData(uint8_t index) {
    size_t data_loc = 0;
    index++;

    if (findAdvField(BLE_HS_ADV_TYPE_MFG_DATA, index, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length > 1) {
            return std::string((char *)field->value, field->length - 1);
        }
    }

    return "";
}

/**
 * @brief Get the count of manufacturer data sets.
 * @return The number of manufacturer data sets.
 */
uint8_t NimBLEAdvertisedDevice::getManufacturerDataCount() {
    return findAdvField(BLE_HS_ADV_TYPE_MFG_DATA);
}

/**
 * @brief Get the URI from the advertisement.
 * @return The URI data.
 */
std::string NimBLEAdvertisedDevice::getURI() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_URI, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length > 1) {
            return std::string((char *)field->value, field->length - 1);
        }
    }

    return "";
}

/**
 * @brief Get the advertised name.
 * @return The name of the advertised device.
 */
std::string NimBLEAdvertisedDevice::getName() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_COMP_NAME, 0, &data_loc) > 0 ||
        findAdvField(BLE_HS_ADV_TYPE_INCOMP_NAME, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length > 1) {
            return std::string((char *)field->value, field->length - 1);
        }
    }

    return "";
}

/**
 * @brief Get the number of target addresses.
 * @return The number of addresses.
 */
uint8_t NimBLEAdvertisedDevice::getTargetAddressCount() {
    uint8_t count = 0;

    count = findAdvField(BLE_HS_ADV_TYPE_PUBLIC_TGT_ADDR);
    count += findAdvField(BLE_HS_ADV_TYPE_RANDOM_TGT_ADDR);

    return count;
}

/**
 * @brief Get the target address at the index.
 * @param [in] index The index of the target address.
 * @return The target address.
 */
NimBLEAddress NimBLEAdvertisedDevice::getTargetAddress(uint8_t index) {
    ble_hs_adv_field *field    = nullptr;
    uint8_t           count    = 0;
    size_t            data_loc = ULONG_MAX;

    index++;
    count = findAdvField(BLE_HS_ADV_TYPE_PUBLIC_TGT_ADDR, index, &data_loc);

    if (count < index) {
        index -= count;
        count = findAdvField(BLE_HS_ADV_TYPE_RANDOM_TGT_ADDR, index, &data_loc);
    }

    if (count > 0 && data_loc != ULONG_MAX) {
        field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length < index * BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN) {
            index -= count - field->length / BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN;
        }
        if (field->length > index * BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN) {
            return NimBLEAddress(field->value + (index - 1) * BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN);
        }
    }

    return NimBLEAddress("");
}

/**
 * @brief Get the service data.
 * @param [in] index The index of the service data requested.
 * @return The advertised service data or empty string if no data.
 */
std::string NimBLEAdvertisedDevice::getServiceData(uint8_t index) {
    ble_hs_adv_field *field = nullptr;
    uint8_t           bytes;
    size_t            data_loc = findServiceData(index, &bytes);

    if (data_loc != ULONG_MAX) {
        field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length > bytes) {
            return std::string((char *)(field->value + bytes), field->length - bytes - 1);
        }
    }

    return "";
}

/**
 * @brief Get the service data.
 * @param [in] uuid The uuid of the service data requested.
 * @return The advertised service data or empty string if no data.
 */
std::string NimBLEAdvertisedDevice::getServiceData(const NimBLEUUID &uuid) {
    ble_hs_adv_field *field = nullptr;
    uint8_t           bytes;
    uint8_t           index     = 0;
    size_t            data_loc  = findServiceData(index, &bytes);
    size_t            plSize    = m_payload.size() - 2;
    uint8_t           uuidBytes = uuid.bitSize() / 8;

    while (data_loc < plSize) {
        field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (bytes == uuidBytes && NimBLEUUID(field->value, bytes, false) == uuid) {
            return std::string((char *)(field->value + bytes), field->length - bytes - 1);
        }

        index++;
        data_loc = findServiceData(index, &bytes);
    }

    NIMBLE_LOGI(LOG_TAG, "No service data found");
    return "";
}

/**
 * @brief Get the UUID of the service data at the index.
 * @param [in] index The index of the service data UUID requested.
 * @return The advertised service data UUID or an empty UUID if not found.
 */
NimBLEUUID NimBLEAdvertisedDevice::getServiceDataUUID(uint8_t index) {
    ble_hs_adv_field *field = nullptr;
    uint8_t           bytes;
    size_t            data_loc = findServiceData(index, &bytes);

    if (data_loc != ULONG_MAX) {
        field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length >= bytes) {
            return NimBLEUUID(field->value, bytes, false);
        }
    }

    return NimBLEUUID("");
}

/**
 * @brief Find the service data at the index.
 * @param [in] index The index of the service data to find.
 * @param [in] bytes A pointer to storage for the number of the bytes in the UUID.
 * @return The index in the vector where the data is located, ULONG_MAX if not found.
 */
size_t NimBLEAdvertisedDevice::findServiceData(uint8_t index, uint8_t *bytes) {
    size_t  data_loc = 0;
    uint8_t found    = 0;

    *bytes = 0;
    index++;
    found = findAdvField(BLE_HS_ADV_TYPE_SVC_DATA_UUID16, index, &data_loc);
    if (found == index) {
        *bytes = 2;
        return data_loc;
    }

    index -= found;
    found = findAdvField(BLE_HS_ADV_TYPE_SVC_DATA_UUID32, index, &data_loc);
    if (found == index) {
        *bytes = 4;
        return data_loc;
    }

    index -= found;
    found = findAdvField(BLE_HS_ADV_TYPE_SVC_DATA_UUID128, index, &data_loc);
    if (found == index) {
        *bytes = 16;
        return data_loc;
    }

    return ULONG_MAX;
}

/**
 * @brief Get the count of advertised service data UUIDS
 * @return The number of service data UUIDS in the vector.
 */
uint8_t NimBLEAdvertisedDevice::getServiceDataCount() {
    uint8_t count = 0;

    count += findAdvField(BLE_HS_ADV_TYPE_SVC_DATA_UUID16);
    count += findAdvField(BLE_HS_ADV_TYPE_SVC_DATA_UUID32);
    count += findAdvField(BLE_HS_ADV_TYPE_SVC_DATA_UUID128);

    return count;
}

/**
 * @brief Get the Service UUID.
 * @param [in] index The index of the service UUID requested.
 * @return The Service UUID of the advertised service, or an empty UUID if not found.
 */
NimBLEUUID NimBLEAdvertisedDevice::getServiceUUID(uint8_t index) {
    uint8_t           count     = 0;
    size_t            data_loc  = 0;
    uint8_t           uuidBytes = 0;
    uint8_t           type      = BLE_HS_ADV_TYPE_INCOMP_UUIDS16;
    ble_hs_adv_field *field     = nullptr;

    index++;

    do {
        count = findAdvField(type, index, &data_loc);
        if (count >= index) {
            if (type < BLE_HS_ADV_TYPE_INCOMP_UUIDS32) {
                uuidBytes = 2;
            } else if (type < BLE_HS_ADV_TYPE_INCOMP_UUIDS128) {
                uuidBytes = 4;
            } else {
                uuidBytes = 16;
            }
            break;

        } else {
            type++;
            index -= count;
        }

    } while (type <= BLE_HS_ADV_TYPE_COMP_UUIDS128);

    if (uuidBytes > 0) {
        field = (ble_hs_adv_field *)&m_payload[data_loc];
        // In the case of more than one field of service uuid's we need to adjust
        // the index to account for the uuids of the previous fields.
        if (field->length < index * uuidBytes) {
            index -= count - field->length / uuidBytes;
        }

        if (field->length > uuidBytes * index) {
            return NimBLEUUID(field->value + uuidBytes * (index - 1), uuidBytes, false);
        }
    }

    return NimBLEUUID("");
}

/**
 * @brief Get the number of services advertised
 * @return The count of services in the advertising packet.
 */
uint8_t NimBLEAdvertisedDevice::getServiceUUIDCount() {
    uint8_t count = 0;

    count += findAdvField(BLE_HS_ADV_TYPE_INCOMP_UUIDS16);
    count += findAdvField(BLE_HS_ADV_TYPE_COMP_UUIDS16);
    count += findAdvField(BLE_HS_ADV_TYPE_INCOMP_UUIDS32);
    count += findAdvField(BLE_HS_ADV_TYPE_COMP_UUIDS32);
    count += findAdvField(BLE_HS_ADV_TYPE_INCOMP_UUIDS128);
    count += findAdvField(BLE_HS_ADV_TYPE_COMP_UUIDS128);

    return count;
}

/**
 * @brief Check advertised services for existence of the required UUID
 * @param [in] uuid The service uuid to look for in the advertisement.
 * @return Return true if service is advertised
 */
bool NimBLEAdvertisedDevice::isAdvertisingService(const NimBLEUUID &uuid) {
    size_t count = getServiceUUIDCount();
    for (size_t i = 0; i < count; i++) {
        if (uuid == getServiceUUID(i)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Get the TX Power.
 * @return The TX Power of the advertised device.
 */
int8_t NimBLEAdvertisedDevice::getTXPower() {
    size_t data_loc = 0;

    if (findAdvField(BLE_HS_ADV_TYPE_TX_PWR_LVL, 0, &data_loc) > 0) {
        ble_hs_adv_field *field = (ble_hs_adv_field *)&m_payload[data_loc];
        if (field->length == BLE_HS_ADV_TX_PWR_LVL_LEN + 1) {
            return *(int8_t *)field->value;
        }
    }

    return -99;
}

uint8_t NimBLEAdvertisedDevice::findAdvField(uint8_t type, uint8_t index, size_t *data_loc) {
    ble_hs_adv_field *field  = nullptr;
    size_t            length = m_payload.size();
    size_t            data   = 0;
    uint8_t           count  = 0;

    if (length < 3) {
        return count;
    }

    while (length > 2) {
        field = (ble_hs_adv_field *)&m_payload[data];

        if (field->length >= length) {
            return count;
        }

        if (field->type == type) {
            switch (type) {
                case BLE_HS_ADV_TYPE_INCOMP_UUIDS16:
                case BLE_HS_ADV_TYPE_COMP_UUIDS16:
                    count += field->length / 2;
                    break;

                case BLE_HS_ADV_TYPE_INCOMP_UUIDS32:
                case BLE_HS_ADV_TYPE_COMP_UUIDS32:
                    count += field->length / 4;
                    break;

                case BLE_HS_ADV_TYPE_INCOMP_UUIDS128:
                case BLE_HS_ADV_TYPE_COMP_UUIDS128:
                    count += field->length / 16;
                    break;

                case BLE_HS_ADV_TYPE_PUBLIC_TGT_ADDR:
                case BLE_HS_ADV_TYPE_RANDOM_TGT_ADDR:
                    count += field->length / 6;
                    break;

                default:
                    count++;
                    break;
            }

            if (data_loc != nullptr) {
                if (index == 0 || count >= index) {
                    break;
                }
            }
        }

        length -= 1 + field->length;
        data += 1 + field->length;
    }

    if (data_loc != nullptr && field != nullptr) {
        *data_loc = data;
    }

    return count;
}

NimBLEScan *NimBLEAdvertisedDevice::getScan() {
    return NimBLEDevice::getScan();
}

/**
 * @brief Create a string representation of this device.
 * @return A string representation of this device.
 */
std::string NimBLEAdvertisedDevice::toString() {
    std::string res = "Name: " + getName() + ", Address: " + getAddress().toString();

    if (haveAppearance()) {
        char val[6];
        snprintf(val, sizeof(val), "%d", getAppearance());
        res += ", appearance: ";
        res += val;
    }

    if (haveManufacturerData()) {
        char *pHex = NimBLEUtils::buildHexData(nullptr, (uint8_t *)getManufacturerData().data(), getManufacturerData().length());
        res += ", manufacturer data: ";
        res += pHex;
        free(pHex);
    }

    if (haveServiceUUID()) {
        res += ", serviceUUID: " + getServiceUUID().toString();
    }

    if (haveTXPower()) {
        char val[5];
        snprintf(val, sizeof(val), "%d", getTXPower());
        res += ", txPower: ";
        res += val;
    }

    if (haveServiceData()) {
        uint8_t count = getServiceDataCount();
        res += "\nService Data:";
        for (uint8_t i = 0; i < count; i++) {
            res += "\nUUID: " + std::string(getServiceDataUUID(i));
            res += ", Data: " + getServiceData(i);
        }
    }

    return res;
}

/**
 * @brief Stores the payload of the advertised device in a vector.
 * @param [in] payload The advertisement payload.
 * @param [in] length The length of the payload in bytes.
 * @param [in] append Indicates if the the data should be appended (scan response).
 */
void NimBLEAdvertisedDevice::setPayload(const uint8_t *payload, uint8_t length, bool append) {
    if (!append) {
        m_advLength = length;
        m_payload.assign(payload, payload + length);
    } else {
        m_payload.insert(m_payload.end(), payload, payload + length);
    }
}
