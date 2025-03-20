#include "Common.h"
#include "Video.h"
#include "riscv.h"
#include <SDL.h>
#include "EmuState.h"
#include "UI.h"
#include "Config.h"

int main(int argc, char *argv[]) {
    std::string basePath = SDL_GetBasePath();
    stripTrailingSlashes(basePath);

    // Get app data path
    std::string appDataPath;
#ifndef _WIN32
    {
        std::string homeDir = getpwuid(getuid())->pw_dir;
        appDataPath         = homeDir + "/.config";
        mkdir(appDataPath.c_str(), 0755);
        appDataPath += "/RiscVEmu";
        mkdir(appDataPath.c_str(), 0755);
    }
#else
    {
        PWSTR path = NULL;
        char  path2[MAX_PATH];
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
        WideCharToMultiByte(CP_UTF8, 0, path, -1, path2, sizeof(path2), NULL, NULL);
        CoTaskMemFree(path);

        appDataPath = path2;
        appDataPath += "/RiscVEmu";
        mkdir(appDataPath.c_str());
    }
#endif

    auto &config = Config::instance();
    config.init(appDataPath);

    auto bootRomPath = basePath + "/bootrom.bin";
    auto sdCardPath  = basePath + "/sdcard.bin";

    int  opt;
    bool paramsOk = true;
    bool showHelp = false;
    while ((opt = getopt(argc, argv, "hb:s:c:")) != -1) {
        if (opt == '?' || opt == ':') {
            paramsOk = false;
            break;
        }
        switch (opt) {
            case 'h': showHelp = true; break;
            case 'b': bootRomPath = optarg; break;
            case 'c': sdCardPath = optarg; break;
            default: paramsOk = false; break;
        }
    }

    if (optind != argc || showHelp) {
        paramsOk = false;
    }

    if (!paramsOk) {
        fprintf(stderr, "Usage: %s <options>\n\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "-h          This help screen\n");
        fprintf(stderr, "-b <path>   Set Boot ROM image path (default: %s/bootrom.bin)\n", basePath.c_str());
        fprintf(stderr, "-c <path>   Set SD card image path  (default: %s/sdcard.bin)\n", basePath.c_str());
        fprintf(stderr, "\n");
        exit(1);
    }

    UI ui;
    ui.start(bootRomPath);

    Config::instance().save();

    return 0;
}
