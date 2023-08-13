#include "Common.h"
#if _WIN32
#    include <Windows.h>
#    include <shlobj.h>
#else
#    undef main
#endif
#include "UI.h"

int main(int argc, char *argv[]) {
    std::string basePath = SDL_GetBasePath();
    stripTrailingSlashes(basePath);

    std::string romPath = basePath + "/aquarius.rom";
    std::string cartRomPath;
    std::string sdCardPath;
    std::string typeInStr;

    int  opt;
    bool paramsOk = true;
    bool showHelp = false;
    while ((opt = getopt(argc, argv, "hr:c:u:t:")) != -1) {
        if (opt == '?' || opt == ':') {
            paramsOk = false;
            break;
        }
        switch (opt) {
            case 'h': showHelp = true; break;
            case 'r': romPath = optarg; break;
            case 'c': cartRomPath = optarg; break;
            case 'u': sdCardPath = optarg; break;
            case 't': typeInStr = optarg; break;
            default: paramsOk = false; break;
        }
    }
    stripTrailingSlashes(sdCardPath);

    if (optind != argc || showHelp) {
        paramsOk = false;
    }
    if (!paramsOk) {
        sdCardPath.clear();
    }

    // Get app data path
    std::string appDataPath;
#ifndef _WIN32
    {
        std::string homeDir = getpwuid(getuid())->pw_dir;
        appDataPath         = homeDir + "/.config";
        mkdir(appDataPath.c_str(), 0755);
        appDataPath += "/AquariusPlusEmu";
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
        appDataPath += "/AquariusPlusEmu";
        mkdir(appDataPath.c_str());
    }
#endif

#ifdef __APPLE__
    if (sdCardPath.empty()) {
        std::string homeDir = getpwuid(getuid())->pw_dir;
        sdCardPath          = homeDir + "/Documents/AquariusPlusDisk";
        mkdir(sdCardPath.c_str(), 0755);
    }
#elif _WIN32
    if (sdCardPath.empty()) {
        // PWSTR path = NULL;
        // char  path2[MAX_PATH];
        // SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
        // WideCharToMultiByte(CP_UTF8, 0, path, -1, path2, sizeof(path2), NULL, NULL);
        // CoTaskMemFree(path);

        // sdCardPath = std::string(path2) + "/AquariusPlusDisk";
        sdCardPath = basePath + "/sdcard";
    }
    mkdir(sdCardPath.c_str());
#else
    if (sdCardPath.empty()) {
        sdCardPath = basePath + "/sdcard";
    }
#endif

    if (!paramsOk) {
        fprintf(stderr, "Usage: %s <options>\n\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "-h          This help screen\n");
        fprintf(stderr, "-r <path>   Set system ROM image path (default: %s/aquarius.rom)\n", basePath.c_str());
        fprintf(stderr, "-c <path>   Set cartridge ROM path\n");
        fprintf(stderr, "-u <path>   SD card base path (default: %s)\n", sdCardPath.c_str());
        fprintf(stderr, "-t <string> Type in string.\n");
        fprintf(stderr, "\n");
        exit(1);
    }

    UI ui;
    ui.start(romPath, sdCardPath, cartRomPath, typeInStr);
    return 0;
}
