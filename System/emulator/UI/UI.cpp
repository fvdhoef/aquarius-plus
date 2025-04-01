#include "UI.h"
#include <SDL.h>

#include "EmuState.h"

#include "Video.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "tinyfiledialogs.h"
#include "Config.h"

#include "lodepng.h"

UI::UI() {
}

void UI::start(
    const std::string &cartRomPath,
    const std::string &typeInStr) {

    auto config = Config::instance();

    emuState.typeInStr  = typeInStr;
    emuState.stopOnHalt = config->stopOnHalt;
    UartProtocol::instance()->init();
    setSDCardPath(config->sdCardPath);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    // Load cartridge ROM
    if (!cartRomPath.empty() && !emuState.loadCartridgeROM(cartRomPath.c_str())) {
        fprintf(stderr, "Error loading cartridge ROM\n");
    }

    // Create main window
    window = SDL_CreateWindow("Aquarius+ emulator", config->wndPosX, config->wndPosY, config->wndWidth, config->wndHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
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
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT * 2);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr; // imguiIniFileName.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ImGui::LoadIniSettingsFromMemory(config->imguiConf.c_str());

    // Initialize emulator
    Audio::instance()->init();
    emuState.coldReset();
    Audio::instance()->start();

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
    ImGuiIO &io         = ImGui::GetIO();
    auto    &platformIO = ImGui::GetPlatformIO();
    auto     config     = Config::instance();

    bool showAppAbout   = false;
    bool showDemoWindow = false;

    bool escapePressed = false;

    int tooSlow = 0;

    listingReloaded();

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        escapePressed = (event.type == SDL_KEYDOWN);
                    }
                    // We decode CTRL-ESCAPE in this weird way to allow the sequence ESCAPE and then CTRL to be used on Windows.
                    if (escapePressed && !event.key.repeat && event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_LCTRL)) {
                        emuState.warmReset();
                        break;
                    }

                    // Don't pass keypresses to emulator when ImGUI has keyboard focus
                    if (io.WantCaptureKeyboard)
                        break;

                    if (allowTyping) {
                        if (!event.key.repeat && event.key.keysym.scancode <= 255) {
                            Keyboard::instance()->handleScancode(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
                            Keyboard::instance()->updateMatrix();
                        }
                    }
                    break;
                }

                case SDL_MOUSEWHEEL: {
                    if (!io.WantCaptureMouse) {
                        UartProtocol::instance()->mouseWheel += event.wheel.y;
                    }
                    break;
                }

                case SDL_WINDOWEVENT: {
                    if (event.window.event == SDL_WINDOWEVENT_MOVED) {
                        config->wndPosX = event.window.data1;
                        config->wndPosY = event.window.data2;

                    } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        config->wndWidth  = event.window.data1;
                        config->wndHeight = event.window.data2;
                    }
                    break;
                }

                case SDL_CONTROLLERDEVICEADDED: {
                    if (!gameCtrl) {
                        gameCtrlIdx = event.cdevice.which;
                        gameCtrl    = SDL_GameControllerOpen(gameCtrlIdx);
                        UartProtocol::instance()->gameCtrlReset(true);
                    }
                    break;
                }
                case SDL_CONTROLLERDEVICEREMOVED: {
                    if (gameCtrlIdx == event.cdevice.which) {
                        SDL_GameControllerClose(gameCtrl);
                        gameCtrl    = nullptr;
                        gameCtrlIdx = -1;
                        UartProtocol::instance()->gameCtrlReset(false);
                    }
                    break;
                }

                case SDL_CONTROLLERAXISMOTION: {
                    auto aqp = UartProtocol::instance();
                    switch (event.caxis.axis) {
                        case SDL_CONTROLLER_AXIS_LEFTX: aqp->gameCtrlLX = event.caxis.value / 256; break;
                        case SDL_CONTROLLER_AXIS_LEFTY: aqp->gameCtrlLY = event.caxis.value / 256; break;
                        case SDL_CONTROLLER_AXIS_RIGHTX: aqp->gameCtrlRX = event.caxis.value / 256; break;
                        case SDL_CONTROLLER_AXIS_RIGHTY: aqp->gameCtrlRY = event.caxis.value / 256; break;
                        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: aqp->gameCtrlLT = event.caxis.value / 128; break;
                        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: aqp->gameCtrlRT = event.caxis.value / 128; break;
                    }
                    aqp->gameCtrlUpdated();
                    break;
                }
                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP: {
                    if (event.cbutton.button < 16) {
                        auto aqp             = UartProtocol::instance();
                        aqp->gameCtrlButtons = (aqp->gameCtrlButtons & ~(1 << event.cbutton.button)) | ((event.cbutton.state & 1) << event.cbutton.button);
                        aqp->gameCtrlUpdated();
                    }
                    break;
                }

                default:
                    if (event.type == SDL_QUIT)
                        done = true;
                    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                        done = true;
                    break;
            }
        }

        // Emulate
        {
            int bufsToRender = Audio::instance()->bufsToRender();
            if (bufsToRender == 0)
                continue;

            if (io.DeltaTime > 0.050f) {
                tooSlow++;
            } else {
                tooSlow = 0;
            }
            if (tooSlow >= 4) {
                tooSlow = 0;
                if (emuState.emulationSpeed > 1) {
                    emuState.emulationSpeed--;
                }
            }
            for (int i = 0; i < bufsToRender; i++) {
                emulate();
            }
        }

        if (io.WantSaveIniSettings) {
            config->imguiConf      = ImGui::SaveIniSettingsToMemory();
            io.WantSaveIniSettings = false;
        }

        // Keyboard::instance()->setScrollLock(config->handCtrlEmulation);

        io.FontGlobalScale = config->fontScale2x ? 2.0f : 1.0f;

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Safe-guard for misbehaving video drivers that don't lock on v-sync
        auto ticks = SDL_GetTicks64();
        if (ticks - lastKeyRepeatCall >= 16) {
            lastKeyRepeatCall = ticks;
            Keyboard::instance()->keyRepeatTimer();
        }

        renderScreen();

        ImVec2 menuBarSize;
        if (ImGui::BeginMainMenuBar()) {
            menuBarSize = ImGui::GetWindowSize();

            if (ImGui::BeginMenu("System")) {
                if (ImGui::MenuItem("Select SD card directory...", "")) {
                    auto path = tinyfd_selectFolderDialog("Select SD card directory", nullptr);
                    if (path) {
                        config->sdCardPath = path;
                        stripTrailingSlashes(config->sdCardPath);
                        setSDCardPath(config->sdCardPath);
                    }
                }
                std::string ejectLabel = "Eject SD card";
                if (!config->sdCardPath.empty()) {
                    ejectLabel += " (" + config->sdCardPath + ")";
                }
                if (ImGui::MenuItem(ejectLabel.c_str(), "", false, !config->sdCardPath.empty())) {
                    config->sdCardPath.clear();
                    setSDCardPath("");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Load cartridge ROM...", "")) {
                    char const *lFilterPatterns[1] = {"*.rom"};
                    char       *romFile            = tinyfd_openFileDialog("Open ROM file", "", 1, lFilterPatterns, "ROM files", 0);
                    if (romFile) {
                        if (emuState.loadCartridgeROM(romFile)) {
                            emuState.coldReset();
                        }
                    }
                }
                if (ImGui::MenuItem("Eject cartridge", "", false, emuState.cartridgeInserted)) {
                    emuState.cartridgeInserted = false;
                    emuState.coldReset();
                }
                ImGui::Separator();
                ImGui::MenuItem("Enable sound", "", &config->enableSound);
                ImGui::MenuItem("Enable mouse", "", &config->enableMouse);
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Aquarius+ (warm)", "")) {
                    emuState.warmReset();
                }
                if (ImGui::MenuItem("Reset Aquarius+ (cold)", "")) {
                    emuState.coldReset();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "")) {
                    done = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Screen")) {
                if (ImGui::MenuItem("Save screenshot...", "")) {
                    char const *lFilterPatterns[1] = {"*.png"};
                    char       *path               = tinyfd_saveFileDialog("Save screenshot", "", 1, lFilterPatterns, "PNG files");
                    if (path) {
                        std::string pngFile = path;
                        if (pngFile.size() < 4 || pngFile.substr(pngFile.size() - 4) != ".png")
                            pngFile += ".png";

                        auto *fb = emuState.video.getFb();

                        std::vector<uint32_t> tmpBuf;
                        tmpBuf.resize(VIDEO_WIDTH * VIDEO_HEIGHT * 2);
                        for (int j = 0; j < VIDEO_HEIGHT * 2; j++) {
                            for (int i = 0; i < VIDEO_WIDTH; i++) {
                                // Convert from RGB444 to RGB888
                                uint16_t col444 = fb[j / 2 * VIDEO_WIDTH + i];

                                unsigned r4 = (col444 >> 8) & 0xF;
                                unsigned g4 = (col444 >> 4) & 0xF;
                                unsigned b4 = (col444 >> 0) & 0xF;

                                unsigned r8 = (r4 << 4) | r4;
                                unsigned g8 = (g4 << 4) | g4;
                                unsigned b8 = (b4 << 4) | b4;

                                tmpBuf[j * VIDEO_WIDTH + i] = (0xFF << 24) | (b8 << 16) | (g8 << 8) | (r8);
                            }
                        }

                        std::vector<unsigned char> png;
                        lodepng::State             state;
                        unsigned                   error = lodepng::encode(png, reinterpret_cast<uint8_t *>(tmpBuf.data()), VIDEO_WIDTH, VIDEO_HEIGHT * 2, state);
                        if (!error)
                            lodepng::save_file(png, pngFile);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Scaling: Nearest Neighbor", "", config->displayScaling == DisplayScaling::NearestNeighbor)) {
                    config->displayScaling = DisplayScaling::NearestNeighbor;
                }
                if (ImGui::MenuItem("Scaling: Linear", "", config->displayScaling == DisplayScaling::Linear)) {
                    config->displayScaling = DisplayScaling::Linear;
                }
                if (ImGui::MenuItem("Scaling: Integer", "", config->displayScaling == DisplayScaling::Integer)) {
                    config->displayScaling = DisplayScaling::Integer;
                }
                ImGui::Separator();
                ImGui::MenuItem("Font scale 2x", "", &config->fontScale2x);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Keyboard")) {
                ImGui::MenuItem("Cursor keys & F1-F6 emulate hand controller (ScrLk)", "", &config->handCtrlEmulation);
                if (ImGui::MenuItem("Paste text from clipboard", "")) {
                    emuState.typeInStr = platformIO.Platform_GetClipboardTextFn(ImGui::GetCurrentContext());
                }
                ImGui::Separator();
                for (int i = 0; i < (int)KeyLayout::Count; i++) {
                    char tmp[64];
                    snprintf(tmp, sizeof(tmp), "Keyboard layout: %s", Keyboard::instance()->getKeyLayoutName((KeyLayout)i).c_str());
                    if (ImGui::MenuItem(tmp, "", Keyboard::instance()->getKeyLayout() == (KeyLayout)i))
                        Keyboard::instance()->setKeyLayout((KeyLayout)i);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                ImGui::MenuItem("Enable debugger", "", &emuState.enableDebugger);
                if (emuState.enableDebugger) {
                    ImGui::MenuItem("Memory editor", "", &config->showMemEdit);
                    ImGui::MenuItem("CPU state", "", &config->showCpuState);
                    ImGui::MenuItem("IO Registers", "", &config->showIoRegsWindow);
                    ImGui::MenuItem("Breakpoints", "", &config->showBreakpoints);
                    ImGui::MenuItem("Assembly listing", "", &config->showAssemblyListing);
                    ImGui::MenuItem("CPU trace", "", &config->showCpuTrace);
                    ImGui::MenuItem("Watch", "", &config->showWatch);
                    ImGui::MenuItem("ESP info", "", &config->showEspInfo);
                    if (ImGui::MenuItem("Stop on HALT instruction", "", &config->stopOnHalt)) {
                        emuState.stopOnHalt = config->stopOnHalt;
                    }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Clear memory (0x00) & reset Aquarius+", "")) {
                        memset(emuState.screenRam, 0, sizeof(emuState.screenRam));
                        memset(emuState.colorRam, 0, sizeof(emuState.colorRam));
                        memset(emuState.mainRam, 0, sizeof(emuState.mainRam));
                        memset(emuState.videoRam, 0, sizeof(emuState.videoRam));
                        memset(emuState.charRam, 0, sizeof(emuState.charRam));
                        emuState.warmReset();
                    }
                    if (ImGui::MenuItem("Clear memory (0xA5) & reset Aquarius+", "")) {
                        memset(emuState.screenRam, 0xA5, sizeof(emuState.screenRam));
                        memset(emuState.colorRam, 0xA5, sizeof(emuState.colorRam));
                        memset(emuState.mainRam, 0xA5, sizeof(emuState.mainRam));
                        memset(emuState.videoRam, 0xA5, sizeof(emuState.videoRam));
                        memset(emuState.charRam, 0xA5, sizeof(emuState.charRam));
                        emuState.warmReset();
                    }

                    ImGui::Separator();
                    ImGui::Text("Emulation speed");
                    ImGui::SameLine();
                    ImGui::SliderInt("##speed", &emuState.emulationSpeed, 1, 20);
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

        if (emuState.emuMode == EmuState::Em_Halted) {
            config->showCpuState = true;
        }

        if (emuState.enableDebugger) {
            wndScreen(&emuState.enableDebugger);
        } else {
            allowTyping = true;
        }

        if (emuState.enableDebugger) {
            if (config->showMemEdit)
                wndMemEdit(&config->showMemEdit);
            if (config->showCpuState)
                wndCpuState(&config->showCpuState);
            if (config->showIoRegsWindow)
                wndIoRegs(&config->showIoRegsWindow);
            if (config->showBreakpoints)
                wndBreakpoints(&config->showBreakpoints);
            if (config->showAssemblyListing)
                wndAssemblyListing(&config->showAssemblyListing);
            if (config->showCpuTrace)
                wndCpuTrace(&config->showCpuTrace);
            if (config->showWatch)
                wndWatch(&config->showWatch);
            if (config->showEspInfo)
                wndEspInfo(&config->showEspInfo);
        }
        if (showAppAbout)
            wndAbout(&showAppAbout);
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        emuState.mouseHideTimeout -= io.DeltaTime;
        if (emuState.mouseHideTimeout < 0)
            emuState.mouseHideTimeout = 0;

        if (!emuState.enableDebugger) {
            auto dst = renderTexture((int)menuBarSize.y);

            if (!io.WantCaptureMouse) {
                // Update mouse
                const ImVec2 p0((float)dst.x, (float)dst.y);
                const ImVec2 p1((float)(dst.x + dst.w), (float)(dst.y + dst.h));
                auto         pos = (io.MousePos - p0) / (p1 - p0) * ImVec2(VIDEO_WIDTH / 2, VIDEO_HEIGHT) - ImVec2(16, 16);

                bool hideMouse =
                    (emuState.mouseHideTimeout > 0) &&
                    (pos.x >= -16 && pos.x < VIDEO_WIDTH - 16) &&
                    (pos.y >= -16 && pos.y < VIDEO_HEIGHT - 16);

                int mx = std::max(0, std::min((int)pos.x, 319));
                int my = std::max(0, std::min((int)pos.y, 199));

                uint8_t buttonMask =
                    (io.MouseDown[0] ? 1 : 0) |
                    (io.MouseDown[1] ? 2 : 0) |
                    (io.MouseDown[2] ? 4 : 0);

                auto aqp          = UartProtocol::instance();
                aqp->mouseX       = (float)mx;
                aqp->mouseY       = (float)my;
                aqp->mouseButtons = buttonMask;

                if (hideMouse)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            }
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        if (first) {
            ImGui::SetWindowFocus("Screen");
            first = false;
        }
    }
}

void UI::renderScreen() {
    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    const uint16_t *fb = emuState.video.getFb();

    for (int j = 0; j < VIDEO_HEIGHT * 2; j++) {
        for (int i = 0; i < VIDEO_WIDTH; i++) {
            // Convert from RGB444 to RGB888
            uint16_t col444 = fb[j / 2 * VIDEO_WIDTH + i];

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

SDL_Rect UI::calcRenderPos(int w, int h, int menuHeight) {
    auto config = Config::instance();

    int sw, sh;
    if (config->displayScaling == DisplayScaling::Integer && w >= VIDEO_WIDTH && h >= VIDEO_HEIGHT * 2) {
        // Retain aspect ratio
        int w1 = (w / VIDEO_WIDTH) * VIDEO_WIDTH;
        int h1 = (w1 * (VIDEO_HEIGHT * 2)) / VIDEO_WIDTH;
        int h2 = (h / (VIDEO_HEIGHT * 2)) * (VIDEO_HEIGHT * 2);
        int w2 = (h2 * VIDEO_WIDTH) / (VIDEO_HEIGHT * 2);

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
        SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);

    } else {
        float aspect = (float)VIDEO_WIDTH / (float)(VIDEO_HEIGHT * 2);

        sh = (int)((float)h);
        sw = (int)((float)sh * aspect);
        if (sw > w) {
            sw = (int)((float)w);
            sh = (int)((float)sw / aspect);
        }
        SDL_SetTextureScaleMode(texture, config->displayScaling == DisplayScaling::NearestNeighbor ? SDL_ScaleModeNearest : SDL_ScaleModeLinear);
    }

    SDL_Rect dst;
    dst.w = (int)sw;
    dst.h = (int)sh;
    dst.x = (w - dst.w) / 2;
    dst.y = menuHeight + (h - dst.h) / 2;
    return dst;
}

SDL_Rect UI::renderTexture(int menuHeight) {
    SDL_Rect dst;
    dst.x = 0;
    dst.y = 0;
    dst.w = 0;
    dst.h = 0;

    float rsx, rsy;
    SDL_RenderGetScale(renderer, &rsx, &rsy);
    auto  drawData = ImGui::GetDrawData();
    float scaleX   = (rsx == 1.0f) ? drawData->FramebufferScale.x : 1.0f;
    float scaleY   = (rsy == 1.0f) ? drawData->FramebufferScale.y : 1.0f;
    int   w        = (int)(drawData->DisplaySize.x * scaleX);
    int   h        = (int)(drawData->DisplaySize.y * scaleY) - menuHeight;
    if (w <= 0 || h <= 0)
        return dst;

    dst = calcRenderPos(w, h, menuHeight);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    return dst;
}

void UI::emulate() {
    // Emulation is performed in sync with the audio. This function will run
    // for the time needed to fill 1 audio buffer, which is about 1/60 of a
    // second.
    auto config = Config::instance();
    if (!emuState.enableDebugger) {
        // Always run when not debugging
        emuState.emuMode = EmuState::Em_Running;
    }

    // Get a buffer from audio subsystem.
    auto abuf = Audio::instance()->getBuffer();
    if (abuf == NULL) {
        // No buffer available, don't emulate for now.
        return;
    }
    memset(abuf, 0, SAMPLES_PER_BUFFER * 2 * 2);

    // Only play audio in running mode, otherwise keep zero
    if (emuState.emuMode != EmuState::Em_Running) {
        for (int aidx = 0; aidx < SAMPLES_PER_BUFFER; aidx++) {
            abuf[aidx * 2 + 0] = 0;
            abuf[aidx * 2 + 1] = 0;
        }
    }

    static bool dbgUpdateScreen = false;

    if (emuState.emuMode == EmuState::Em_Running) {
        dbgUpdateScreen = true;

        // Increase emulation speed while pasting text
        int emuSpeed = emuState.enableDebugger ? emuState.emulationSpeed : 1;
        if (!emuState.typeInStr.empty())
            emuSpeed = 16;

        // Render each audio sample
        for (int aidx = 0; aidx < SAMPLES_PER_BUFFER * emuSpeed; aidx++) {
            while (true) {
                auto flags = emuState.emulate();
                if (emuState.emuMode != EmuState::Em_Running)
                    break;

                if (flags & ERF_NEW_AUDIO_SAMPLE)
                    break;
            }
            if (emuState.emuMode != EmuState::Em_Running)
                break;

            if (config->enableSound && emuSpeed == 1) {
                float al = emuState.audioLeft / 65535.0f;
                float ar = emuState.audioRight / 65535.0f;
                float l  = dcBlockLeft.filter(al);
                float r  = dcBlockRight.filter(ar);
                l        = std::min(std::max(l, -1.0f), 1.0f);
                r        = std::min(std::max(r, -1.0f), 1.0f);

                abuf[aidx * 2 + 0] = (int16_t)(l * 32767.0f);
                abuf[aidx * 2 + 1] = (int16_t)(r * 32767.0f);
            }
        }
    }

    if (emuState.emuMode != EmuState::Em_Running) {
        emuState.haltAfterRet  = -1;
        emuState.tmpBreakpoint = -1;
        if (emuState.emuMode == EmuState::Em_Step) {
            dbgUpdateScreen  = true;
            emuState.emuMode = EmuState::Em_Halted;
            emuState.emulate();
        }

        if (dbgUpdateScreen) {
            dbgUpdateScreen = false;

            // Update screen
            int line = emuState.videoLine;
            for (int i = 0; i < 240; i++) {
                emuState.videoLine = i;
                emuState.video.drawLine();
            }
            emuState.videoLine = line;
        }
    }

    // Return buffer to audio subsystem.
    Audio::instance()->putBuffer(abuf);
}

void UI::wndAbout(bool *p_open) {
    if (ImGui::Begin("About Aquarius+ emulator", p_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
        extern const char *versionStr;
        ImGui::Text("Aquarius+ emulator version: %s", versionStr);
        ImGui::Separator();
        ImGui::Text("The Aquarius+ emulator is part of the Aquarius+ project.");
        ImGui::Text("Developed by Frank van den Hoef.");
        ImGui::Separator();
        ImGui::Text("Members of the Aquarius+ project team:");
        ImGui::Text("* Frank van den Hoef - Hardware / Firmware / Emulator");
        ImGui::Text("* Sean P. Harrington - Platform evangelist");
        ImGui::Text("* Curtis F. Kaylor   - plusBASIC");
        ImGui::Separator();
        ImGui::Text(
            "Thanks go out to all the people contributing to this\n"
            "project and those who enjoy playing with it, either with\n"
            "this emulator or the actual hardware!");
    }
    ImGui::End();
}

void UI::wndCpuState(bool *p_open) {
    bool open = ImGui::Begin("CPU state", p_open, ImGuiWindowFlags_AlwaysAutoResize);
    if (open) {
        ImGui::PushStyleColor(ImGuiCol_Button, emuState.emuMode == EmuState::Em_Halted ? (ImVec4)ImColor(192, 0, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        ImGui::BeginDisabled(emuState.emuMode != EmuState::Em_Running);
        ImGui::SetNextItemShortcut(ImGuiMod_Shift | ImGuiKey_F5, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
        if (ImGui::Button("Halt")) {
            emuState.emuMode = EmuState::Em_Halted;
        }
        ImGui::EndDisabled();
        ImGui::PopStyleColor();

        ImGui::BeginDisabled(emuState.emuMode == EmuState::Em_Running);
        ImGui::SameLine();

        ImGui::SetNextItemShortcut(ImGuiKey_F11, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
        if (ImGui::Button("Step Into")) {
            emuState.emuMode = EmuState::Em_Step;
        }
        ImGui::SameLine();

        ImGui::SetNextItemShortcut(ImGuiKey_F10, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
        if (ImGui::Button("Step Over")) {
            int tmpBreakpoint = -1;

            if (emuState.z80ctx.halted) {
                // Step over HALT instruction
                emuState.z80ctx.halted = 0;
                emuState.z80ctx.PC++;
            } else {
                uint8_t opcode = emuState.memRead(emuState.z80ctx.PC);
                if (opcode == 0xCD ||          // CALL nn
                    (opcode & 0xC7) == 0xC4) { // CALL c,nn

                    tmpBreakpoint = emuState.z80ctx.PC + 3;

                } else if ((opcode & 0xC7) == 0xC7) { // RST
                    tmpBreakpoint = emuState.z80ctx.PC + 1;
                    if ((opcode & 0x38) == 0x08 ||
                        (opcode & 0x38) == 0x30) {

                        // Skip one extra byte on RST 08H/30H, since on the Aq these
                        // system calls absorb the byte following this instruction.
                        tmpBreakpoint++;
                    }

                } else if (opcode == 0xED) {
                    opcode = emuState.memRead(emuState.z80ctx.PC + 1);
                    if (opcode == 0xB9 || // CPDR
                        opcode == 0xB1 || // CPIR
                        opcode == 0xBA || // INDR
                        opcode == 0xB2 || // INIR
                        opcode == 0xB8 || // LDDR
                        opcode == 0xB0 || // LDIR
                        opcode == 0xBB || // OTDR
                        opcode == 0xB3) { // OTIR
                    }
                    tmpBreakpoint = emuState.z80ctx.PC + 2;
                }
                emuState.tmpBreakpoint = tmpBreakpoint;
                if (tmpBreakpoint >= 0) {
                    emuState.emuMode = EmuState::Em_Running;
                } else {
                    emuState.emuMode = EmuState::Em_Step;
                }
            }
        }

        ImGui::SameLine();
        ImGui::SetNextItemShortcut(ImGuiMod_Shift | ImGuiKey_F10, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
        if (ImGui::Button("Step Out")) {
            emuState.haltAfterRet = 0;
            emuState.emuMode      = EmuState::Em_Running;
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, emuState.emuMode == EmuState::Em_Running ? (ImVec4)ImColor(0, 128, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        ImGui::SetNextItemShortcut(ImGuiKey_F5, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
        if (ImGui::Button("Go")) {
            emuState.emuMode = EmuState::Em_Running;
        }
        ImGui::PopStyleColor();
        ImGui::EndDisabled();

        ImGui::Separator();

        {
            uint16_t    addr = emuState.z80ctx.PC;
            std::string name;
            if (asmListing.findNearestSymbol(addr, name)) {
                ImGui::Text("%s ($%04X + %u)", name.c_str(), addr, emuState.z80ctx.PC - addr);
                ImGui::Separator();
            }
        }

        {
            char tmp1[64];
            char tmp2[64];
            emuState.z80ctx.tstates = 0;

            bool prevEnableBp          = emuState.enableBreakpoints;
            emuState.enableBreakpoints = false;
            Z80Debug(&emuState.z80ctx, tmp1, tmp2);
            emuState.enableBreakpoints = prevEnableBp;

            ImGui::Text("         %-12s %s", tmp1, tmp2);
        }
        ImGui::Separator();

        auto drawAddrVal = [&](const std::string &name, uint16_t val) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%-3s", name.c_str());
            ImGui::TableNextColumn();

            if (emuState.emuMode == EmuState::Em_Running) {
                ImGui::Text("%04X", val);
            } else {

                char addr[32];
                snprintf(addr, sizeof(addr), "%04X##%s", val, name.c_str());
                ImGui::Selectable(addr);
                addrPopup(val);
            }

            ImGui::TableNextColumn();

            uint8_t data[8];
            for (int i = 0; i < 8; i++)
                data[i] = emuState.memRead(val + i);
            ImGui::Text(
                "%02X %02X %02X %02X %02X %02X %02X %02X",
                emuState.memRead(val + 0),
                emuState.memRead(val + 1),
                emuState.memRead(val + 2),
                emuState.memRead(val + 3),
                emuState.memRead(val + 4),
                emuState.memRead(val + 5),
                emuState.memRead(val + 6),
                emuState.memRead(val + 7));

            ImGui::TableNextColumn();
            std::string str;
            for (int i = 0; i < 8; i++)
                str.push_back((data[i] >= 32 && data[i] <= 0x7E) ? data[i] : '.');
            ImGui::TextUnformatted(str.c_str());
        };

        auto drawAF = [](const std::string &name, uint16_t val) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%-3s", name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%04X", val);
            ImGui::TableNextColumn();

            auto a = val >> 8;
            auto f = val & 0xFF;

            ImGui::Text(
                "    %c %c %c %c %c %c %c %c", //      %c",
                (f & 0x80) ? 'S' : '-',
                (f & 0x40) ? 'Z' : '-',
                (f & 0x20) ? 'X' : '-',
                (f & 0x10) ? 'H' : '-',
                (f & 0x08) ? 'X' : '-',
                (f & 0x04) ? 'P' : '-',
                (f & 0x02) ? 'N' : '-',
                (f & 0x01) ? 'C' : '-');

            ImGui::TableNextColumn();
            ImGui::Text("%c", (a >= 32 && a <= 0x7E) ? a : '.');
        };

        if (ImGui::BeginTable("RegTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Reg", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Contents", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed);
            // ImGui::TableHeadersRow();

            drawAddrVal("PC", emuState.z80ctx.PC);
            drawAddrVal("SP", emuState.z80ctx.R1.wr.SP);
            drawAF("AF", emuState.z80ctx.R1.wr.AF);
            drawAddrVal("BC", emuState.z80ctx.R1.wr.BC);
            drawAddrVal("DE", emuState.z80ctx.R1.wr.DE);
            drawAddrVal("HL", emuState.z80ctx.R1.wr.HL);
            drawAddrVal("IX", emuState.z80ctx.R1.wr.IX);
            drawAddrVal("IY", emuState.z80ctx.R1.wr.IY);
            drawAF("AF'", emuState.z80ctx.R2.wr.AF);
            drawAddrVal("BC'", emuState.z80ctx.R2.wr.BC);
            drawAddrVal("DE'", emuState.z80ctx.R2.wr.DE);
            drawAddrVal("HL'", emuState.z80ctx.R2.wr.HL);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("IR");
            ImGui::TableNextColumn();
            ImGui::Text("%04X", (emuState.z80ctx.I << 8) | emuState.z80ctx.R);
            ImGui::TableNextColumn();
            ImGui::Text("IM %u  Interrupts %3s", emuState.z80ctx.IM, emuState.z80ctx.IFF1 ? "On" : "Off");
            ImGui::TableNextColumn();

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void UI::wndScreen(bool *p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = ImGui::Begin("Screen", p_open, ImGuiWindowFlags_None);
    ImGui::PopStyleVar();

    if (open) {
        if (texture) {
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            ImGui::InvisibleButton("##imgbtn", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

            auto dst = calcRenderPos((int)canvas_sz.x, (int)canvas_sz.y, 0);

            const ImVec2 p0(canvas_p0.x + dst.x, canvas_p0.y + dst.y);
            const ImVec2 p1(canvas_p0.x + dst.x + dst.w, canvas_p0.y + dst.y + dst.h);

            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddImage((ImTextureID)texture, p0, p1, {0, 0}, {1, 1});

            ImGuiIO &io  = ImGui::GetIO();
            auto     pos = (io.MousePos - p0) / (p1 - p0) * ImVec2(VIDEO_WIDTH / 2, VIDEO_HEIGHT) - ImVec2(16, 16);

            int mx = std::max(0, std::min((int)pos.x, 319));
            int my = std::max(0, std::min((int)pos.y, 199));

            uint8_t buttonMask =
                (io.MouseDown[0] ? 1 : 0) |
                (io.MouseDown[1] ? 2 : 0) |
                (io.MouseDown[2] ? 4 : 0);

            static bool dragging = false;
            bool        update   = false;

            if (ImGui::IsItemActive()) {
                dragging = true;
                update   = true;
            } else if (dragging) {
                update   = true;
                dragging = false;
            }
            if (ImGui::IsItemHovered()) {
                update = true;
                if (emuState.mouseHideTimeout > 0)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            }
            if (update) {
                auto aqp          = UartProtocol::instance();
                aqp->mouseX       = (float)mx;
                aqp->mouseY       = (float)my;
                aqp->mouseButtons = buttonMask;
                aqp->mouseWheel += (int)io.MouseWheel;
            }
        }
        allowTyping = ImGui::IsWindowFocused();
    }
    ImGui::End();
}

void UI::wndBreakpoints(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Breakpoints", p_open, 0)) {
        ImGui::Checkbox("Enable breakpoints", &emuState.enableBreakpoints);
        ImGui::SameLine(ImGui::GetWindowWidth() - 25);
        if (ImGui::Button("+")) {
            emuState.breakpoints.emplace_back();
        }
        ImGui::Separator();
        if (ImGui::BeginTable("Table", 8, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("En", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Symbol", 0);
            ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin((int)emuState.breakpoints.size());
            int eraseIdx = -1;

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    auto &bp = emuState.breakpoints[row_n];

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmtstr("##en%d", row_n).c_str(), &bp.enabled);
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 6);
                    ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &bp.addr, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite);
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::BeginCombo(fmtstr("##name%d", row_n).c_str(), bp.name.c_str())) {
                        for (auto &sym : asmListing.symbolsStrAddr) {
                            if (ImGui::Selectable(fmtstr("%04X %s", sym.second, sym.first.c_str()).c_str())) {
                                bp.name = sym.first;
                                bp.addr = sym.second;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmtstr("##onR%d", row_n).c_str(), &bp.onR);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmtstr("##onW%d", row_n).c_str(), &bp.onW);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmtstr("##onX%d", row_n).c_str(), &bp.onX);
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 9);

                    static const char *types[] = {"Mem", "IO 8", "IO 16"};
                    if (ImGui::BeginCombo(fmtstr("##type%d", row_n).c_str(), types[bp.type])) {
                        for (int i = 0; i < 3; i++) {
                            if (ImGui::Selectable(types[i])) {
                                bp.type = i;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Button(fmtstr("X##del%d", row_n).c_str())) {
                        eraseIdx = row_n;
                    }
                }
            }
            if (eraseIdx >= 0) {
                emuState.breakpoints.erase(emuState.breakpoints.begin() + eraseIdx);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

static ImU8 z80memRead(const ImU8 *data, size_t off) {
    (void)data;
    return emuState.memRead((uint16_t)off);
}
static void z80memWrite(ImU8 *data, size_t off, ImU8 d) {
    (void)data;
    emuState.memWrite((uint16_t)off, d);
}

void UI::wndMemEdit(bool *p_open) {
    struct MemoryArea {
        MemoryArea(const std::string &_name, void *_data, size_t _size)
            : name(_name), data(_data), size(_size) {
        }
        std::string name;
        void       *data;
        size_t      size;
    };
    static std::vector<MemoryArea> memAreas;

    if (memAreas.empty()) {
        memAreas.emplace_back("Z80 memory", nullptr, 0x10000);
        memAreas.emplace_back("Screen RAM", emuState.screenRam, sizeof(emuState.screenRam));
        memAreas.emplace_back("Color RAM", emuState.colorRam, sizeof(emuState.colorRam));
        memAreas.emplace_back("Page 19: Cartridge ROM", emuState.cartRom, sizeof(emuState.cartRom));
        memAreas.emplace_back("Page 20: Video RAM", emuState.videoRam, sizeof(emuState.videoRam));
        memAreas.emplace_back("Page 21: Character RAM", emuState.charRam, sizeof(emuState.charRam));

        for (int i = 32; i < 64; i++) {
            char tmp[256];
            snprintf(tmp, sizeof(tmp), "Page %d: Main RAM $%05X-$%05X", i, (i - 32) * 16384, ((i + 1) - 32) * 16384 - 1);
            memAreas.emplace_back(tmp, emuState.mainRam + (i - 32) * 16384, 16384);
        }
    }

    auto config = Config::instance();
    if (config->memEditMemSelect < 0 || config->memEditMemSelect > (int)memAreas.size()) {
        // Invalid setting, reset to 0
        config->memEditMemSelect = 0;
    }

    MemoryEditor::Sizes s;
    memEdit.calcSizes(s, memAreas[config->memEditMemSelect].size, 0);
    ImGui::SetNextWindowSize(ImVec2(s.windowWidth, s.windowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(s.windowWidth, 150.0f), ImVec2(s.windowWidth, FLT_MAX));

    if (ImGui::Begin("Memory editor", p_open, ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::BeginCombo("Memory select", memAreas[config->memEditMemSelect].name.c_str(), ImGuiComboFlags_HeightLargest)) {
            for (int i = 0; i < (int)memAreas.size(); i++) {
                if (ImGui::Selectable(memAreas[i].name.c_str(), config->memEditMemSelect == i)) {
                    config->memEditMemSelect = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();

        memEdit.readFn  = (config->memEditMemSelect == 0) ? z80memRead : nullptr;
        memEdit.writeFn = (config->memEditMemSelect == 0) ? z80memWrite : nullptr;
        memEdit.drawContents(memAreas[config->memEditMemSelect].data, memAreas[config->memEditMemSelect].size, 0);
        if (memEdit.contentsWidthChanged) {
            memEdit.calcSizes(s, memAreas[config->memEditMemSelect].size, 0);
            ImGui::SetWindowSize(ImVec2(s.windowWidth, ImGui::GetWindowSize().y));
        }
    }
    ImGui::End();
}

void UI::wndIoRegs(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(330, FLT_MAX));

    bool open = ImGui::Begin("IO Registers", p_open, 0);
    if (open) {
        if (ImGui::CollapsingHeader("Video")) {
            ImGui::Text("$E0     VCTRL   : $%02X", emuState.videoCtrl);
            ImGui::Text("$E1/$E2 VSCRX   : %u", emuState.videoScrX);
            ImGui::Text("$E3     VSCRY   : %u", emuState.videoScrY);
            ImGui::Text("$E4     VSPRSEL : %u", emuState.videoSprSel);
            ImGui::Text("$E5/$E6 VSPRX   : %u", emuState.videoSprX[emuState.videoSprSel]);
            ImGui::Text("$E7     VSPRY   : %u", emuState.videoSprY[emuState.videoSprSel]);
            ImGui::Text("$E8/$E9 VSPRIDX : %u", emuState.videoSprIdx[emuState.videoSprSel]);
            ImGui::Text("$E9     VSPRATTR: $%02X", emuState.videoSprAttr[emuState.videoSprSel]);
            ImGui::Text("$EA     VPALSEL : %u", emuState.videoPalSel);
            ImGui::Text("$EC     VLINE   : %u", emuState.videoLine);
            ImGui::Text("$ED     VIRQLINE: %u", emuState.videoIrqLine);
        }
        if (ImGui::CollapsingHeader("Interrupt")) {
            ImGui::Text("$EE IRQMASK: $%02X %s%s", emuState.irqMask, emuState.irqMask & 2 ? "[LINE]" : "", emuState.irqMask & 1 ? "[VBLANK]" : "");
            ImGui::Text("$EF IRQSTAT: $%02X %s%s", emuState.irqStatus, emuState.irqStatus & 2 ? "[LINE]" : "", emuState.irqStatus & 1 ? "[VBLANK]" : "");
        }
        if (ImGui::CollapsingHeader("Banking")) {
            ImGui::Text("$F0 BANK0: $%02X - page:%u%s%s", emuState.bankRegs[0], emuState.bankRegs[0] & 0x3F, emuState.bankRegs[0] & 0x80 ? " RO" : "", emuState.bankRegs[0] & 0x40 ? " OVL" : "");
            ImGui::Text("$F1 BANK1: $%02X - page:%u%s%s", emuState.bankRegs[1], emuState.bankRegs[1] & 0x3F, emuState.bankRegs[1] & 0x80 ? " RO" : "", emuState.bankRegs[1] & 0x40 ? " OVL" : "");
            ImGui::Text("$F2 BANK2: $%02X - page:%u%s%s", emuState.bankRegs[2], emuState.bankRegs[2] & 0x3F, emuState.bankRegs[2] & 0x80 ? " RO" : "", emuState.bankRegs[2] & 0x40 ? " OVL" : "");
            ImGui::Text("$F3 BANK3: $%02X - page:%u%s%s", emuState.bankRegs[3], emuState.bankRegs[3] & 0x3F, emuState.bankRegs[3] & 0x80 ? " RO" : "", emuState.bankRegs[3] & 0x40 ? " OVL" : "");
        }
        if (ImGui::CollapsingHeader("Key buffer")) {
            auto keyMode = Keyboard::instance()->getKeyMode();

            {
                uint8_t val = emuState.kbBufCnt == 0 ? 0 : emuState.kbBuf[emuState.kbBufRdIdx];
                ImGui::Text("$FA KEYBUF: $%02X (%c)", val, val > 32 && val < 127 ? val : '.');
            }
            ImGui::Text(
                "   KEYMODE: $%02X %s%s%s\n",
                keyMode,
                (keyMode & 1) ? "[Enable]" : "",
                (keyMode & 2) ? "[ASCII]" : "[ScanCode]",
                (keyMode & 4) ? "[Repeat]" : "");

            std::string str = "Key buffer: ";

            int rdIdx = emuState.kbBufRdIdx;
            for (int i = 0; i < emuState.kbBufCnt; i++) {
                if (keyMode & 2) {
                    uint8_t val = emuState.kbBuf[rdIdx];
                    str += fmtstr("%c", val > 32 && val < 127 ? val : '.');
                } else {
                    str += fmtstr("%02X ", emuState.kbBuf[rdIdx]);
                }
                rdIdx++;
                if (rdIdx == sizeof(emuState.kbBuf))
                    rdIdx = 0;
            }
            ImGui::Text("%s", str.c_str());
        }
        if (ImGui::CollapsingHeader("Other")) {
            uint8_t sysctrl =
                ((emuState.sysCtrlTurboUnlimited ? (1 << 3) : 0) |
                 (emuState.sysCtrlTurbo ? (1 << 2) : 0) |
                 (emuState.sysCtrlAyDisable ? (1 << 1) : 0) |
                 (emuState.sysCtrlDisableExt ? (1 << 0) : 0));
            ImGui::Text(
                "$FB SYSCTRL: $%02X %s%s%s%s", sysctrl,
                sysctrl & 8 ? "[UNLIMITED]" : "",
                sysctrl & 4 ? "[TURBO]" : "",
                sysctrl & 2 ? "[AYDIS]" : "",
                sysctrl & 1 ? "[EXTDIS]" : "");
        }
        if (ImGui::CollapsingHeader("Audio AY1")) {
            ImGui::Text(" 0 AFINE   : $%02X", emuState.ay1.regs[0]);
            ImGui::Text(" 1 ACOARSE : $%02X", emuState.ay1.regs[1]);
            ImGui::Text(" 2 BFINE   : $%02X", emuState.ay1.regs[2]);
            ImGui::Text(" 3 BCOARSE : $%02X", emuState.ay1.regs[3]);
            ImGui::Text(" 4 CFINE   : $%02X", emuState.ay1.regs[4]);
            ImGui::Text(" 5 CCOARSE : $%02X", emuState.ay1.regs[5]);
            ImGui::Text(" 6 NOISEPER: $%02X", emuState.ay1.regs[6]);
            ImGui::Text(" 7 ENABLE  : $%02X", emuState.ay1.regs[7]);
            ImGui::Text(" 8 AVOL    : $%02X", emuState.ay1.regs[8]);
            ImGui::Text(" 9 BVOL    : $%02X", emuState.ay1.regs[9]);
            ImGui::Text("10 CVOL    : $%02X", emuState.ay1.regs[10]);
            ImGui::Text("11 EAFINE  : $%02X", emuState.ay1.regs[11]);
            ImGui::Text("12 EACOARSE: $%02X", emuState.ay1.regs[12]);
            ImGui::Text("13 EASHAPE : $%02X", emuState.ay1.regs[13]);
            ImGui::Text("14 PORTA   : $%02X", emuState.handCtrl1);
            ImGui::Text("15 PORTB   : $%02X", emuState.handCtrl2);
        }
        if (ImGui::CollapsingHeader("Audio AY2")) {
            ImGui::Text(" 0 AFINE   : $%02X", emuState.ay2.regs[0]);
            ImGui::Text(" 1 ACOARSE : $%02X", emuState.ay2.regs[1]);
            ImGui::Text(" 2 BFINE   : $%02X", emuState.ay2.regs[2]);
            ImGui::Text(" 3 BCOARSE : $%02X", emuState.ay2.regs[3]);
            ImGui::Text(" 4 CFINE   : $%02X", emuState.ay2.regs[4]);
            ImGui::Text(" 5 CCOARSE : $%02X", emuState.ay2.regs[5]);
            ImGui::Text(" 6 NOISEPER: $%02X", emuState.ay2.regs[6]);
            ImGui::Text(" 7 ENABLE  : $%02X", emuState.ay2.regs[7]);
            ImGui::Text(" 8 AVOL    : $%02X", emuState.ay2.regs[8]);
            ImGui::Text(" 9 BVOL    : $%02X", emuState.ay2.regs[9]);
            ImGui::Text("10 CVOL    : $%02X", emuState.ay2.regs[10]);
            ImGui::Text("11 EAFINE  : $%02X", emuState.ay2.regs[11]);
            ImGui::Text("12 EACOARSE: $%02X", emuState.ay2.regs[12]);
            ImGui::Text("13 EASHAPE : $%02X", emuState.ay2.regs[13]);
        }
        if (ImGui::CollapsingHeader("Sprites")) {
            if (ImGui::BeginTable("Table", 10, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Tile", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("En", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Pri", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Pal", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("H16", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("VF", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("HF");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                clipper.Begin(64);
                while (clipper.Step()) {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%2d", row_n);
                        ImGui::TableNextColumn();
                        ImGui::Text("%3d", emuState.videoSprX[row_n]);
                        ImGui::TableNextColumn();
                        ImGui::Text("%3d", emuState.videoSprY[row_n]);
                        ImGui::TableNextColumn();
                        ImGui::Text("%3d", emuState.videoSprIdx[row_n]);
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted((emuState.videoSprAttr[row_n] & 0x80) ? "X" : "");
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted((emuState.videoSprAttr[row_n] & 0x40) ? "X" : "");
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", (emuState.videoSprAttr[row_n] >> 4) & 3);
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted((emuState.videoSprAttr[row_n] & 0x08) ? "X" : "");
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted((emuState.videoSprAttr[row_n] & 0x04) ? "X" : "");
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted((emuState.videoSprAttr[row_n] & 0x02) ? "X" : "");
                    }
                }
                ImGui::EndTable();
            }
        }
        if (ImGui::CollapsingHeader("Palette")) {
            if (ImGui::BeginTable("Table", 8, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Pal", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Idx", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("G", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Color");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                clipper.Begin(64);
                while (clipper.Step()) {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                        int r = (emuState.videoPalette[row_n] >> 8) & 0xF;
                        int g = (emuState.videoPalette[row_n] >> 4) & 0xF;
                        int b = (emuState.videoPalette[row_n] >> 0) & 0xF;

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%2d", row_n);
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", row_n / 16);
                        ImGui::TableNextColumn();
                        ImGui::Text("%2d", row_n & 15);
                        ImGui::TableNextColumn();
                        ImGui::Text("%03X", emuState.videoPalette[row_n]);
                        ImGui::TableNextColumn();
                        ImGui::Text("%2d", r);
                        ImGui::TableNextColumn();
                        ImGui::Text("%2d", g);
                        ImGui::TableNextColumn();
                        ImGui::Text("%2d", b);
                        ImGui::TableNextColumn();
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor((r << 4) | r, (g << 4) | g, (b << 4) | b)));
                    }
                }
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();
}

void UI::addrPopup(uint16_t addr) {
    if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft) ||
        ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight)) {

        auto sym = asmListing.symbolsAddrStr.find(addr);
        if (sym != asmListing.symbolsAddrStr.end()) {
            ImGui::Text("Address $%04X (%s)", addr, sym->second.c_str());
        } else {
            ImGui::Text("Address $%04X", addr);
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Run to here")) {
            emuState.tmpBreakpoint = addr;
            emuState.emuMode       = EmuState::Em_Running;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Add breakpoint")) {
            EmuState::Breakpoint bp;
            bp.enabled = true;
            bp.addr    = addr;
            bp.onR     = false;
            bp.onW     = false;
            bp.onX     = true;
            emuState.breakpoints.push_back(bp);
            ImGui::CloseCurrentPopup();
            listingReloaded();
        }
        if (ImGui::MenuItem("Show in memory editor")) {
            auto config              = Config::instance();
            config->showMemEdit      = true;
            config->memEditMemSelect = 0;
            memEdit.gotoAddr         = addr;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::BeginMenu("Add watch")) {
            int i = -1;

            if (ImGui::MenuItem("Hex 8"))
                i = (int)EmuState::WatchType::Hex8;
            if (ImGui::MenuItem("Dec U8"))
                i = (int)EmuState::WatchType::DecU8;
            if (ImGui::MenuItem("Dec S8"))
                i = (int)EmuState::WatchType::DecS8;
            if (ImGui::MenuItem("Hex 16"))
                i = (int)EmuState::WatchType::Hex16;
            if (ImGui::MenuItem("Dec U16"))
                i = (int)EmuState::WatchType::DecU16;
            if (ImGui::MenuItem("Dec S16"))
                i = (int)EmuState::WatchType::DecS16;

            if (i >= 0) {
                EmuState::Watch w;
                w.addr = addr;
                w.type = (EmuState::WatchType)i;
                emuState.watches.push_back(w);
                ImGui::CloseCurrentPopup();
                listingReloaded();
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

void UI::wndAssemblyListing(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));

    bool open = ImGui::Begin("Assembly listing", p_open, 0);
    if (open) {
        if (asmListing.lines.empty()) {
            if (ImGui::Button("Load zmac listing")) {
                char const *lFilterPatterns[1] = {"*.lst"};
                char       *lstFile            = tinyfd_openFileDialog("Open zmac listing file", "", 1, lFilterPatterns, "Zmac listing files", 0);
                if (lstFile) {
                    asmListing.load(lstFile);
                    listingReloaded();
                }
            }
        } else {
            if (ImGui::Button("Reload")) {
                auto path = asmListing.getPath();
                asmListing.load(path);
                listingReloaded();
            }
            ImGui::SameLine();
            if (ImGui::Button("X")) {
                asmListing.clear();
                listingReloaded();
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(asmListing.getPath().c_str());
        }
        ImGui::Separator();

        if (ImGui::BeginTable("Table", 5, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            static float itemsHeight  = -1;
            static int   lastPC       = -1;
            bool         updateScroll = (emuState.emuMode == EmuState::Em_Halted && lastPC != emuState.z80ctx.PC);
            if (updateScroll) {
                lastPC = emuState.z80ctx.PC;
            }

            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("LineNr", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Text");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin((int)asmListing.lines.size());

            while (clipper.Step()) {
                if (clipper.ItemsHeight > 0) {
                    itemsHeight = clipper.ItemsHeight;
                }

                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    auto &line = asmListing.lines[row_n];

                    ImGui::TableNextRow();
                    if (emuState.emuMode == EmuState::Em_Halted && emuState.z80ctx.PC == line.addr) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg]));
                        updateScroll = false;
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(line.file.c_str());
                    ImGui::TableNextColumn();
                    if (line.addr >= 0) {
                        char addr[32];
                        snprintf(addr, sizeof(addr), "%04X##%d", line.addr, row_n);
                        ImGui::Selectable(addr);
                        addrPopup(line.addr);
                    }

                    // ImGui::Text("%04X", line.addr);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(line.bytes.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%6d", line.linenr);
                    ImGui::TableNextColumn();

                    {
                        auto commentStart = line.s.find(";");
                        if (commentStart != line.s.npos) {
                            ImGui::TextUnformatted(line.s.substr(0, commentStart).c_str());
                            ImGui::SameLine(0, 0);
                            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.4f, 1.0f), "%s", line.s.substr(commentStart).c_str());
                        } else {
                            ImGui::TextUnformatted(line.s.c_str());
                        }
                    }
                }
            }

            if (updateScroll) {
                for (unsigned i = 0; i < asmListing.lines.size(); i++) {
                    const auto &line = asmListing.lines[i];

                    if (line.addr >= 0 && line.addr == emuState.z80ctx.PC) {
                        ImGui::SetScrollY(std::max(0.0f, itemsHeight * (i - 5)));
                        break;
                    }
                }
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void UI::wndCpuTrace(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(700, 200), ImVec2(FLT_MAX, FLT_MAX));
    bool open = ImGui::Begin("CPU trace", p_open, 0);
    if (open) {
        ImGui::Checkbox("Enable tracing", &emuState.traceEnable);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 8);

        const int minDepth = 16, maxDepth = 16384;
        ImGui::DragInt("Trace depth", &emuState.traceDepth, 1, minDepth, maxDepth);
        emuState.traceDepth = std::max(minDepth, std::min(emuState.traceDepth, maxDepth));

        ImGui::Separator();

        if (ImGui::BeginTable("Table", 15, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Instruction", 0);
            ImGui::TableSetupColumn("SP", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("AF", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("BC", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("DE", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("HL", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("IX", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("IY", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("AF'", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("BC'", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("DE'", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("HL'", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin((int)emuState.cpuTrace.size());

            auto regColumn = [](uint16_t value, uint16_t prevValue) {
                ImGui::TableNextColumn();

                if (prevValue != value)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor(0, 128, 0)));

                ImGui::Text("%04X", value);
            };

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    auto &entry     = emuState.cpuTrace[row_n];
                    auto &prevEntry = row_n < 1 ? entry : emuState.cpuTrace[row_n - 1];

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%4d", row_n - (emuState.traceDepth - 1));
                    ImGui::TableNextColumn();
                    ImGui::Text("%04X", entry.pc);
                    ImGui::TableNextColumn();
                    ImGui::Text("%-11s", entry.bytes + 1);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entry.instrStr);

                    regColumn(entry.r1.wr.SP, prevEntry.r1.wr.SP);
                    regColumn(entry.r1.wr.AF, prevEntry.r1.wr.AF);
                    regColumn(entry.r1.wr.BC, prevEntry.r1.wr.BC);
                    regColumn(entry.r1.wr.DE, prevEntry.r1.wr.DE);
                    regColumn(entry.r1.wr.HL, prevEntry.r1.wr.HL);
                    regColumn(entry.r1.wr.IX, prevEntry.r1.wr.IX);
                    regColumn(entry.r1.wr.IY, prevEntry.r1.wr.IY);
                    regColumn(entry.r2.wr.AF, prevEntry.r2.wr.AF);
                    regColumn(entry.r2.wr.BC, prevEntry.r2.wr.BC);
                    regColumn(entry.r2.wr.DE, prevEntry.r2.wr.DE);
                    regColumn(entry.r2.wr.HL, prevEntry.r2.wr.HL);
                    // ImGui::TextUnformatted(line.file.c_str());
                }
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void UI::wndWatch(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Watch", p_open, 0)) {
        if (ImGui::BeginTable("Table", 5, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Symbol", 0);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            int              numWatches = (int)emuState.watches.size();
            clipper.Begin(numWatches + 1);
            int eraseIdx = -1;

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    ImGui::TableNextRow();

                    if (row_n < numWatches) {
                        auto &w = emuState.watches[row_n];
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 7);
                        switch (w.type) {
                            case EmuState::WatchType::Hex8: {
                                uint8_t val = emuState.memRead(w.addr);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U8, &val, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    emuState.memWrite(w.addr, val);
                                }
                                break;
                            }
                            case EmuState::WatchType::DecU8: {
                                uint8_t val = emuState.memRead(w.addr);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U8, &val, nullptr, nullptr, "%u", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    emuState.memWrite(w.addr, val);
                                }
                                break;
                            }
                            case EmuState::WatchType::DecS8: {
                                int8_t val = emuState.memRead(w.addr);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_S8, &val, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    emuState.memWrite(w.addr, val);
                                }
                                break;
                            }
                            case EmuState::WatchType::Hex16: {
                                uint16_t val = emuState.memRead(w.addr) | (emuState.memRead(w.addr + 1) << 8);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &val, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    emuState.memWrite(w.addr, val & 0xFF);
                                    emuState.memWrite(w.addr + 1, (val >> 8) & 0xFF);
                                }
                                break;
                            }
                            case EmuState::WatchType::DecU16: {
                                uint16_t val = emuState.memRead(w.addr) | (emuState.memRead(w.addr + 1) << 8);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &val, nullptr, nullptr, "%u", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    emuState.memWrite(w.addr, val & 0xFF);
                                    emuState.memWrite(w.addr + 1, (val >> 8) & 0xFF);
                                }
                                break;
                            }
                            case EmuState::WatchType::DecS16: {
                                int16_t val = emuState.memRead(w.addr) | (emuState.memRead(w.addr + 1) << 8);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_S16, &val, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    emuState.memWrite(w.addr, val & 0xFF);
                                    emuState.memWrite(w.addr + 1, (val >> 8) & 0xFF);
                                }
                                break;
                            }
                        }
                        // ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &w.value, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite);
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 6);
                        if (ImGui::InputScalar(fmtstr("##addr%d", row_n).c_str(), ImGuiDataType_U16, &w.addr, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            auto sym = asmListing.symbolsAddrStr.find(w.addr);
                            if (sym != asmListing.symbolsAddrStr.end()) {
                                w.name = sym->second;
                            } else {
                                w.name = "";
                            }
                        }
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(-1);
                        if (ImGui::BeginCombo(fmtstr("##name%d", row_n).c_str(), w.name.c_str())) {
                            for (auto &sym : asmListing.symbolsStrAddr) {
                                if (ImGui::Selectable(fmtstr("%04X %s", sym.second, sym.first.c_str()).c_str())) {
                                    w.name = sym.first;
                                    w.addr = sym.second;
                                }
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 11);

                        static const char *types[] = {"Hex 8", "Dec U8", "Dec S8", "Hex 16", "Dec U16", "Dec S16"};
                        if (ImGui::BeginCombo(fmtstr("##type%d", row_n).c_str(), types[(int)w.type])) {
                            for (int i = 0; i < 6; i++) {
                                if (ImGui::Selectable(types[i])) {
                                    w.type = (EmuState::WatchType)i;
                                }
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(fmtstr("X##del%d", row_n).c_str())) {
                            eraseIdx = row_n;
                        }
                    } else {
                        ImGui::TableNextColumn();
                        ImGui::TableNextColumn();
                        ImGui::TableNextColumn();
                        ImGui::TableNextColumn();
                        ImGui::TableNextColumn();
                        if (ImGui::Button(fmtstr("+##add%d", row_n).c_str())) {
                            emuState.watches.emplace_back();
                        }
                    }
                }
            }
            if (eraseIdx >= 0) {
                emuState.watches.erase(emuState.watches.begin() + eraseIdx);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void UI::wndEspInfo(bool *p_open) {
    bool open = ImGui::Begin("ESP info", p_open, 0);
    if (open) {
        ImGui::SeparatorText("Current path");
        auto &curPath = UartProtocol::instance()->currentPath;
        ImGui::Text("%s", curPath.empty() ? "/" : curPath.c_str());
        ImGui::SeparatorText("File descriptors");
        if (ImGui::BeginTable("Table", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (auto &entry : UartProtocol::instance()->fi) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%u", entry.first);
                ImGui::TableNextColumn();
                ImGui::Text("$%02X", entry.second.flags);
                ImGui::TableNextColumn();
                ImGui::Text("%s", entry.second.name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%u", entry.second.offset);
            }

            ImGui::EndTable();
        }
        ImGui::SeparatorText("Directory descriptors");
        if (ImGui::BeginTable("Table2", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (auto &entry : UartProtocol::instance()->di) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%u", entry.first);
                ImGui::TableNextColumn();
                ImGui::Text("%s", entry.second.name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%u", entry.second.offset);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void UI::listingReloaded() {
    // Update watches
    for (auto &w : emuState.watches) {
        if (!asmListing.findSymbolAddr(w.name, w.addr)) {
            if (!asmListing.findSymbolName(w.addr, w.name)) {
                w.name = "";
            }
        }
    }

    // Update breakpoints
    for (auto &bp : emuState.breakpoints) {
        if (!asmListing.findSymbolAddr(bp.name, bp.addr)) {
            if (!asmListing.findSymbolName(bp.addr, bp.name)) {
                bp.name = "";
            }
        }
    }
}
