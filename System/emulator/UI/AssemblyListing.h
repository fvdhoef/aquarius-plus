#pragma once

#include "Common.h"

class AssemblyListing {
public:
    AssemblyListing();

    void load(const std::string &path);
    void clear();

    const std::string &getPath() {
        return path;
    };

    struct Line {
        Line(const std::string &_file, int _linenr, int _addr, const std::string &_bytes, const std::string &_s)
            : file(_file), linenr(_linenr), addr(_addr), bytes(_bytes), s(_s) {
        }

        std::string file;
        int         linenr;
        int         addr;
        std::string bytes;
        std::string s;
    };

    std::vector<Line>               lines;
    std::map<uint16_t, std::string> symbolsAddrStr;
    std::map<std::string, uint16_t> symbolsStrAddr;
    std::map<uint16_t, std::string> equsAddrStr;
    std::map<std::string, uint16_t> equsStrAddr;

    bool findSymbolAddr(const std::string &name, uint16_t &addr) {
        auto it = symbolsStrAddr.find(name);
        if (it == symbolsStrAddr.end()) {
            it = equsStrAddr.find(name);
            if (it == equsStrAddr.end())
                return false;
        }
        addr = it->second;
        return true;
    }

    bool findSymbolName(uint16_t addr, std::string &name) {
        auto it = symbolsAddrStr.find(addr);
        if (it == symbolsAddrStr.end()) {
            it = equsAddrStr.find(addr);
            if (it == equsAddrStr.end())
                return false;
        }
        name = it->second;
        return true;
    }

private:
    std::string path;
};
