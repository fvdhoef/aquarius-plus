#pragma once

#include "NimBLEAddress.h"

/**
 * @brief Connection information.
 */
class NimBLEConnInfo {
    friend class NimBLEServer;
    friend class NimBLEClient;
    ble_gap_conn_desc m_desc;
    NimBLEConnInfo() {
        m_desc = {};
    }
    NimBLEConnInfo(ble_gap_conn_desc desc) {
        m_desc = desc;
    }

public:
    // clang-format off
    NimBLEAddress getAddress()      { return NimBLEAddress(m_desc.peer_ota_addr); }
    NimBLEAddress getIdAddress()    { return NimBLEAddress(m_desc.peer_id_addr); }
    uint16_t      getConnHandle()   { return m_desc.conn_handle; }
    uint16_t      getConnInterval() { return m_desc.conn_itvl; }
    uint16_t      getConnTimeout()  { return m_desc.supervision_timeout; }
    uint16_t      getConnLatency()  { return m_desc.conn_latency; }
    uint16_t      getMTU()          { return ble_att_mtu(m_desc.conn_handle); }
    bool          isMaster()        { return (m_desc.role == BLE_GAP_ROLE_MASTER); }
    bool          isSlave()         { return (m_desc.role == BLE_GAP_ROLE_SLAVE); }
    bool          isBonded()        { return (m_desc.sec_state.bonded == 1); }
    bool          isEncrypted()     { return (m_desc.sec_state.encrypted == 1); }
    bool          isAuthenticated() { return (m_desc.sec_state.authenticated == 1); }
    uint8_t       getSecKeySize()   { return m_desc.sec_state.key_size; }
    // clang-format on
};
