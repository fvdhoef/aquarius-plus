#include "Common.h"

void splitPath(const std::string &path, std::vector<std::string> &result) {
    const char *delimiters = "/\\";
    size_t      start;
    size_t      end = 0;
    while ((start = path.find_first_not_of(delimiters, end)) != std::string::npos) {
        end = path.find_first_of(delimiters, start);
        result.push_back(path.substr(start, end - start));
    }
}

bool startsWith(const std::string &s1, const std::string &s2, bool caseSensitive) {
    if (caseSensitive) {
        return (strncmp(s1.c_str(), s2.c_str(), s2.size()) == 0);
    } else {
        return (strncasecmp(s1.c_str(), s2.c_str(), s2.size()) == 0);
    }
}

#include "VFS.h"
bool createPath(const std::string &path) {
    std::vector<std::string> pathElements;
    splitPath(path, pathElements);

    auto vfs = getSDCardVFS();

    std::string subPath;
    for (auto &elem : pathElements) {
        subPath = subPath + elem + '/';
        if (subPath == "/")
            continue;

        int res = vfs->mkdir(subPath);
        if (res != 0 && res != ERR_EXISTS)
            return false;
    }
    return true;
}
