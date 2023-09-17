/*
 * NimBLEAddress.cpp
 *
 *  Created: on Jan 24 2020
 *      Author H2zero
 *
 * Originally:
 *
 * BLEAddress.cpp
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#include "NimBLEAddress.h"
#include "NimBLEUtils.h"

/*************************************************
 * NOTE: NimBLE address bytes are in INVERSE ORDER!
 * We will accomodate that fact in these methods.
 *************************************************/

/**
 * @brief Create an address from a hex string
 *
 * A hex string is of the format:
 * ```
 * 00:00:00:00:00:00
 * ```
 * which is 17 characters in length.
 *
 * @param [in] stringAddress The hex string representation of the address.
 * @param [in] type The type of the address.
 */
NimBLEAddress::NimBLEAddress(const std::string &stringAddress, uint8_t type) {
    m_addrType = type;

    if (stringAddress.length() == 0) {
        memset(m_address, 0, 6);
        return;
    }

    if (stringAddress.length() == 6) {
        std::reverse_copy(stringAddress.data(), stringAddress.data() + 6, m_address);
        return;
    }

    if (stringAddress.length() != 17) {
        memset(m_address, 0, sizeof(m_address)); // "00:00:00:00:00:00" represents an invalid address
        return;
    }

    int data[6];
    if (sscanf(stringAddress.c_str(), "%x:%x:%x:%x:%x:%x", &data[5], &data[4], &data[3], &data[2], &data[1], &data[0]) != 6) {
        memset(m_address, 0, sizeof(m_address)); // "00:00:00:00:00:00" represents an invalid address
    }
    for (size_t index = 0; index < sizeof(m_address); index++) {
        m_address[index] = data[index];
    }
}

/**
 * @brief Convenience operator to convert this address to string representation.
 * @details This allows passing NimBLEAddress to functions
 * that accept std::string and/or or it's methods as a parameter.
 */
NimBLEAddress::operator std::string() const {
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x", m_address[5], m_address[4], m_address[3], m_address[2], m_address[1], m_address[0]);
    return std::string(buffer);
}
