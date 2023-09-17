/*
 * NimBLEUUID.h
 *
 *  Created: on Jan 24 2020
 *      Author H2zero
 *
 * Originally:
 *
 * BLEUUID.h
 *
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */

#pragma once

#include "nimconfig.h"
#include "host/ble_uuid.h"
#include <string>

/**
 * @brief A model of a %BLE UUID.
 */
class NimBLEUUID {
public:
    NimBLEUUID(const std::string &uuid);
    NimBLEUUID(uint16_t uuid);
    NimBLEUUID(uint32_t uuid);
    NimBLEUUID(const ble_uuid128_t *uuid);
    NimBLEUUID(const uint8_t *pData, size_t size, bool msbFirst);
    NimBLEUUID(uint32_t first, uint16_t second, uint16_t third, uint64_t fourth);
    NimBLEUUID() {
        m_valueSet = false;
    }

    uint8_t bitSize() const {
        if (!m_valueSet)
            return 0;
        return m_uuid.u.type;
    }
    bool equals(const NimBLEUUID &uuid) const {
        return *this == uuid;
    }
    const ble_uuid_any_t *getNative() const;
    const NimBLEUUID     &to128();
    const NimBLEUUID     &to16();
    std::string           toString() const {
        return std::string(*this);
    }
    static NimBLEUUID fromString(const std::string &uuid);

    bool operator==(const NimBLEUUID &rhs) const;
    bool operator!=(const NimBLEUUID &rhs) const {
        return !this->operator==(rhs);
    }
    operator std::string() const {
        if (!m_valueSet)
            return std::string(); // If we have no value, nothing to format.

        char buf[BLE_UUID_STR_LEN];
        return ble_uuid_to_str(&m_uuid.u, buf);
    }

private:
    ble_uuid_any_t m_uuid;
    bool           m_valueSet = false;
};
