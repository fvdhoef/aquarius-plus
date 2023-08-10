#pragma once

#include "Common.h"
#include <SDL.h>

class UI {
public:
    UI();

    void start(const std::string &romPath, const std::string &sdCardPath, const std::string &cartRomPath, const std::string &typeInStr);

private:
    void mainLoop();

    void renderScreen();
    void renderTexture();
    void emulate();

    void showAboutWindow(bool *p_open);

    SDL_Texture  *texture  = nullptr;
    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;
};
