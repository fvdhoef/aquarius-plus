#include "UI.h"
#include <SDL.h>

#include "EmuState.h"

#include "EmuState.h"

#include "Video.h"
#include "AqKeyboard.h"
#include "AqUartProtocol.h"
#include "SDCardVFS.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "MemoryEditor.h"
#include "tinyfiledialogs.h"
#include "Config.h"

UI::UI() {
}

void UI::start(
    const std::string &romPath,
    const std::string &cartRomPath,
    const std::string &typeInStr) {

    auto &config = Config::instance();

    emuState.typeInStr = typeInStr;
    AqUartProtocol::instance().init();
    SDCardVFS::instance().init(config.sdCardPath);

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
    window = SDL_CreateWindow("Aquarius+ emulator", config.wndPosX, config.wndPosY, config.wndWidth, config.wndHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
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
    io.IniFilename = nullptr; // imguiIniFileName.c_str();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ImGui::LoadIniSettingsFromMemory(config.imguiConf.c_str());

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
    ImGuiIO &io     = ImGui::GetIO();
    auto    &config = Config::instance();

    bool showAppAbout   = false;
    bool showDemoWindow = false;

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

                    if (allowTyping) {
                        if (!event.key.repeat && event.key.keysym.scancode <= 255) {
                            AqKeyboard::instance().handleScancode(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
                            AqKeyboard::instance().updateMatrix();
                        }
                    }
                    break;
                }

                case SDL_WINDOWEVENT: {
                    if (event.window.event == SDL_WINDOWEVENT_MOVED) {
                        config.wndPosX = event.window.data1;
                        config.wndPosY = event.window.data2;

                    } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        config.wndWidth  = event.window.data1;
                        config.wndHeight = event.window.data2;
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

        if (io.WantSaveIniSettings) {
            config.imguiConf       = ImGui::SaveIniSettingsToMemory();
            io.WantSaveIniSettings = false;
        }

        AqKeyboard::instance().setScrollLock(config.handCtrlEmulation);

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("System")) {
                if (ImGui::MenuItem("Select SD card directory...", "")) {
                    auto path = tinyfd_selectFolderDialog("Select SD card directory", nullptr);
                    if (path) {
                        config.sdCardPath = path;
                        stripTrailingSlashes(config.sdCardPath);
                        SDCardVFS::instance().init(config.sdCardPath);
                    }
                }
                std::string ejectLabel = "Eject SD card";
                if (!config.sdCardPath.empty()) {
                    ejectLabel += " (" + config.sdCardPath + ")";
                }
                if (ImGui::MenuItem(ejectLabel.c_str(), "", false, !config.sdCardPath.empty())) {
                    config.sdCardPath.clear();
                    SDCardVFS::instance().init("");
                }
                ImGui::Separator();
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
                ImGui::MenuItem("Enable sound", "", &config.enableSound);
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
                ImGui::MenuItem("Cursor keys & F1-F6 emulate hand controller (ScrLk)", "", &config.handCtrlEmulation);
                if (ImGui::MenuItem("Paste text from clipboard", "")) {
                    emuState.typeInStr = io.GetClipboardTextFn(nullptr);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                ImGui::MenuItem("Screen in window", "", &config.showScreenWindow);
                ImGui::MenuItem("Memory editor", "", &config.showMemEdit);
                ImGui::MenuItem("CPU State", "", &config.showCpuState);
                ImGui::MenuItem("IO Registers", "", &config.showIoRegsWindow);
                ImGui::MenuItem("Breakpoints", "", &config.showBreakpoints);
                ImGui::MenuItem("Assembly listing", "", &config.showAssemblyListing);

                if (ImGui::MenuItem("Clear memory (0x00) & reset Aquarius+", "")) {
                    memset(emuState.screenRam, 0, sizeof(emuState.screenRam));
                    memset(emuState.colorRam, 0, sizeof(emuState.colorRam));
                    memset(emuState.mainRam, 0, sizeof(emuState.mainRam));
                    memset(emuState.videoRam, 0, sizeof(emuState.videoRam));
                    memset(emuState.charRam, 0, sizeof(emuState.charRam));
                    emuState.reset();
                }
                if (ImGui::MenuItem("Clear memory (0xA5) & reset Aquarius+", "")) {
                    memset(emuState.screenRam, 0xA5, sizeof(emuState.screenRam));
                    memset(emuState.colorRam, 0xA5, sizeof(emuState.colorRam));
                    memset(emuState.mainRam, 0xA5, sizeof(emuState.mainRam));
                    memset(emuState.videoRam, 0xA5, sizeof(emuState.videoRam));
                    memset(emuState.charRam, 0xA5, sizeof(emuState.charRam));
                    emuState.reset();
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

        if (config.showScreenWindow) {
            wndScreen(&config.showScreenWindow);
        } else {
            allowTyping = true;
        }
        if (config.showMemEdit)
            wndMemEdit(&config.showMemEdit);
        if (config.showCpuState)
            wndCpuState(&config.showCpuState);
        if (config.showIoRegsWindow)
            wndIoRegs(&config.showIoRegsWindow);
        if (config.showBreakpoints)
            wndBreakpoints(&config.showBreakpoints);
        if (config.showAssemblyListing)
            wndAssemblyListing(&config.showAssemblyListing);
        if (showAppAbout)
            wndAbout(&showAppAbout);
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!config.showScreenWindow) {
            renderTexture();
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

        if (first)
            ImGui::SetWindowFocus("Screen");

        first = false;
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
    auto &config = Config::instance();

    // Get a buffer from audio subsystem.
    auto abuf = Audio::instance().getBuffer();
    if (abuf == NULL) {
        // No buffer available, don't emulate for now.
        return;
    }

    // Only play audio in running mode, otherwise keep zero
    if (emuState.emuMode != EmuState::Em_Running) {
        for (int aidx = 0; aidx < SAMPLES_PER_BUFFER; aidx++) {
            abuf[aidx * 2 + 0] = 0;
            abuf[aidx * 2 + 1] = 0;
        }
    }

    if (emuState.emuMode == EmuState::Em_Running) {
        // Render each audio sample
        for (int aidx = 0; aidx < SAMPLES_PER_BUFFER; aidx++) {
            while (true) {
                auto flags = emuState.emulate();
                if (emuState.emuMode != EmuState::Em_Running)
                    break;

                if (flags & ERF_RENDER_SCREEN)
                    renderScreen();
                if (flags & ERF_NEW_AUDIO_SAMPLE)
                    break;
            }
            if (emuState.emuMode != EmuState::Em_Running)
                break;

            float al = emuState.audioLeft / 65535.0f;
            float ar = emuState.audioRight / 65535.0f;
            float l  = dcBlockLeft.filter(al);
            float r  = dcBlockRight.filter(ar);
            l        = std::min(std::max(l, -1.0f), 1.0f);
            r        = std::min(std::max(r, -1.0f), 1.0f);

            if (!config.enableSound) {
                l = 0;
                r = 0;
            }
            abuf[aidx * 2 + 0] = (int16_t)(l * 32767.0f);
            abuf[aidx * 2 + 1] = (int16_t)(r * 32767.0f);
        }
    }

    if (emuState.emuMode != EmuState::Em_Running) {
        emuState.tmpBreakpoint = -1;
        if (emuState.emuMode == EmuState::Em_Step) {
            emuState.emuMode = EmuState::Em_Halted;
            emuState.emulate();
        }
        renderScreen();
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

void UI::wndCpuState(bool *p_open) {
    bool open = ImGui::Begin("CPU state", p_open, ImGuiWindowFlags_AlwaysAutoResize);
    if (open) {
        ImGui::PushStyleColor(ImGuiCol_Button, emuState.emuMode == EmuState::Em_Halted ? (ImVec4)ImColor(192, 0, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        if (ImGui::Button("Halt")) {
            emuState.emuMode = EmuState::Em_Halted;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Step")) {
            emuState.emuMode = EmuState::Em_Step;
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, emuState.emuMode == EmuState::Em_Running ? (ImVec4)ImColor(0, 128, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        if (ImGui::Button("Go")) {
            emuState.emuMode = EmuState::Em_Running;
        }
        ImGui::PopStyleColor();

        ImGui::Separator();
        {
            char tmp1[64];
            char tmp2[64];
            emuState.z80ctx.tstates = 0;
            Z80Debug(&emuState.z80ctx, tmp1, tmp2);
            ImGui::Text("         %-12s %s", tmp1, tmp2);
        }
        ImGui::Separator();

        auto drawAddrVal = [](const std::string &name, uint16_t val) {
            uint8_t data[8];
            for (int i = 0; i < 8; i++)
                data[i] = emuState.memRead(0, val + i);

            auto str = fmtstr("%-3s %04X ", name.c_str(), val);
            for (int i = 0; i < 8; i++)
                str += fmtstr(" %02X", data[i]);
            str += "  ";
            for (int i = 0; i < 8; i++)
                str.push_back((data[i] >= 32 && data[i] <= 0x7E) ? data[i] : '.');
            ImGui::TextUnformatted(str.c_str());
        };

        auto drawAF = [](const std::string &name, uint16_t val) {
            ImGui::Text(
                "%-3s %04X      %c %c %c %c %c %c %c %c",
                name.c_str(), val,
                (val & 0x80) ? 'S' : '-',
                (val & 0x40) ? 'Z' : '-',
                (val & 0x20) ? 'X' : '-',
                (val & 0x10) ? 'H' : '-',
                (val & 0x08) ? 'X' : '-',
                (val & 0x04) ? 'P' : '-',
                (val & 0x02) ? 'N' : '-',
                (val & 0x01) ? 'C' : '-');
        };

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
        ImGui::Text(
            "IR  %04X  IM %u  Interrupts %3s",
            (emuState.z80ctx.I << 8) | emuState.z80ctx.R,
            emuState.z80ctx.IM,
            emuState.z80ctx.IFF1 ? "On" : "Off");
    }
    ImGui::End();
}

void UI::wndScreen(bool *p_open) {
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = ImGui::Begin("Screen", p_open, ImGuiWindowFlags_AlwaysAutoResize);
    // ImGui::PopStyleVar();

    if (open) {
        auto &config = Config::instance();

        if (open) {
            static int e = 0;
            if (e <= 0) {
                e = config.scrScale;
                e = std::min(std::max(config.scrScale, 1), 4);
            }

            ImGui::RadioButton("1x", &e, 1);
            ImGui::SameLine();
            ImGui::RadioButton("2x", &e, 2);
            ImGui::SameLine();
            ImGui::RadioButton("3x", &e, 3);
            ImGui::SameLine();
            ImGui::RadioButton("4x", &e, 4);
            config.scrScale = e;

            if (texture) {
                ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2((float)(VIDEO_WIDTH * e), (float)(VIDEO_HEIGHT * e)));
            }
            allowTyping = ImGui::IsWindowFocused();
        }
    }
    ImGui::End();
}

void UI::wndBreakpoints(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(330, FLT_MAX));
    if (ImGui::Begin("Breakpoints", p_open, 0)) {
        ImGui::Checkbox("Enable breakpoints", &emuState.enableBreakpoints);
        ImGui::SameLine(ImGui::GetWindowWidth() - 35);
        if (ImGui::Button("+")) {
            emuState.breakpoints.emplace_back();
        }

        int eraseIdx = -1;
        for (int i = 0; i < (int)emuState.breakpoints.size(); i++) {
            auto &bp = emuState.breakpoints[i];
            ImGui::PushID(i);

            ImGui::Separator();

            if (emuState.lastBp == i) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(255, 110, 0, 144));
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 110, 0, 200));
            }

            ImGui::Checkbox("En", &bp.enabled);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 6);
            ImGui::InputScalar("##", ImGuiDataType_U16, &bp.value, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite);
            ImGui::SameLine();
            ImGui::Checkbox("R", &bp.onR);
            ImGui::SameLine();
            ImGui::Checkbox("W", &bp.onW);
            ImGui::SameLine();

            if (bp.type != 0)
                ImGui::BeginDisabled();
            ImGui::Checkbox("X", &bp.onX);
            if (bp.type != 0)
                ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 10);
            ImGui::Combo(
                "##Type", &bp.type,
                "Mem\0"
                "IO 8\0"
                "IO 16\0");
            ImGui::SameLine();
            if (ImGui::Button("X##Delete")) {
                eraseIdx = i;
            }

            if (emuState.lastBp == i) {
                ImGui::PopStyleColor(2);
            }

            ImGui::PopID();
        }
        if (eraseIdx >= 0) {
            emuState.breakpoints.erase(emuState.breakpoints.begin() + eraseIdx);
        }
    }
    ImGui::End();
}

static ImU8 z80memRead(const ImU8 *data, size_t off) {
    (void)data;
    return emuState.memRead(0, (uint16_t)off);
}
static void z80memWrite(ImU8 *data, size_t off, ImU8 d) {
    (void)data;
    emuState.memWrite(0, (uint16_t)off, d);
}

void UI::wndMemEdit(bool *p_open) {
    static MemoryEditor memEdit;
    static int          itemCurrent = 0;

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
        memAreas.emplace_back("Page  0: System ROM $0000-$3FFF", emuState.systemRom + 0, 16384);
        memAreas.emplace_back("Page  1: System ROM $4000-$7FFF", emuState.systemRom + 16384, 16384);
        memAreas.emplace_back("Page  2: System ROM $8000-$9FFF", emuState.systemRom + 32768, 8192);
        memAreas.emplace_back("Page 19: Cartridge ROM", emuState.cartRom, sizeof(emuState.cartRom));
        memAreas.emplace_back("Page 20: Video RAM", emuState.videoRam, sizeof(emuState.videoRam));
        memAreas.emplace_back("Page 21: Character RAM", emuState.charRam, sizeof(emuState.charRam));

        for (int i = 32; i < 64; i++) {
            char tmp[256];
            snprintf(tmp, sizeof(tmp), "Page %d: Main RAM $%05X-$%05X", i, (i - 32) * 16384, ((i + 1) - 32) * 16384 - 1);
            memAreas.emplace_back(tmp, emuState.mainRam + (i - 32) * 16384, 16384);
        }
    }

    MemoryEditor::Sizes s;
    memEdit.calcSizes(s, memAreas[itemCurrent].size, 0);
    ImGui::SetNextWindowSize(ImVec2(s.windowWidth, s.windowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(s.windowWidth, 150.0f), ImVec2(s.windowWidth, FLT_MAX));

    if (ImGui::Begin("Memory editor", p_open, ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::BeginCombo("Memory select", memAreas[itemCurrent].name.c_str(), ImGuiComboFlags_HeightLargest)) {
            for (int i = 0; i < (int)memAreas.size(); i++) {
                if (ImGui::Selectable(memAreas[i].name.c_str(), itemCurrent == i)) {
                    itemCurrent = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();

        memEdit.readFn  = (itemCurrent == 0) ? z80memRead : nullptr;
        memEdit.writeFn = (itemCurrent == 0) ? z80memWrite : nullptr;
        memEdit.drawContents(memAreas[itemCurrent].data, memAreas[itemCurrent].size, 0);
        if (memEdit.contentsWidthChanged) {
            memEdit.calcSizes(s, memAreas[itemCurrent].size, 0);
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
    }
    ImGui::End();
}

void UI::wndAssemblyListing(bool *p_open) {
    bool open = ImGui::Begin("Assembly listing", p_open, 0);
    if (open) {
        if (ImGui::Button("Load zmac listing")) {
            char const *lFilterPatterns[1] = {"*.lst"};
            char       *lstFile            = tinyfd_openFileDialog("Open zmac listing file", "", 1, lFilterPatterns, "Zmac listing files", 0);
            asmListing.load(lstFile);
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(asmListing.getPath().c_str());
        ImGui::Separator();

        if (ImGui::BeginTable("Table", 5, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("LineNr", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Text");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(asmListing.lines.size());

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    auto &line = asmListing.lines[row_n];

                    ImGui::TableNextRow();
                    if (emuState.z80ctx.PC == line.addr) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg]));
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(line.file.c_str());
                    ImGui::TableNextColumn();
                    if (line.addr >= 0) {
                        char addr[10];
                        snprintf(addr, sizeof(addr), "%04X##%d", line.addr, row_n);
                        ImGui::Selectable(addr);
                        if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
                            ImGui::Text("Address $%04X", line.addr);
                            ImGui::Separator();
                            if (ImGui::MenuItem("Run to here")) {
                                emuState.tmpBreakpoint = line.addr;
                                emuState.emuMode       = EmuState::Em_Running;
                                ImGui::CloseCurrentPopup();
                            }
                            if (ImGui::MenuItem("Add breakpoint")) {
                                EmuState::Breakpoint bp;
                                bp.enabled = true;
                                bp.value   = line.addr;
                                bp.onX     = true;
                                emuState.breakpoints.push_back(bp);
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }
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
            ImGui::EndTable();
        }
    }
    ImGui::End();
}
