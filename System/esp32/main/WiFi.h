#pragma once

#include "Common.h"

struct WiFiApInfo {
    std::string ssid;
    int8_t      rssi     = -128;
    char        authMode = ' ';

    void clear() {
        ssid.clear();
        rssi     = -128;
        authMode = ' ';
    }
};

struct NetworkInfo {
    bool       connected;
    WiFiApInfo currentNetwork;

    uint8_t mac[6];

    std::string ip;
    std::string netmask;
    std::string gateway;
    std::string dns1;
    std::string dns2;
    std::string dns3;

    std::vector<WiFiApInfo> knownNetworks;
    std::vector<WiFiApInfo> otherNetworks;
};

class WiFi {
public:
    virtual void init() = 0;

    virtual bool getEnabled()            = 0;
    virtual void setEnabled(bool enable) = 0;

    virtual std::string getHostName()                        = 0;
    virtual void        setHostName(const std::string &name) = 0;

    virtual void        updateInfo()     = 0;
    virtual NetworkInfo getNetworkInfo() = 0;

    virtual void joinNetwork(const std::string &ssid, const std::string &password) = 0;
    virtual void forgetNetwork(const std::string &ssid)                            = 0;
};

WiFi *getWiFi();
