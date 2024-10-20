#pragma once

#include "Common.h"

class FileServer {
public:
    virtual void start()     = 0;
    virtual void stop()      = 0;
    virtual bool isRunning() = 0;
};

FileServer *getFileServer();
