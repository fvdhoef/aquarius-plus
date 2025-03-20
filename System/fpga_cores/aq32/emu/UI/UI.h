#pragma once

#include "Common.h"
#include <SDL.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "MemoryEditor.h"

class UI {
public:
    UI();
    void start(
        const std::string &romPath);

private:
    void mainLoop();

    void     renderScreen();
    SDL_Rect renderTexture(int menuHeight);
    SDL_Rect calcRenderPos(int w, int h, int menuHeight);

    void wndAbout(bool *p_open);
    void wndCpuState(bool *p_open);
    void wndScreen(bool *p_open);
    void wndMemEdit(bool *p_open);
    void wndBreakpoints(bool *p_open);
    void wndIoRegs(bool *p_open);
    void wndWatch(bool *p_open);

    SDL_Texture  *texture  = nullptr;
    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;

    bool allowTyping = false;
    bool first       = true;

    MemoryEditor memEdit;
};
