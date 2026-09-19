#pragma once

#include "Common.h"

struct BleAddr { // This needs to be identical to ble_addr_t
    uint8_t type;
    uint8_t val[6];
};

struct BtDevInfo {
    BleAddr     addr;
    std::string name;
    uint16_t    appearance = 0;
    int8_t      rssi       = -128;
};

struct BluetoothInfo {
    std::vector<BtDevInfo> connectedDevices;
    std::vector<BtDevInfo> knownDevices;
    std::vector<BtDevInfo> otherDevices;
};

class Bluetooth {
public:
    virtual void init() = 0;

    virtual bool getEnabled()            = 0;
    virtual void setEnabled(bool enable) = 0;

    virtual bool addDevice(const BleAddr &addr, const std::string &name) = 0;
    virtual void forgetDevice(const BleAddr &addr)                       = 0;

    virtual BluetoothInfo getBluetoothInfo() = 0;
};

Bluetooth *getBluetooth();
