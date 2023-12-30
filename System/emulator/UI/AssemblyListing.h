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

    std::vector<Line>                    lines;
    std::multimap<uint16_t, std::string> symbolsAddrStr;
    std::multimap<std::string, uint16_t> symbolsStrAddr;

    bool findSymbolAddr(const std::string &name, uint16_t &addr) {
        auto it = symbolsStrAddr.find(name);
        if (it == symbolsStrAddr.end())
            return false;
        addr = it->second;
        return true;
    }

    bool findSymbolName(uint16_t addr, std::string &name) {
        auto it = symbolsAddrStr.find(addr);
        if (it == symbolsAddrStr.end())
            return false;
        name = it->second;
        return true;
    }

    bool findNearestSymbol(uint16_t &addr, std::string &name) {
        auto it = symbolsAddrStr.lower_bound(addr);
        if (it == symbolsAddrStr.end())
            return false;

        ++it;
        if (it != symbolsAddrStr.end() && it->first == addr) {
            name = it->second;
            return true;
        }
        --it;
        if (it->first <= addr) {
            name = it->second;
            addr = it->first;
            return true;
        }
        if (it != symbolsAddrStr.begin()) {
            --it;
            if (it->first <= addr) {
                name = it->second;
                addr = it->first;
                return true;
            }
        }

        return false;
    }

private:
    std::string path;
};
