#include "UI.h"
#include <SDL.h>

#include "EmuState.h"

#include "EmuState.h"

#include "Video.h"
#include "AqKeyboard.h"
#include "Audio.h"
#include "AqUartProtocol.h"
#include "SDCardVFS.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "MemoryEditor.h"
#include "tinyfiledialogs.h"

UI::UI() {
}

void UI::start(const std::string &romPath, const std::string &sdCardPath, const std::string &cartRomPath, const std::string &typeInStr) {
    emuState.typeInStr = typeInStr;
    AqUartProtocol::instance().init();
    SDCardVFS::instance().init(sdCardPath.c_str());

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    // Load system ROM
    if (!emuState.loadSystemROM(romPath))
        exit(1);

    // Load cartridge ROM
    if (!cartRomPath.empty() && !emuState.loadCartridgeROM(cartRomPath.c_str())) {
        fprintf(stderr, "Error loading cartridge ROM\n");
    }

    // Create main window
    unsigned long wnd_width = VIDEO_WIDTH * 2, wnd_height = VIDEO_HEIGHT * 2;
    window = SDL_CreateWindow("Aquarius+ emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wnd_width, wnd_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Create screen texture
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Initialize emulator
    Audio::instance().init();
    emuState.reset();
    Audio::instance().start();
    AqKeyboard::instance().init();

    // Run main loop
    mainLoop();

    // Deinitialize
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void UI::mainLoop() {
    ImGuiIO &io = ImGui::GetIO();

    bool showScreenWindow = false;
    bool showMemEdit      = false;
    bool showRegsWindow   = false;
    bool showAppAbout     = false;
    bool showDemoWindow   = false;

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    // Don't pass keypresses to emulator when ImGUI has keyboard focus
                    if (io.WantCaptureKeyboard)
                        break;

                    if (!event.key.repeat && event.key.keysym.scancode <= 255) {
                        AqKeyboard::instance().handleScancode(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
                        AqKeyboard::instance().updateMatrix();
                    }
                    break;
                }

                // Called everytime an audio buffer is done playing
                case SDL_USEREVENT: emulate(); break;

                default:
                    if (event.type == SDL_QUIT)
                        done = true;
                    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                        done = true;
                    break;
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("System")) {
                if (ImGui::MenuItem("Load cartridge ROM...", "")) {
                    char const *lFilterPatterns[1] = {"*.rom"};
                    char       *romFile            = tinyfd_openFileDialog("Open ROM file", "", 1, lFilterPatterns, "ROM files", 0);
                    if (romFile) {
                        if (emuState.loadCartridgeROM(romFile)) {
                            emuState.reset();
                        }
                    }
                }
                if (ImGui::MenuItem("Eject cartridge", "", false, emuState.cartridgeInserted)) {
                    emuState.cartridgeInserted = false;
                    emuState.reset();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Aquarius+", "")) {
                    emuState.reset();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "")) {
                    done = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Keyboard")) {
                bool scrLk = AqKeyboard::instance().scrollLockOn();
                if (ImGui::MenuItem("Cursor keys & F1-F6 emulate hand controller (ScrLk)", "", &scrLk)) {
                    AqKeyboard::instance().setScrollLock(scrLk);
                }
                if (ImGui::MenuItem("Paste text from clipboard", "")) {
                    emuState.typeInStr = io.GetClipboardTextFn(nullptr);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("Screen in window", "")) {
                    showScreenWindow = true;
                }
                if (ImGui::MenuItem("Memory editor", "")) {
                    showMemEdit = true;
                }
                if (ImGui::MenuItem("Registers", "")) {
                    showRegsWindow = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About", ""))
                    showAppAbout = true;
                ImGui::Separator();
                if (ImGui::MenuItem("ImGui library demo window", ""))
                    showDemoWindow = true;

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (showScreenWindow)
            wndScreen(&showScreenWindow);
        if (showMemEdit)
            wndMemEdit(&showMemEdit);
        if (showRegsWindow)
            wndRegs(&showRegsWindow);
        if (showAppAbout)
            wndAbout(&showAppAbout);
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!showScreenWindow) {
            renderTexture();
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
}

void UI::renderScreen() {
    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    const uint16_t *fb = emuState.video.getFb();

    for (int j = 0; j < VIDEO_HEIGHT; j++) {
        for (int i = 0; i < VIDEO_WIDTH; i++) {

            // Convert from RGB444 to RGB888
            uint16_t col444 = fb[j * VIDEO_WIDTH + i];

            unsigned r4 = (col444 >> 8) & 0xF;
            unsigned g4 = (col444 >> 4) & 0xF;
            unsigned b4 = (col444 >> 0) & 0xF;

            unsigned r8 = (r4 << 4) | r4;
            unsigned g8 = (g4 << 4) | g4;
            unsigned b8 = (b4 << 4) | b4;

            ((uint32_t *)((uintptr_t)pixels + j * pitch))[i] = (r8 << 16) | (g8 << 8) | (b8);
        }
    }

    SDL_UnlockTexture(texture);
}

void UI::renderTexture() {
    float rsx, rsy;
    SDL_RenderGetScale(renderer, &rsx, &rsy);
    auto  drawData = ImGui::GetDrawData();
    float scaleX   = (rsx == 1.0f) ? drawData->FramebufferScale.x : 1.0f;
    float scaleY   = (rsy == 1.0f) ? drawData->FramebufferScale.y : 1.0f;
    int   w        = (int)(drawData->DisplaySize.x * scaleX);
    int   h        = (int)(drawData->DisplaySize.y * scaleY);
    if (w <= 0 || h <= 0)
        return;

    // Retain aspect ratio
    int w1 = (w / VIDEO_WIDTH) * VIDEO_WIDTH;
    int h1 = (w1 * VIDEO_HEIGHT) / VIDEO_WIDTH;
    int h2 = (h / VIDEO_HEIGHT) * VIDEO_HEIGHT;
    int w2 = (h2 * VIDEO_WIDTH) / VIDEO_HEIGHT;

    int sw, sh;
    if (w1 == 0 || h1 == 0) {
        sw = w;
        sh = h;
    } else if (w1 <= w && h1 <= h) {
        sw = w1;
        sh = h1;
    } else {
        sw = w2;
        sh = h2;
    }

    SDL_Rect dst;
    dst.w = (int)sw;
    dst.h = (int)sh;
    dst.x = (w - dst.w) / 2;
    dst.y = (h - dst.h) / 2;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

void UI::emulate() {
    // Emulation is performed in sync with the audio. This function will run
    // for the time needed to fill 1 audio buffer, which is about 1/60 of a
    // second.

    // Get a buffer from audio subsystem.
    auto abuf = Audio::instance().getBuffer();
    if (abuf == NULL) {
        // No buffer available, don't emulate for now.
        return;
    }

    // Render each audio sample
    for (int aidx = 0; aidx < SAMPLES_PER_BUFFER; aidx++) {
        do {
            emuState.z80ctx.tstates = 0;
            Z80Execute(&emuState.z80ctx);
            int delta = emuState.z80ctx.tstates * 2;

            int prevLineHalfCycles = emuState.lineHalfCycles;

            emuState.lineHalfCycles += delta;
            emuState.sampleHalfCycles += delta;

            // Handle VIRQLINE register
            if (prevLineHalfCycles < 320 && emuState.lineHalfCycles >= 320 && emuState.videoLine == emuState.videoIrqLine) {
                emuState.irqStatus |= (1 << 1);
            }

            if (emuState.lineHalfCycles >= HCYCLES_PER_LINE) {
                emuState.lineHalfCycles -= HCYCLES_PER_LINE;

                emuState.video.drawLine();

                emuState.videoLine++;
                if (emuState.videoLine == 200) {
                    emuState.irqStatus |= (1 << 0);

                } else if (emuState.videoLine == 262) {
                    renderScreen();
                    emuState.videoLine = 0;
                    emuState.keyboardTypeIn();
                }
            }

            // Generate interrupt if needed
            if ((emuState.irqStatus & emuState.irqMask) != 0) {
                Z80INT(&emuState.z80ctx, 0xFF);
            }
        } while (emuState.sampleHalfCycles < HCYCLES_PER_SAMPLE);

        emuState.sampleHalfCycles -= HCYCLES_PER_SAMPLE;

        uint16_t beep = emuState.soundOutput ? 10000 : 0;

        // Take average of 5 AY8910 samples to match sampling rate (16*5*44100 = 3.528MHz)
        unsigned left  = 0;
        unsigned right = 0;

        for (int i = 0; i < 5; i++) {
            uint16_t abc[3];
            emuState.ay1.render(abc);
            left += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
            right += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];

            emuState.ay2.render(abc);
            left += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
            right += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];
        }

        left  = left + beep;
        right = right + beep;

        abuf[aidx * 2 + 0] = left;
        abuf[aidx * 2 + 1] = right;
    }

    // Return buffer to audio subsystem.
    Audio::instance().putBuffer(abuf);
}

void UI::wndAbout(bool *p_open) {
    if (ImGui::Begin("About Aquarius+ emulator", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("The Aquarius+ emulator is part of the Aquarius+ project.");
        ImGui::Text("Developed by Frank van den Hoef.\n");
        ImGui::Separator();
        ImGui::Text("Members of the Aquarius+ project team:");
        ImGui::Text("- Frank van den Hoef");
        ImGui::Text("- Sean P. Harrington");
        ImGui::Text("- Curtis F. Kaylor");
        ImGui::Separator();
        ImGui::Text(
            "Thanks go out all the people contributing to this project and\n"
            "those who enjoy playing with it, either with this emulator or\n"
            "the actual hardware!");
    }
    ImGui::End();
}

void UI::wndRegs(bool *p_open) {
    bool open = ImGui::Begin("Registers", p_open, ImGuiWindowFlags_AlwaysAutoResize);
    if (open) {
        ImGui::Text("PC: %04X", emuState.z80ctx.PC);
        ImGui::Separator();
        ImGui::Text(
            "S:%u Z:%u H:%u PV:%u N:%u C:%u",
            (emuState.z80ctx.R1.br.F & F_S) ? 1 : 0,
            (emuState.z80ctx.R1.br.F & F_Z) ? 1 : 0,
            (emuState.z80ctx.R1.br.F & F_H) ? 1 : 0,
            (emuState.z80ctx.R1.br.F & F_PV) ? 1 : 0,
            (emuState.z80ctx.R1.br.F & F_N) ? 1 : 0,
            (emuState.z80ctx.R1.br.F & F_C) ? 1 : 0);
        ImGui::Separator();

        ImGui::BeginGroup();
        ImGui::Text("A : %02X", emuState.z80ctx.R1.br.A);
        ImGui::Text("F : %02X", emuState.z80ctx.R1.br.F);
        ImGui::Text("BC: %04X", emuState.z80ctx.R1.wr.BC);
        ImGui::Text("DE: %04X", emuState.z80ctx.R1.wr.DE);
        ImGui::Text("HL: %04X", emuState.z80ctx.R1.wr.HL);
        ImGui::Text("IX: %04X", emuState.z80ctx.R1.wr.IX);
        ImGui::Text("IY: %04X", emuState.z80ctx.R1.wr.IY);
        ImGui::Text("SP: %04X", emuState.z80ctx.R1.wr.SP);
        ImGui::EndGroup();
        ImGui::SameLine(0, 30);
        ImGui::BeginGroup();
        ImGui::Text("A' : %02X", emuState.z80ctx.R2.br.A);
        ImGui::Text("F' : %02X", emuState.z80ctx.R2.br.F);
        ImGui::Text("BC': %04X", emuState.z80ctx.R2.wr.BC);
        ImGui::Text("DE': %04X", emuState.z80ctx.R2.wr.DE);
        ImGui::Text("HL': %04X", emuState.z80ctx.R2.wr.HL);
        ImGui::Text("IX': %04X", emuState.z80ctx.R2.wr.IX);
        ImGui::Text("IY': %04X", emuState.z80ctx.R2.wr.IY);
        ImGui::Text("SP': %04X", emuState.z80ctx.R2.wr.SP);
        ImGui::EndGroup();
    }
    ImGui::End();
}

void UI::wndScreen(bool *p_open) {
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = ImGui::Begin("Screen", p_open, ImGuiWindowFlags_AlwaysAutoResize);
    // ImGui::PopStyleVar();

    if (open) {
        static int e = 1;
        ImGui::RadioButton("1x", &e, 1);
        ImGui::SameLine();
        ImGui::RadioButton("2x", &e, 2);
        ImGui::SameLine();
        ImGui::RadioButton("3x", &e, 3);
        ImGui::SameLine();
        ImGui::RadioButton("4x", &e, 4);

        if (texture) {
            ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2((float)(VIDEO_WIDTH * e), (float)(VIDEO_HEIGHT * e)));
        }
    }

    ImGui::End();
}

void UI::wndMemEdit(bool *p_open) {
    static MemoryEditor memEdit;

    void      *pMem            = emuState.colorRam;
    size_t     memSize         = sizeof(emuState.colorRam);
    size_t     baseDisplayAddr = 0;
    static int itemCurrent     = 0;
    switch (itemCurrent) {
        case 0:
            pMem    = emuState.screenRam;
            memSize = sizeof(emuState.screenRam);
            break;
        case 1:
            pMem    = emuState.colorRam;
            memSize = sizeof(emuState.colorRam);
            break;
        case 2:
            pMem    = emuState.systemRom;
            memSize = emuState.systemRomSize;
            break;
        case 3:
            pMem    = emuState.mainRam;
            memSize = sizeof(emuState.mainRam);
            break;
        case 4:
            pMem    = emuState.cartRom;
            memSize = sizeof(emuState.cartRom);
            break;
        case 5:
            pMem    = emuState.videoRam;
            memSize = sizeof(emuState.videoRam);
            break;
        case 6:
            pMem    = emuState.charRam;
            memSize = sizeof(emuState.charRam);
            break;
    }

    MemoryEditor::Sizes s;
    memEdit.calcSizes(s, memSize, baseDisplayAddr);
    ImGui::SetNextWindowSize(ImVec2(s.windowWidth, s.windowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(s.windowWidth, 150.0f), ImVec2(s.windowWidth, FLT_MAX));

    if (ImGui::Begin("Memory editor", p_open, ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            ImGui::OpenPopup("context");

        ImGui::Combo(
            "Memory select", &itemCurrent,
            "Screen RAM\0"
            "Color RAM\0"
            "System ROM\0"
            "Main RAM\0"
            "Cartridge ROM\0"
            "Video RAM\0"
            "Character RAM\0");

        ImGui::Separator();

        memEdit.drawContents(pMem, memSize, baseDisplayAddr);
        if (memEdit.contentsWidthChanged) {
            memEdit.calcSizes(s, memSize, baseDisplayAddr);
            ImGui::SetWindowSize(ImVec2(s.windowWidth, ImGui::GetWindowSize().y));
        }
    }
    ImGui::End();
}
