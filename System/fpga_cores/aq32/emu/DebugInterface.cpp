#include "DebugInterface.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "EmuState.h"
#include <sys/time.h>

#if 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

static std::string bufToHex(const void *buf, unsigned size) {
    std::string result;

    auto p = reinterpret_cast<const uint8_t *>(buf);
    while (size--)
        result += fmtstr("%02X", *(p++));

    return result;
}

static int64_t getTimeUs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (int64_t)tv.tv_sec * 1000000LL + tv.tv_usec;
}

DebugInterface::DebugInterface() {
}

void DebugInterface::process() {
    bool gotCommand = false;
    auto tStart     = getTimeUs();

    while (1) {
        if (listenSocket < 0) {
            listenSocket = socket(AF_INET, SOCK_STREAM, 0);

            const int enable = 1;
            setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

            fcntl(listenSocket, F_SETFL, fcntl(listenSocket, F_GETFL, 0) | O_NONBLOCK);

            sockaddr_in serverAddress;
            serverAddress.sin_family      = AF_INET;
            serverAddress.sin_port        = htons(2331);
            serverAddress.sin_addr.s_addr = INADDR_ANY;
            bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
            listen(listenSocket, 5);
        }

        if (conn < 0) {
            conn = accept(listenSocket, nullptr, nullptr);
            if (conn < 0)
                return;

            fcntl(conn, F_SETFL, fcntl(conn, F_GETFL, 0) | O_NONBLOCK);
            dprintf("Got connection!\n");

            emuState.reset();
            emuState.emuMode = EmuState::Em_Halted;
            emuState.breakpoints.clear();
        }

        if (conn >= 0) {
            uint8_t buf[512];
            int     sz = recv(conn, buf, sizeof(buf), 0);
            if (sz <= 0) {
                if (sz == -1 && errno == EAGAIN) {
                    // No data received
                    if (emuState.emuMode != EmuState::Em_Running) {
                        if (gotCommand && (getTimeUs() - tStart) < 1000000)
                            continue;
                    }
                    return;
                }

                close(conn);
                conn = -1;
                dprintf("Connection closed.\n");
                exit(0);
                return;
            }

            gotCommand = true;
            parseBuf(buf, sz);
        }
    }
}

void DebugInterface::parseBuf(const uint8_t *p, size_t len) {
    while (len--) {
        uint8_t ch = *(p++);

        if (ch == 3) {
            // CTRL-C
            dprintf("Halting\n");
            emuState.emuMode = EmuState::Em_Halted;

            send(conn, "+", 1, 0);
            // last signal
            sendStopReply();

        } else if (ch == '$') {
            rxState = 1;
            rxStr.clear();
        } else if (rxState == 1) {
            if (ch == '#') {
                rxState = 2;
            } else {
                rxStr += ch;
            }
        } else if (rxState == 2) {
            rxChecksum[0] = ch;
            rxState       = 3;
        } else if (rxState == 3) {
            rxChecksum[1] = ch;
            rxChecksum[2] = 0;

            uint8_t checksum = strtoul(rxChecksum, nullptr, 16);

            uint8_t checksum2 = 0;
            for (uint8_t ch : rxStr) {
                checksum2 += ch;
            }
            if (checksum2 == checksum) {
                dprintf("> '%s'\n", rxStr.c_str());
                // dprintf("< +\n");
                send(conn, "+", 1, 0);
                receivedCmd(rxStr);
            } else {
                dprintf("< -\n");
                send(conn, "-", 1, 0);
            }

            rxState = 0;
            rxStr.clear();
        }
    }
}

void DebugInterface::breakpointHit() {
    if (conn >= 0)
        sendStopReply();
}

std::string DebugInterface::readMemory(uintptr_t addr, size_t size) {
    dprintf("Read memory %08lX (size:%08lX)\n", addr, size);

    std::vector<uint8_t> respData;

    while (size) {
        auto paddr = emuState.cpu.tlb_lookup(addr);
        if (paddr < 0)
            break;

        auto data = emuState.memRead(paddr);
        if (data < 0)
            break;

        if ((addr & 3) == 0) {
            respData.push_back((data >> 0) & 0xFF);
            if (size >= 2)
                respData.push_back((data >> 8) & 0xFF);
            if (size >= 3)
                respData.push_back((data >> 16) & 0xFF);
            if (size >= 4)
                respData.push_back((data >> 24) & 0xFF);

            if (size <= 4)
                break;

            size -= 4;
            addr += 4;

        } else if ((addr & 3) == 1) {
            respData.push_back((data >> 8) & 0xFF);
            size--;
            addr++;
        } else if ((addr & 3) == 2) {
            respData.push_back((data >> 16) & 0xFF);
            size--;
            addr++;
        } else if ((addr & 3) == 3) {
            respData.push_back((data >> 24) & 0xFF);
            size--;
            addr++;
        }
    }
    return bufToHex(respData.data(), respData.size());
}

void DebugInterface::writeMemory(uintptr_t addr, std::vector<uint8_t> data) {
    printf("Write memory %08lX: ", addr);
    for (auto val : data) {
        printf("%02X ", val);
    }
    printf("\n");
    exit(1);
}

void DebugInterface::receivedCmd(const std::string &cmd) {
    if (cmd.empty())
        return;

    if (cmd == "?") {
        // last signal
        sendStopReply();

    } else if (cmd == "qAttached") {
        sendResponse("1");
    } else if (cmd == "qTStatus") {
        sendResponse("T0");

        // } else if (cmd == "vCont?") {
        //     sendResponse("vCont;c;s;t");

    } else if (cmd == "Hg0") {
        sendResponse("OK");
    } else if (cmd == "Hc-1") {
        sendResponse("OK");
    } else if (cmd == "qfThreadInfo") {
        sendResponse("m00");
    } else if (cmd == "qsThreadInfo") {
        sendResponse("l");
    } else if (cmd == "qC") {
        sendResponse("QC0000");
    } else if (cmd == "qOffsets") {
        sendResponse("Text=00;Data=00;Bss=00");
    } else if (cmd == "qSymbol") {
        sendResponse("OK");

    } else if (cmd == "g") {
        // read registers
        uint32_t regs[33];
        for (int i = 0; i < 32; i++) {
            regs[i] = emuState.cpu.regs[i];
        }
        regs[32] = emuState.cpu.pc;
        sendResponse(bufToHex(regs, sizeof(regs)));

    } else if (cmd[0] == 'm') {
        char    *endptr;
        unsigned addr = strtoul(cmd.c_str() + 1, &endptr, 16);
        if (endptr[0] == ',') {
            unsigned size = strtoul(endptr + 1, &endptr, 16);
            sendResponse(readMemory(addr, size));
        }

    } else if (cmd[0] == 'M') {
        char    *endptr;
        unsigned addr = strtoul(cmd.c_str() + 1, &endptr, 16);
        if (endptr[0] == ',') {
            unsigned size = strtoul(endptr + 1, &endptr, 16);

            if (endptr[0] == ':') {
                std::vector<uint8_t> data;
                const char          *p = endptr + 1;

                for (unsigned i = 0; i < size; i++) {
                    uint8_t val = 0;

                    for (int j = 0; j < 2; j++) {
                        uint8_t ch = *(p++);
                        if (ch >= '0' && ch <= '9') {
                            val = (val << 4) | (ch - '0');
                        } else if (ch >= 'a' && ch <= 'f') {
                            val = (val << 4) | (ch - 'a' + 10);
                        } else if (ch >= 'A' && ch <= 'F') {
                            val = (val << 4) | (ch - 'A' + 10);
                        } else {
                            return;
                        }
                    }

                    data.push_back(val);
                }

                writeMemory(addr, data);
                sendResponse("OK");
            }
        }

    } else if (cmd.substr(0, 3) == "Z0," || cmd.substr(0, 3) == "Z1,") {
        // Add breakpoint
        uint32_t addr = strtoul(cmd.c_str() + 3, nullptr, 16);
        dprintf("Add breakpoint %08X\n", addr);

        emuState.enableBreakpoints = true;
        for (auto &bp : emuState.breakpoints) {
            if (bp.addr == addr) {
                bp.enabled = true;
                sendResponse("OK");
                return;
            }
        }

        EmuState::Breakpoint bp;
        bp.addr    = addr;
        bp.enabled = true;
        emuState.breakpoints.push_back(bp);
        sendResponse("OK");

    } else if (cmd.substr(0, 3) == "z0," || cmd.substr(0, 3) == "z1,") {
        // Remove breakpoint
        uint32_t addr = strtoul(cmd.c_str() + 3, nullptr, 16);
        dprintf("Remove breakpoint %08X\n", addr);

        auto it = emuState.breakpoints.begin();
        while (it != emuState.breakpoints.end()) {
            if (it->addr == addr) {
                it = emuState.breakpoints.erase(it);
            } else {
                ++it;
            }
        }

        sendResponse("OK");

    } else if (cmd == "c") {
        dprintf("Continue\n");
        emuState.emuMode = EmuState::Em_Running;

    } else if (cmd == "s") {
        // Step
        dprintf("Step\n");
        emuState.emulateStep();
        sendStopReply();

        // } else if (cmd.substr(0, 11) == "qSupported:") {
        //     std::vector<std::string> features;

        //     std::string cmd2 = cmd.substr(11);

        //     size_t start;
        //     size_t end = 0;
        //     while ((start = cmd2.find_first_not_of(";", end)) != std::string::npos) {
        //         end = cmd2.find_first_of(";", start);
        //         features.push_back(cmd2.substr(start, end - start));
        //     }

        //     std::string resp;

        //     for (const auto &feature : features) {
        //         bool supported = false;

        //         if (feature == "hwbreak+") {
        //             supported = true;
        //         }

        //         if (feature.substr(feature.length() - 1) == "+") {
        //             if (!resp.empty())
        //                 resp += ";";
        //             resp += feature.substr(0, feature.length() - 1) + (supported ? "+" : "-");
        //         }
        //     }

        //     sendResponse(resp);

    } else {
        // Unsupported command
        sendResponse("");
    }
}

void DebugInterface::sendResponse(const std::string &resp) {
    uint8_t checksum = 0;
    for (uint8_t ch : resp) {
        checksum += ch;
    }

    char csStr[3];
    snprintf(csStr, sizeof(csStr), "%02X", checksum);

    std::string str = "$" + resp + "#" + csStr;
    send(conn, str.c_str(), str.length(), 0);

    dprintf("< %s\n", str.c_str());
}

void DebugInterface::sendStopReply(uint8_t signal) {
    std::string resp = fmtstr("S%02X", signal);

    // for (int i = 1; i < 32; i++) {
    //     resp += fmtstr("%02X:%08X;", i, __builtin_bswap32(emuState.cpu.regs[i]));
    // }
    // resp += fmtstr("%02X:%08X;", 32, __builtin_bswap32(emuState.cpu.pc));
    sendResponse(resp);
}
