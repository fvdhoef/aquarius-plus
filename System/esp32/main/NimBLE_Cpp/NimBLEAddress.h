/*
 * NimBLEAddress.h
 *
 *  Created: on Jan 24 2020
 *      Author H2zero
 *
 * Originally:
 *
 * BLEAddress.h
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#pragma once

#include "nimconfig.h"
#include "nimble/ble.h"
#include <string>
#include <algorithm>

/**
 * @brief A %BLE device address.
 *
 * Every %BLE device has a unique address which can be used to identify it and form connections.
 */
class NimBLEAddress {
public:
    NimBLEAddress()
        : NimBLEAddress("") {
    }
    NimBLEAddress(ble_addr_t address) {
        memcpy(m_address, address.val, 6);
        m_addrType = address.type;
    }
    NimBLEAddress(uint8_t address[6], uint8_t type = BLE_ADDR_PUBLIC) {
        std::reverse_copy(address, address + sizeof(m_address), m_address);
        m_addrType = type;
    }
    NimBLEAddress(const std::string &stringAddress, uint8_t type = BLE_ADDR_PUBLIC);
    NimBLEAddress(const uint64_t &address, uint8_t type = BLE_ADDR_PUBLIC) {
        memcpy(m_address, &address, sizeof(m_address));
        m_addrType = type;
    }
    bool equals(const NimBLEAddress &otherAddress) const {
        return *this == otherAddress;
    }

    const uint8_t *getNative() const {
        return m_address;
    }
    std::string toString() const {
        return std::string(*this);
    }
    uint8_t getType() const {
        return m_addrType;
    }

    bool operator==(const NimBLEAddress &rhs) const {
        return memcmp(rhs.m_address, m_address, sizeof(m_address)) == 0;
    }
    bool operator!=(const NimBLEAddress &rhs) const {
        return !this->operator==(rhs);
    }
    operator std::string() const;
    operator uint64_t() const {
        uint64_t address = 0;
        memcpy(&address, m_address, sizeof(m_address));
        return address;
    }

private:
    uint8_t m_address[6];
    uint8_t m_addrType;
};
