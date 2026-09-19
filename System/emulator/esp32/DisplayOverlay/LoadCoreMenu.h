#pragma once

#include "Menu.h"
#include "VFS.h"

class LoadCoreMenu : public Menu {
public:
    LoadCoreMenu() : Menu("Change active core", 38) {
    }

    struct Core {
        std::string name;
        std::string path;
    };

    std::vector<Core> findCores() {
        std::vector<Core> result;

        {
            auto &item = result.emplace_back();
            item.name  = "Aquarius+ (built-in)";
            item.path  = "esp:aqplus.core";
        }

        auto vfsCtx = VFSContext::getDefault();
        int  dd1    = vfsCtx->openDirExt("/cores", 0, 0);
        if (dd1 >= 0) {
            DirEnumEntry dee1;
            while (vfsCtx->readDir(dd1, &dee1) == 0) {
                if ((dee1.attr & DE_ATTR_DIR) == 0)
                    continue;

                int dd2 = vfsCtx->openDirExt(("/cores/" + dee1.filename + "/*.core").c_str(), 0, 0);
                if (dd2 < 0)
                    continue;

                DirEnumEntry dee2;
                while (vfsCtx->readDir(dd2, &dee2) == 0) {
                    if ((dee2.attr & DE_ATTR_DIR) != 0)
                        continue;

                    auto &item = result.emplace_back();
                    item.name  = dee2.filename;
                    item.path  = "/cores/" + dee1.filename + "/" + dee2.filename;

                    int fd = vfsCtx->open(FO_RDONLY, "/cores/" + dee1.filename + "/" + dee2.filename + "info");
                    if (fd >= 0) {
                        char tmp[32];
                        if (vfsCtx->readline(fd, 32, tmp) >= 0) {
                            item.name = tmp;
                        }
                        vfsCtx->close(fd);
                    }
                }
                vfsCtx->closeDir(dd2);
            }
            vfsCtx->closeDir(dd1);
        }
        return result;
    }

    void loadCore(const char *path) {
        nvs_handle_t h;
        if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
            if (nvs_set_str(h, "core", path) == ESP_OK) {
                nvs_commit(h);
            }
            nvs_close(h);
        }
        FpgaCore::loadCore(path);
    }

    void onUpdate() override {
        items.clear();

        auto cores = findCores();

        for (auto &core : cores) {
            auto &item   = items.emplace_back(MenuItemType::subMenu, core.name);
            auto  path   = core.path;
            item.onEnter = [this, path]() { loadCore(path.c_str()); };
        }
    }
};
