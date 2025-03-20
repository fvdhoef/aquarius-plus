#pragma once

#include "Common.h"

class DebugInterface {
public:
    DebugInterface();

    void process();
    void breakpointHit();

private:
    void parseBuf(const uint8_t *p, size_t len);
    void receivedCmd(const std::string &cmd);
    void sendResponse(const std::string &resp);
    void sendStopReply(uint8_t signal = 5);

    std::string readMemory(uintptr_t addr, size_t size);
    void        writeMemory(uintptr_t addr, std::vector<uint8_t> data);

    int listenSocket = -1;
    int conn         = -1;

    int         rxState = 0;
    std::string rxStr;
    char        rxChecksum[3];
};
