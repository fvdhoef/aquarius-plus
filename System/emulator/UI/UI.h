#pragma once

#include "Common.h"
#include <SDL.h>
#include "Audio.h"
#include "AssemblyListing.h"

class UI {
public:
    UI();

    void start(
        const std::string &romPath,
        const std::string &cartRomPath,
        const std::string &typeInStr);

private:
    void mainLoop();

    void renderScreen();
    void renderTexture();
    void emulate();

    void wndAbout(bool *p_open);
    void wndCpuState(bool *p_open);
    void wndScreen(bool *p_open);
    void wndMemEdit(bool *p_open);
    void wndBreakpoints(bool *p_open);
    void wndIoRegs(bool *p_open);
    void wndAssemblyListing(bool *p_open);

    SDL_Texture  *texture  = nullptr;
    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;

    DCBlock dcBlockLeft;
    DCBlock dcBlockRight;

    bool allowTyping = false;
    bool first       = true;

    AssemblyListing asmListing;
};
