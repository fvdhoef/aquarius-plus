#pragma once

#include "Menu.h"
#include <nvs_flash.h>
#include "Keyboard.h"

#define LATEST_TAG_LEN (32)

class GitHubUpdateMenu : public Menu {
public:
    GitHubUpdateMenu() : Menu("Select firmware version", 38) {
    }

    void onEnter() override;

private:
    void doUpdate(const char *tag);
    char latestTag[LATEST_TAG_LEN];
};
