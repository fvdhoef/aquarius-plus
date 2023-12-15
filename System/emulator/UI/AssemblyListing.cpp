#include "AssemblyListing.h"
#include "Config.h"
#include <stack>

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(),
            s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

AssemblyListing::AssemblyListing() {
    auto cfgPath = Config::instance().asmListingPath;
    if (!cfgPath.empty()) {
        load(cfgPath);
    }
}

void AssemblyListing::load(const std::string &_path) {
    try {
        lines.clear();

        path = _path;
        std::ifstream ifs(path);
        if (!ifs.good()) {
            path.clear();
            return;
        }

        std::stack<int> lineNrStack;

        std::string curFile;
        std::string mainFileName;

        int  lineNr             = 1;
        bool prevLineHasInclude = false;

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) {
                // End of listing, start of statistics and symbol table
                break;
            }

            int         addr = -1;
            std::string bytes;
            bool        incrLineNr = true;

            // Convert tabs to spaces
            std::string line2;
            for (auto ch : line) {
                if (ch == '\t') {
                    do {
                        line2.push_back(' ');
                    } while ((line2.size() & 7) != 0);
                } else {
                    line2.push_back(ch);
                }
            }
            line = line2;

            if (line.rfind("**** ", 0) == 0) {
                // File switch
                curFile = line.substr(5, line.rfind(" ****") - 5);

                // Remove path from filename
                size_t pos = curFile.rfind("/");
                if (pos != curFile.npos) {
                    curFile = curFile.substr(pos + 1);
                }

                if (prevLineHasInclude) {
                    lineNrStack.push(lineNr);
                    lineNr = 1;
                } else if (!lineNrStack.empty()) {
                    lineNr = lineNrStack.top();
                    lineNrStack.pop();
                }

                if (lineNrStack.empty()) {
                    // The zasm listing does not include the name of the initial file,
                    // but it shows it when switching back from an include file, so save it here.
                    mainFileName = curFile;
                }

                continue;

            } else if (line.rfind("                ", 0) == 0) {
                // Line without address or bytes
                line = line.substr(16);

            } else if (line.rfind("      ", 0) == 0) {
                // Line without address, but with bytes
                bytes      = line.substr(6);
                line       = "";
                incrLineNr = false;

            } else {
                // Line with address and bytes
                if (line.size() > 4) {
                    addr = strtoul(line.c_str(), nullptr, 16);
                }
                bytes = line.substr(6, 8);
                line  = line.substr(16);
            }

            trim(bytes);
            if (bytes.size() < 9) {
                bytes.insert(bytes.end(), 9 - bytes.size(), ' ');
            }

            prevLineHasInclude = false;
            {
                // Detect include statement
                auto l = line;
                trim(l);
                if (strncasecmp(l.c_str(), "include", 7) == 0) {
                    prevLineHasInclude = true;
                }
            }

            lines.emplace_back(curFile, lineNr, addr, bytes, line);
            if (incrLineNr)
                lineNr++;
        }

        // Update initial lines with empty filename
        for (auto &l : lines) {
            if (!l.file.empty())
                break;
            l.file = mainFileName;
        }

        Config::instance().asmListingPath = path;

    } catch (...) {
        clear();
    }
}

void AssemblyListing::clear() {
    lines.clear();
    path.clear();
    Config::instance().asmListingPath.clear();
}
