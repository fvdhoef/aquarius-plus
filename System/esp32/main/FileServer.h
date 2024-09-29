#pragma once

#include "Common.h"

class FileServer {
public:
    virtual void init() = 0;
};

FileServer *getFileServer();
