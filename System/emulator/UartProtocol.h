#pragma once

#include "Common.h"
#include "VFS.h"

#define MAX_FDS (10)
#define MAX_DDS (10)

enum GameCtrlBtnIdx {
    GCB_A_IDX          = 0,
    GCB_B_IDX          = 1,
    GCB_X_IDX          = 2,
    GCB_Y_IDX          = 3,
    GCB_VIEW_IDX       = 4,
    GCB_GUIDE_IDX      = 5,
    GCB_MENU_IDX       = 6,
    GCB_LS_IDX         = 7,
    GCB_RS_IDX         = 8,
    GCB_LB_IDX         = 9,
    GCB_RB_IDX         = 10,
    GCB_DPAD_UP_IDX    = 11,
    GCB_DPAD_DOWN_IDX  = 12,
    GCB_DPAD_LEFT_IDX  = 13,
    GCB_DPAD_RIGHT_IDX = 14,
    GCB_SHARE_IDX      = 15,

    GCB_A          = (1 << GCB_A_IDX),
    GCB_B          = (1 << GCB_B_IDX),
    GCB_X          = (1 << GCB_X_IDX),
    GCB_Y          = (1 << GCB_Y_IDX),
    GCB_VIEW       = (1 << GCB_VIEW_IDX),
    GCB_GUIDE      = (1 << GCB_GUIDE_IDX),
    GCB_MENU       = (1 << GCB_MENU_IDX),
    GCB_LS         = (1 << GCB_LS_IDX),
    GCB_RS         = (1 << GCB_RS_IDX),
    GCB_LB         = (1 << GCB_LB_IDX),
    GCB_RB         = (1 << GCB_RB_IDX),
    GCB_DPAD_UP    = (1 << GCB_DPAD_UP_IDX),
    GCB_DPAD_DOWN  = (1 << GCB_DPAD_DOWN_IDX),
    GCB_DPAD_LEFT  = (1 << GCB_DPAD_LEFT_IDX),
    GCB_DPAD_RIGHT = (1 << GCB_DPAD_RIGHT_IDX),
    GCB_SHARE      = (1 << GCB_SHARE_IDX),
};

class UartProtocol {
    UartProtocol();

public:
    static UartProtocol *instance();

    void init();

    void    writeCtrl(uint8_t data);
    void    writeData(uint8_t data);
    uint8_t readCtrl();
    uint8_t readData();

    struct FileInfo {
        uint8_t     flags;
        std::string name;
        unsigned    offset;
    };
    std::map<uint8_t, FileInfo> fi;

    struct DirInfo {
        std::string name;
        unsigned    offset;
    };
    std::map<uint8_t, DirInfo> di;

private:
    int txFifoRead();

    void        txFifoWrite(uint8_t data);
    void        txFifoWrite(const void *buf, size_t length);
    void        splitPath(const std::string &path, std::vector<std::string> &result);
    std::string resolvePath(std::string path, VFS **vfs, std::string *wildCard = nullptr);
    void        closeAllDescriptors();
    void        receivedByte(uint8_t data);

    void cmdReset();
    void cmdVersion();
    void cmdGetDateTime(uint8_t type);
    void cmdKeyMode(uint8_t mode);
    void cmdGetMouse();
    void cmdGetGameCtrl(uint8_t idx);
    void cmdOpen(uint8_t flags, const char *pathArg);
    void cmdClose(uint8_t fd);
    void cmdRead(uint8_t fd, uint16_t size);
    void cmdReadLine(uint8_t fd, uint16_t size);
    void cmdWrite(uint8_t fd, uint16_t size, const void *data);
    void cmdSeek(uint8_t fd, uint32_t offset);
    void cmdTell(uint8_t fd);
    void cmdOpenDirExt(const char *pathArg, uint8_t flags, uint16_t skip_count);
    void cmdCloseDir(uint8_t dd);
    void cmdReadDir(uint8_t dd);
    void cmdDelete(const char *pathArg);
    void cmdRename(const char *oldArg, const char *newArg);
    void cmdMkDir(const char *pathArg);
    void cmdChDir(const char *pathArg);
    void cmdStat(const char *pathArg);
    void cmdGetCwd();
    void cmdCloseAll();
    void cmdLoadFpga(const char *pathArg);

    std::string currentPath;
    uint8_t     rxBuf[16 + 0x10000];
    unsigned    rxBufIdx = 0;
    VFS        *fdVfs[MAX_FDS];
    uint8_t     fds[MAX_FDS];
    DirEnumCtx  deCtxs[MAX_DDS];
    int         deIdx[MAX_DDS];
    const char *newPath = nullptr;

    uint8_t  txBuf[0x10000 + 16];
    unsigned txBufWrIdx = 0;
    unsigned txBufRdIdx = 0;
    unsigned txBufCnt   = 0;

    bool    mousePresent = false;
    float   mouseX       = 0;
    float   mouseY       = 0;
    uint8_t mouseButtons = 0;
    int     mouseWheel   = 0;

public:
    bool     gameCtrlPresent = false;
    int8_t   gameCtrlLX      = 0;
    int8_t   gameCtrlLY      = 0;
    int8_t   gameCtrlRX      = 0;
    int8_t   gameCtrlRY      = 0;
    uint8_t  gameCtrlLT      = 0;
    uint8_t  gameCtrlRT      = 0;
    uint16_t gameCtrlButtons = 0;

    void gameCtrlReset(bool present) {
        gameCtrlPresent = present;
        gameCtrlLX      = 0;
        gameCtrlLY      = 0;
        gameCtrlRX      = 0;
        gameCtrlRY      = 0;
        gameCtrlLT      = 0;
        gameCtrlRT      = 0;
        gameCtrlButtons = 0;
        gameCtrlUpdated();
    }

    void gameCtrlUpdated();

private:
    friend class UI;
};
