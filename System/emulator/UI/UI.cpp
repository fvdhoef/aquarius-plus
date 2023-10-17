#include "UI.h"
#include <SDL.h>

#include "EmuState.h"

#include "Video.h"
#include "AqKeyboard.h"
#include "AqUartProtocol.h"
#include "SDCardVFS.h"

#define IMGUI_DEFINE_MATH_OPERATORS
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

    bool escapePressed = false;

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
                        emuState.reset();
                        break;
                    }

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

        // Safe-guard for misbehaving video drivers that don't lock on v-sync
        auto ticks = SDL_GetTicks64();
        if (ticks - lastKeyRepeatCall > 16) {
            lastKeyRepeatCall = ticks;
            AqKeyboard::instance().repeatTimer();
        }

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
                ImGui::MenuItem("Enable mouse", "", &config.enableMouse);
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
                ImGui::Separator();
                for (int i = 0; i < (int)KeyLayout::Count; i++) {
                    char tmp[64];
                    snprintf(tmp, sizeof(tmp), "Keyboard layout: %s", getKeyLayoutName((KeyLayout)i).c_str());
                    if (ImGui::MenuItem(tmp, "", getKeyLayout() == (KeyLayout)i))
                        setKeyLayout((KeyLayout)i);
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
                ImGui::MenuItem("CPU trace", "", &config.showCpuTrace);
                ImGui::MenuItem("ESP info", "", &config.showEspInfo);
                ImGui::Separator();

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

                ImGui::Separator();
                {
                    if (ImGui::MenuItem("Emulation speed 1x", "", emuState.emulationSpeed == 1))
                        emuState.emulationSpeed = 1;
                    if (ImGui::MenuItem("Emulation speed 2x", "", emuState.emulationSpeed == 2))
                        emuState.emulationSpeed = 2;
                    if (ImGui::MenuItem("Emulation speed 4x", "", emuState.emulationSpeed == 4))
                        emuState.emulationSpeed = 4;
                    if (ImGui::MenuItem("Emulation speed 8x", "", emuState.emulationSpeed == 8))
                        emuState.emulationSpeed = 8;
                    if (ImGui::MenuItem("Emulation speed 16x", "", emuState.emulationSpeed == 16))
                        emuState.emulationSpeed = 16;
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
            config.showCpuState = true;
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
        if (config.showCpuTrace)
            wndCpuTrace(&config.showCpuTrace);
        if (config.showEspInfo)
            wndEspInfo(&config.showEspInfo);
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

        if (!config.showScreenWindow) {
            auto dst = renderTexture();

            if (!io.WantCaptureMouse) {
                // Update mouse
                const ImVec2 p0((float)dst.x, (float)dst.y);
                const ImVec2 p1((float)(dst.x + dst.w), (float)(dst.y + dst.h));
                auto         pos = (io.MousePos - p0) / (p1 - p0) * ImVec2(VIDEO_WIDTH, VIDEO_HEIGHT) - ImVec2(16, 16);

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

                emuState.mouseX       = mx;
                emuState.mouseY       = my;
                emuState.mouseButtons = buttonMask;
                if (hideMouse)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            }
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

SDL_Rect UI::renderTexture() {
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
    int   h        = (int)(drawData->DisplaySize.y * scaleY);
    if (w <= 0 || h <= 0)
        return dst;

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

    dst.w = (int)sw;
    dst.h = (int)sh;
    dst.x = (w - dst.w) / 2;
    dst.y = (h - dst.h) / 2;
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    return dst;
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
        int emuSpeed = emuState.typeInStr.empty() ? emuState.emulationSpeed : 16;
        emuSpeed     = emuState.emulationSpeed;

        // Render each audio sample
        for (int aidx = 0; aidx < SAMPLES_PER_BUFFER * emuSpeed; aidx++) {
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

            if (config.enableSound && emuState.emulationSpeed == 1) {
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

            renderScreen();
        }
    }

    // Return buffer to audio subsystem.
    Audio::instance().putBuffer(abuf);
}

void UI::wndAbout(bool *p_open) {
    if (ImGui::Begin("About Aquarius+ emulator", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        extern const char *versionStr;
        ImGui::Text("Aquarius+ emulator version: %s", versionStr);
        ImGui::Separator();
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
        ImGui::BeginDisabled(emuState.emuMode != EmuState::Em_Running);
        if (ImGui::Button("Halt")) {
            emuState.emuMode = EmuState::Em_Halted;
        }
        ImGui::EndDisabled();
        ImGui::PopStyleColor();

        ImGui::BeginDisabled(emuState.emuMode == EmuState::Em_Running);
        ImGui::SameLine();
        if (ImGui::Button("Step Into")) {
            emuState.emuMode = EmuState::Em_Step;
        }
        ImGui::SameLine();

        if (ImGui::Button("Step Over")) {
            char tmp1[64];
            char tmp2[64];
            emuState.z80ctx.tstates = 0;
            Z80Debug(&emuState.z80ctx, tmp1, tmp2);

            unsigned instLen = (unsigned)strlen(tmp1) / 3;
            bool     isCall  = (strncmp(tmp2, "CALL ", 5) == 0);
            bool     isRst   = (strncmp(tmp2, "RST ", 4) == 0);

            if (isCall || isRst) {
                emuState.tmpBreakpoint = emuState.z80ctx.PC + instLen;

                if (strncmp(tmp2, "RST 8H", 6) == 0 ||
                    strncmp(tmp2, "RST 30H", 7) == 0) {

                    // Skip one extra byte on RST 08H/30H, since on the Aq these
                    // system calls absorb the byte following this instruction.
                    emuState.tmpBreakpoint++;
                }

                emuState.emuMode = EmuState::Em_Running;
            } else {
                emuState.emuMode = EmuState::Em_Step;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Step Out")) {
            emuState.haltAfterRet = 0;
            emuState.emuMode      = EmuState::Em_Running;
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, emuState.emuMode == EmuState::Em_Running ? (ImVec4)ImColor(0, 128, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        if (ImGui::Button("Go")) {
            emuState.emuMode = EmuState::Em_Running;
        }
        ImGui::PopStyleColor();
        ImGui::EndDisabled();

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
            auto a  = val >> 8;
            auto ch = (a >= 32 && a <= 0x7E) ? a : '.';

            ImGui::Text(
                "%-3s %04X      %c %c %c %c %c %c %c %c      %c",
                name.c_str(), val,
                (val & 0x80) ? 'S' : '-',
                (val & 0x40) ? 'Z' : '-',
                (val & 0x20) ? 'X' : '-',
                (val & 0x10) ? 'H' : '-',
                (val & 0x08) ? 'X' : '-',
                (val & 0x04) ? 'P' : '-',
                (val & 0x02) ? 'N' : '-',
                (val & 0x01) ? 'C' : '-',
                ch);
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
            // ImGui::SameLine();
            // ImGui::Text("%3d %3d %d\n", emuState.mouseX, emuState.mouseY, emuState.mouseButtons);

            config.scrScale = e;

            if (texture) {
                ImGuiIO &io = ImGui::GetIO();

                auto sz = ImVec2((float)(VIDEO_WIDTH * e), (float)(VIDEO_HEIGHT * e));
                ImGui::InvisibleButton("##imgbtn", sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
                const ImVec2 p0  = ImGui::GetItemRectMin();
                const ImVec2 p1  = ImGui::GetItemRectMax();
                auto         pos = (io.MousePos - p0) / (p1 - p0) * ImVec2(VIDEO_WIDTH, VIDEO_HEIGHT) - ImVec2(16, 16);

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
                    emuState.mouseX       = mx;
                    emuState.mouseY       = my;
                    emuState.mouseButtons = buttonMask;
                }

                ImDrawList *draw_list = ImGui::GetWindowDrawList();
                draw_list->AddImage(texture, p0, p1, {0, 0}, {1, 1});
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

    auto &config = Config::instance();
    if (config.memEditMemSelect < 0 || config.memEditMemSelect > (int)memAreas.size()) {
        // Invalid setting, reset to 0
        config.memEditMemSelect = 0;
    }

    MemoryEditor::Sizes s;
    memEdit.calcSizes(s, memAreas[config.memEditMemSelect].size, 0);
    ImGui::SetNextWindowSize(ImVec2(s.windowWidth, s.windowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(s.windowWidth, 150.0f), ImVec2(s.windowWidth, FLT_MAX));

    if (ImGui::Begin("Memory editor", p_open, ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::BeginCombo("Memory select", memAreas[config.memEditMemSelect].name.c_str(), ImGuiComboFlags_HeightLargest)) {
            for (int i = 0; i < (int)memAreas.size(); i++) {
                if (ImGui::Selectable(memAreas[i].name.c_str(), config.memEditMemSelect == i)) {
                    config.memEditMemSelect = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();

        memEdit.readFn  = (config.memEditMemSelect == 0) ? z80memRead : nullptr;
        memEdit.writeFn = (config.memEditMemSelect == 0) ? z80memWrite : nullptr;
        memEdit.drawContents(memAreas[config.memEditMemSelect].data, memAreas[config.memEditMemSelect].size, 0);
        if (memEdit.contentsWidthChanged) {
            memEdit.calcSizes(s, memAreas[config.memEditMemSelect].size, 0);
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
            auto keyMode = getKeyMode();

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
                }
            }
        } else {
            if (ImGui::Button("X")) {
                asmListing.clear();
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
                                bp.onR     = false;
                                bp.onW     = false;
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

void UI::wndEspInfo(bool *p_open) {
    bool open = ImGui::Begin("ESP info", p_open, 0);
    if (open) {
        ImGui::SeparatorText("Current path");
        auto &curPath = AqUartProtocol::instance().currentPath;
        ImGui::Text("%s", curPath.empty() ? "/" : curPath.c_str());
        ImGui::SeparatorText("File descriptors");
        if (ImGui::BeginTable("Table", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (auto &entry : AqUartProtocol::instance().fi) {
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

            for (auto &entry : AqUartProtocol::instance().di) {
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
