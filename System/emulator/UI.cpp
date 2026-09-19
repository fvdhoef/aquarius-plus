#include "UI.h"
#include <SDL.h>

#include "EmuState.h"
#include "Audio.h"
#include "Midi.h"
#include "AssemblyListing.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "MemoryEditor.h"

#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "tinyfiledialogs.h"
#include "Config.h"

#include "lodepng.h"
#include "FpgaCore.h"

extern "C" void app_main(void);

class UIInt : public UI {
public:
    SDL_Texture        *texture       = nullptr;
    int                 textureWidth  = 0;
    int                 textureHeight = 0;
    SDL_Window         *window        = nullptr;
    SDL_Renderer       *renderer      = nullptr;
    SDL_GameController *gameCtrl      = nullptr;
    int                 gameCtrlIdx   = -1;
    GamePadData         gamePadData;
    bool                allowTyping    = false;
    bool                first          = true;
    int                 emulationSpeed = 1;
    bool                escapePressed  = false;
    ImVec2              menuBarSize;

    void start(const std::string &typeInStr) override {
        auto config = Config::instance();

        memset(&gamePadData, 0, sizeof(gamePadData));
        setSDCardPath(config->sdCardPath);

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
            SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
            exit(1);
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

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        auto ctx                       = ImGui::CreateContext();
        ctx->ConfigNavWindowingKeyNext = 0;
        ctx->ConfigNavWindowingKeyPrev = 0;

        ImGuiIO &io    = ImGui::GetIO();
        io.IniFilename = nullptr; // imguiIniFileName.c_str();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        ImGui::LoadIniSettingsFromMemory(config->imguiConf.c_str());

        // Initialize emulator
        Midi::instance()->init();
        Audio::instance()->init();
        Audio::instance()->start();

        // Run main loop
        FPGA::instance()->init();
        FreeRtosMock_init();
        app_main();
        {
            auto emuState = EmuState::get();
            if (emuState)
                emuState->pasteText(typeInStr);
        }
        mainLoop();
        FreeRtosMock_deinit();
        EmuState::loadCore("");

        // Deinitialize
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void onKey(SDL_Scancode scancode, uint16_t mod, bool keyDown, bool isRepeat) {
        if (isRepeat)
            return;

        if (scancode == SDL_SCANCODE_ESCAPE) {
            escapePressed = keyDown;
        }
        // We decode CTRL-ESCAPE in this weird way to allow the sequence ESCAPE and then CTRL to be used on Windows.
        if (escapePressed && keyDown && (mod & KMOD_LCTRL)) {
            auto emuState = EmuState::get();
            if (emuState) {
                if ((mod & KMOD_LSHIFT) != 0) {
                    esp_restart();
                } else {
                    emuState->reset(false);
                }
            }
            return;
        }

        // Don't pass keypresses to emulator when ImGUI has keyboard focus
        if (ImGui::GetIO().WantCaptureKeyboard)
            return;

        if (allowTyping && scancode <= 255) {
            Keyboard::instance()->handleScancode(scancode, keyDown);
        }
    }

    void forceWindowsOnScreen() {
        auto dispSize = ImGui::GetIO().DisplaySize;
        for (auto wnd : GImGui->Windows) {
            if (wnd->Hidden || wnd->IsFallbackWindow ||
                (wnd->Flags & (ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_ChildWindow)))
                continue;

            auto size = ImMin(wnd->Size, dispSize);
            auto pos  = ImClamp(wnd->Pos, ImVec2(0, menuBarSize.y), dispSize - size);

            if (pos != wnd->Pos)
                ImGui::SetWindowPos(wnd, pos);
            if (size != wnd->Size)
                ImGui::SetWindowSize(wnd, size);
        }
    }

    void mainLoop() {
        ImGuiIO &io         = ImGui::GetIO();
        auto    &platformIO = ImGui::GetPlatformIO();
        auto     config     = Config::instance();

        bool showAppAbout   = false;
        bool showDemoWindow = false;

        int tooSlow = 0;

        // listingReloaded();

        bool done = false;
        while (!done) {
            auto emuState = EmuState::get();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                switch (event.type) {
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        onKey(event.key.keysym.scancode, event.key.keysym.mod, event.type == SDL_KEYDOWN, event.key.repeat != 0);
                        break;

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

                            memset(&gamePadData, 0, sizeof(gamePadData));
                            auto fpgaCore = FpgaCore::get();
                            if (fpgaCore)
                                fpgaCore->gamepadReport(0, gamePadData);
                        }
                        break;
                    }
                    case SDL_CONTROLLERDEVICEREMOVED: {
                        if (gameCtrlIdx == event.cdevice.which) {
                            SDL_GameControllerClose(gameCtrl);
                            gameCtrl    = nullptr;
                            gameCtrlIdx = -1;

                            memset(&gamePadData, 0, sizeof(gamePadData));
                            auto fpgaCore = FpgaCore::get();
                            if (fpgaCore)
                                fpgaCore->gamepadReport(0, gamePadData);
                        }
                        break;
                    }

                    case SDL_CONTROLLERAXISMOTION: {
                        switch (event.caxis.axis) {
                            case SDL_CONTROLLER_AXIS_LEFTX: gamePadData.lx = event.caxis.value / 256; break;
                            case SDL_CONTROLLER_AXIS_LEFTY: gamePadData.ly = event.caxis.value / 256; break;
                            case SDL_CONTROLLER_AXIS_RIGHTX: gamePadData.rx = event.caxis.value / 256; break;
                            case SDL_CONTROLLER_AXIS_RIGHTY: gamePadData.ry = event.caxis.value / 256; break;
                            case SDL_CONTROLLER_AXIS_TRIGGERLEFT: gamePadData.lt = event.caxis.value / 128; break;
                            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: gamePadData.rt = event.caxis.value / 128; break;
                        }

                        auto fpgaCore = FpgaCore::get();
                        if (fpgaCore)
                            fpgaCore->gamepadReport(0, gamePadData);
                        break;
                    }
                    case SDL_CONTROLLERBUTTONDOWN:
                    case SDL_CONTROLLERBUTTONUP: {
                        if (event.cbutton.button < 16) {
                            gamePadData.buttons = (gamePadData.buttons & ~(1 << event.cbutton.button)) | ((event.cbutton.state & 1) << event.cbutton.button);

                            auto fpgaCore = FpgaCore::get();
                            if (fpgaCore)
                                fpgaCore->gamepadReport(0, gamePadData);
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
                    if (emulationSpeed > 1) {
                        emulationSpeed--;
                    }
                }

                for (int i = 0; i < bufsToRender; i++) {
                    auto abuf = Audio::instance()->getBuffer();
                    assert(abuf != nullptr);
                    memset(abuf, 0, SAMPLES_PER_BUFFER * 2 * 2);

                    if (emuState) {
                        emuState->setDebuggerEnabled(config->enableDebugger);

                        // Increase emulation speed while pasting text
                        int emuSpeed = config->enableDebugger ? emulationSpeed : 1;
                        if (!emuState->pasteIsDone())
                            emuSpeed = 16;

                        bool enableSound = config->enableSound;
                        if (emuSpeed != 1)
                            enableSound = false;

                        for (int i = 0; i < emuSpeed; i++)
                            emuState->emulateFrame(enableSound ? abuf : nullptr, SAMPLES_PER_BUFFER);
                    }

                    Audio::instance()->putBuffer(abuf);
                }

                // Update screen
                if (emuState) {
                    int w, h;

                    emuState->getVideoSize(w, h);
                    if (w != textureWidth || h != textureHeight) {
                        if (texture) {
                            SDL_DestroyTexture(texture);
                        }
                        textureWidth  = w;
                        textureHeight = h;
                        texture       = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
                    }
                    if (texture) {
                        void *pixels;
                        int   pitch;
                        SDL_LockTexture(texture, NULL, &pixels, &pitch);
                        emuState->getPixels(pixels, pitch);
                        SDL_UnlockTexture(texture);
                    }
                }
            }

            if (io.WantSaveIniSettings) {
                config->imguiConf      = ImGui::SaveIniSettingsToMemory();
                io.WantSaveIniSettings = false;
            }

            io.FontGlobalScale = config->fontScale2x ? 2.0f : 1.0f;

            forceWindowsOnScreen();

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

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
                    if (emuState)
                        emuState->fileMenu();
                    ImGui::MenuItem("Enable sound", "", &config->enableSound);
                    ImGui::MenuItem("Enable mouse", "", &config->enableMouse);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset Aquarius+ (warm)", "")) {
                        if (emuState)
                            emuState->reset(false);
                    }
                    if (ImGui::MenuItem("Reset Aquarius+ (cold)", "")) {
                        if (emuState)
                            emuState->reset(true);
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
                        if (path && emuState) {
                            std::string pngFile = path;
                            if (pngFile.size() < 4 || pngFile.substr(pngFile.size() - 4) != ".png")
                                pngFile += ".png";

                            int w, h;
                            emuState->getVideoSize(w, h);

                            std::vector<uint32_t> tmpBuf;
                            tmpBuf.resize(w * h);
                            if (emuState)
                                emuState->getPixels(tmpBuf.data(), w * sizeof(uint32_t));

                            std::vector<unsigned char> png;
                            lodepng::State             state;
                            unsigned                   error = lodepng::encode(png, reinterpret_cast<uint8_t *>(tmpBuf.data()), w, h, state);
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
                    ImGui::MenuItem("UI font scale 2x", "", &config->fontScale2x);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Keyboard")) {
                    if (ImGui::MenuItem("Paste text from clipboard", "") && emuState) {
                        emuState->pasteText(platformIO.Platform_GetClipboardTextFn(ImGui::GetCurrentContext()));
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
                    ImGui::MenuItem("Enable debugger", "", &config->enableDebugger);
                    if (config->enableDebugger) {
                        ImGui::Separator();
                        if (emuState) {
                            emuState->dbgMenu();
                            ImGui::Separator();
                        }
                        ImGui::MenuItem("ESP info", "", &config->showEspInfo);
                        ImGui::Separator();
                        ImGui::Text("Emulation speed");
                        ImGui::SameLine();
                        ImGui::SliderInt("##speed", &emulationSpeed, 1, 20);
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

            if (config->enableDebugger) {
                wndScreen(&config->enableDebugger);
            } else {
                allowTyping = true;
            }

            if (emuState)
                emuState->dbgWindows();
            if (config->enableDebugger && config->showEspInfo)
                wndEspInfo(&config->showEspInfo);

            if (showAppAbout)
                wndAbout(&showAppAbout);
            if (showDemoWindow)
                ImGui::ShowDemoWindow(&showDemoWindow);

            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (!config->enableDebugger) {
                auto dst = renderTexture((int)menuBarSize.y);

                if (!io.WantCaptureMouse) {
                    // Update mouse
                    const ImVec2 p0((float)dst.x, (float)dst.y);
                    const ImVec2 p1((float)(dst.x + dst.w), (float)(dst.y + dst.h));
                    auto         pos = (io.MousePos - p0) / (p1 - p0) * ImVec2((float)textureWidth, (float)textureHeight);

                    uint8_t buttonMask =
                        (io.MouseDown[0] ? 1 : 0) |
                        (io.MouseDown[1] ? 2 : 0) |
                        (io.MouseDown[2] ? 4 : 0);

                    FpgaCore::get()->mouseReport((int)pos.x, (int)pos.y, buttonMask, (int)io.MouseWheel, true);
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

    SDL_Rect calcRenderPos(int w, int h, int menuHeight) {
        auto config = Config::instance();

        int sw, sh;
        if (config->displayScaling == DisplayScaling::Integer && w >= textureWidth && h >= textureHeight) {
            // Retain aspect ratio
            int w1 = (w / textureWidth) * textureWidth;
            int h1 = (w1 * textureHeight) / textureWidth;
            int h2 = (h / textureHeight) * textureHeight;
            int w2 = (h2 * textureWidth) / textureHeight;

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
            float aspect = (float)textureWidth / (float)textureHeight;

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

    SDL_Rect renderTexture(int menuHeight) {
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

    void wndAbout(bool *p_open) {
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

    void wndScreen(bool *p_open) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));
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
                auto     pos = (io.MousePos - p0) / (p1 - p0) * ImVec2((float)textureWidth, (float)textureHeight);

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
                }
                if (update) {
                    auto core = FpgaCore::get();
                    if (core) {
                        core->mouseReport((int)pos.x, (int)pos.y, buttonMask, (int)io.MouseWheel, true);
                    }
                }
            }
            allowTyping = ImGui::IsWindowFocused();
        }
        ImGui::End();
    }

    void wndEspInfo(bool *p_open) {
        bool open = ImGui::Begin("ESP info", p_open, 0);
        if (open) {
            ImGui::SeparatorText("Current path");
            auto curPath = VFSContext::getDefault()->getCurrentPath();
            ImGui::Text("%s", curPath.empty() ? "/" : curPath.c_str());
            ImGui::SeparatorText("File descriptors");
            if (ImGui::BeginTable("Table", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                for (auto &entry : VFSContext::getDefault()->fi) {
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

                for (auto &entry : VFSContext::getDefault()->di) {
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
};

UI *UI::instance() {
    static UIInt obj;
    return &obj;
}
