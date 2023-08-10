#pragma once

#include "Common.h"
#include <SDL.h>

class UI {
public:
    UI();

    void start(
        const std::string &romPath,
        const std::string &sdCardPath,
        const std::string &cartRomPath,
        const std::string &typeInStr);

private:
    void renderScreen(SDL_Renderer *renderer);
    void renderTexture(SDL_Renderer *renderer);
    void emulate(SDL_Renderer *renderer);

    void showAboutWindow(bool *p_open);

    SDL_Texture *texture = NULL;
};
