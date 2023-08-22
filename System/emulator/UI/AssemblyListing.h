#pragma once

#include "Common.h"

class AssemblyListing {
public:
    AssemblyListing();

    void load(const std::string &path);

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

    std::vector<Line> lines;

private:
    std::string path;
};
