#pragma once

#include "Common.h"
#include <SDL.h>
#include "Audio.h"
#include "AssemblyListing.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "MemoryEditor.h"

class UI {
public:
    UI();

    void start(
        const std::string &romPath,
        const std::string &cartRomPath,
        const std::string &typeInStr);

private:
    void mainLoop();

    void     renderScreen();
    SDL_Rect renderTexture(int menuHeight);
    SDL_Rect calcRenderPos(int w, int h, int menuHeight);
    void     emulate();

    void wndAbout(bool *p_open);
    void wndCpuState(bool *p_open);
    void wndScreen(bool *p_open);
    void wndMemEdit(bool *p_open);
    void wndBreakpoints(bool *p_open);
    void wndIoRegs(bool *p_open);
    void wndAssemblyListing(bool *p_open);
    void wndCpuTrace(bool *p_open);
    void wndEspInfo(bool *p_open);

    SDL_Texture  *texture  = nullptr;
    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;

    DCBlock dcBlockLeft;
    DCBlock dcBlockRight;

    bool allowTyping = false;
    bool first       = true;

    uint64_t lastKeyRepeatCall = 0;

    AssemblyListing asmListing;
    MemoryEditor    memEdit;

    void addrPopup(uint16_t addr);
};
