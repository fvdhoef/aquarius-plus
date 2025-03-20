#include "UI.h"
#include "Common.h"
#include "EmuState.h"
#include <SDL.h>
#include <chrono>
#include <thread>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "tinyfiledialogs.h"
#include "Config.h"

UI::UI() {
}

void UI::renderScreen() {
    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    emuState.video.render(pixels, pitch);
    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);
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

void UI::start(const std::string &romPath) {
    auto &config = Config::instance();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    // Load boot ROM
    emuState.loadRom(romPath);

    // Create window
    window = SDL_CreateWindow(
        "Aquarius32 emulator",
        config.wndPosX, config.wndPosY, config.wndWidth, config.wndHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Create renderer
    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED)) == NULL) {
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ImGui::LoadIniSettingsFromMemory(config.imguiConf.c_str());

    // Scale font on high DPI displays
    {
        float hdpi;
        if (SDL_GetDisplayDPI(0, NULL, &hdpi, NULL) == 0 && hdpi > 160.0f) {
            io.FontGlobalScale = 2;
        }
    }

    // Initialize emulator
    emuState.video.init();

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

    renderScreen();

    uint64_t prevTime = 0;

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
                            emuState.keybScanCode(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
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

                default:
                    if (event.type == SDL_QUIT)
                        done = true;
                    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                        done = true;
                    break;
            }
        }

        // Try to maintain 60Hz
        static Uint64 frequency = SDL_GetPerformanceFrequency();
        Uint64        curTime   = SDL_GetPerformanceCounter();
        auto          tDelta    = ((double)(curTime - prevTime) / frequency);
        if (tDelta < 0.016666) {
            std::this_thread::sleep_for(std::chrono::microseconds((unsigned)((0.016666 - tDelta) * 1000000.0f)));
        }
        prevTime = curTime;

        // Emulate
        {
            emuState.emulate();
            renderScreen();
        }

        if (io.WantSaveIniSettings) {
            config.imguiConf       = ImGui::SaveIniSettingsToMemory();
            io.WantSaveIniSettings = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImVec2 menuBarSize;
        if (ImGui::BeginMainMenuBar()) {
            menuBarSize = ImGui::GetWindowSize();

            if (ImGui::BeginMenu("System")) {
                // if (ImGui::MenuItem("Select SD card directory...", "")) {
                //     auto path = tinyfd_selectFolderDialog("Select SD card directory", nullptr);
                //     if (path) {
                //         config.sdCardPath = path;
                //         stripTrailingSlashes(config.sdCardPath);
                //         SDCardVFS::instance().init(config.sdCardPath);
                //     }
                // }
                // std::string ejectLabel = "Eject SD card";
                // if (!config.sdCardPath.empty()) {
                //     ejectLabel += " (" + config.sdCardPath + ")";
                // }
                // if (ImGui::MenuItem(ejectLabel.c_str(), "", false, !config.sdCardPath.empty())) {
                //     config.sdCardPath.clear();
                //     SDCardVFS::instance().init("");
                // }
                // ImGui::Separator();
                ImGui::MenuItem("Enable sound", "", &config.enableSound);
                ImGui::MenuItem("Enable mouse", "", &config.enableMouse);
                ImGui::Separator();
                if (ImGui::MenuItem("Reset System", "")) {
                    emuState.reset();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "")) {
                    done = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Screen")) {
                if (ImGui::MenuItem("Scaling: Nearest Neighbor", "", config.displayScaling == DisplayScaling::NearestNeighbor)) {
                    config.displayScaling = DisplayScaling::NearestNeighbor;
                }
                if (ImGui::MenuItem("Scaling: Linear", "", config.displayScaling == DisplayScaling::Linear)) {
                    config.displayScaling = DisplayScaling::Linear;
                }
                if (ImGui::MenuItem("Scaling: Integer", "", config.displayScaling == DisplayScaling::Integer)) {
                    config.displayScaling = DisplayScaling::Integer;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                ImGui::MenuItem("Enable debugger", "", &emuState.enableDebugger);
                if (emuState.enableDebugger) {
                    ImGui::MenuItem("Memory editor", "", &config.showMemEdit);
                    ImGui::MenuItem("CPU state", "", &config.showCpuState);
                    ImGui::MenuItem("IO Registers", "", &config.showIoRegsWindow);
                    ImGui::MenuItem("Breakpoints", "", &config.showBreakpoints);
                    // ImGui::MenuItem("Assembly listing", "", &config.showAssemblyListing);
                    // ImGui::MenuItem("CPU trace", "", &config.showCpuTrace);
                    ImGui::MenuItem("Watch", "", &config.showWatch);
                    // ImGui::Separator();
                    // ImGui::Text("Emulation speed");
                    // ImGui::SameLine();
                    // ImGui::SliderInt("##speed", &emuState.emulationSpeed, 1, 20);
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

        if (emuState.enableDebugger) {
            wndScreen(&emuState.enableDebugger);

            if (config.showCpuState)
                wndCpuState(&config.showCpuState);
            if (config.showMemEdit)
                wndMemEdit(&config.showMemEdit);
            if (config.showIoRegsWindow)
                wndIoRegs(&config.showIoRegsWindow);
            if (config.showBreakpoints)
                wndBreakpoints(&config.showBreakpoints);
            if (config.showWatch)
                wndWatch(&config.showWatch);

        } else {
            emuState.emuMode = EmuState::Em_Running;
            allowTyping      = true;
        }

        if (showAppAbout)
            wndAbout(&showAppAbout);
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!emuState.enableDebugger) {
            auto dst = renderTexture((int)menuBarSize.y);
            (void)dst;

            // if (!io.WantCaptureMouse) {
            //     // Update mouse
            //     const ImVec2 p0((float)dst.x, (float)dst.y);
            //     const ImVec2 p1((float)(dst.x + dst.w), (float)(dst.y + dst.h));
            //     auto         pos = (io.MousePos - p0) / (p1 - p0) * ImVec2(VIDEO_WIDTH, VIDEO_HEIGHT) - ImVec2(16, 16);

            //     bool hideMouse =
            //         (emuState.mouseHideTimeout > 0) &&
            //         (pos.x >= -16 && pos.x < VIDEO_WIDTH - 16) &&
            //         (pos.y >= -16 && pos.y < VIDEO_HEIGHT - 16);

            //     int mx = std::max(0, std::min((int)pos.x, 319));
            //     int my = std::max(0, std::min((int)pos.y, 199));

            //     uint8_t buttonMask =
            //         (io.MouseDown[0] ? 1 : 0) |
            //         (io.MouseDown[1] ? 2 : 0) |
            //         (io.MouseDown[2] ? 4 : 0);

            //     emuState.mouseX       = mx;
            //     emuState.mouseY       = my;
            //     emuState.mouseButtons = buttonMask;
            //     if (hideMouse)
            //         ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            // }
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

        if (first)
            ImGui::SetWindowFocus("Screen");

        first = false;
    }
}

SDL_Rect UI::calcRenderPos(int w, int h, int menuHeight) {
    auto &config = Config::instance();

    int sw, sh;
    if (config.displayScaling == DisplayScaling::Integer && w >= VIDEO_WIDTH && h >= VIDEO_HEIGHT) {
        // Retain aspect ratio
        int w1 = (w / VIDEO_WIDTH) * VIDEO_WIDTH;
        int h1 = (w1 * VIDEO_HEIGHT) / VIDEO_WIDTH;
        int h2 = (h / VIDEO_HEIGHT) * VIDEO_HEIGHT;
        int w2 = (h2 * VIDEO_WIDTH) / VIDEO_HEIGHT;

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
        float aspect = (float)VIDEO_WIDTH / (float)VIDEO_HEIGHT;

        sh = (int)((float)h);
        sw = (int)((float)sh * aspect);
        if (sw > w) {
            sw = (int)((float)w);
            sh = (int)((float)sw / aspect);
        }
        SDL_SetTextureScaleMode(texture, config.displayScaling == DisplayScaling::NearestNeighbor ? SDL_ScaleModeNearest : SDL_ScaleModeLinear);
    }

    SDL_Rect dst;
    dst.w = (int)sw;
    dst.h = (int)sh;
    dst.x = (w - dst.w) / 2;
    dst.y = menuHeight + (h - dst.h) / 2;
    return dst;
}

void UI::wndScreen(bool *p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = ImGui::Begin("Screen", p_open, ImGuiWindowFlags_None);
    ImGui::PopStyleVar();

    if (open) {
        if (texture) {
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            if (canvas_sz.x > 0 && canvas_sz.y > 0) {
                ImGui::InvisibleButton("##imgbtn", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

                auto dst = calcRenderPos((int)canvas_sz.x, (int)canvas_sz.y, 0);

                const ImVec2 p0(canvas_p0.x + dst.x, canvas_p0.y + dst.y);
                const ImVec2 p1(canvas_p0.x + dst.x + dst.w, canvas_p0.y + dst.y + dst.h);

                ImDrawList *draw_list = ImGui::GetWindowDrawList();
                draw_list->AddImage(texture, p0, p1, {0, 0}, {1, 1});

                if (ImGui::IsItemHovered()) {
                    ImGuiIO &io = ImGui::GetIO();
                    // emuState.usbHost.mouse(
                    //     (int)io.MouseDelta.x,
                    //     (int)io.MouseDelta.y,
                    //     (int)io.MouseWheel,
                    //     (io.MouseDown[0] ? 1 : 0) | (io.MouseDown[1] ? 2 : 0) | (io.MouseDown[2] ? 4 : 0));
                }
            }
        }
        allowTyping = ImGui::IsWindowFocused();
    }
    ImGui::End();
}

void UI::wndAbout(bool *p_open) {
    if (ImGui::Begin("About Aquarius32 emulator", p_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
        extern const char *versionStr;
        ImGui::Text("Aquarius32 emulator version: %s", versionStr);
        ImGui::Separator();
        ImGui::Text("Developed by Frank van den Hoef.");
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
#if 0
        ImGui::SameLine();
        if (ImGui::Button("Step Over")) {
            // int tmpBreakpoint = -1;

            // if (emuState.z80ctx.halted) {
            //     // Step over HALT instruction
            //     emuState.z80ctx.halted = 0;
            //     emuState.z80ctx.PC++;
            // } else {
            //     uint8_t opcode = emuState.memRead(emuState.z80ctx.PC);
            //     if (opcode == 0xCD ||          // CALL nn
            //         (opcode & 0xC7) == 0xC4) { // CALL c,nn

            //         tmpBreakpoint = emuState.z80ctx.PC + 3;

            //     } else if ((opcode & 0xC7) == 0xC7) { // RST
            //         tmpBreakpoint = emuState.z80ctx.PC + 1;
            //         if ((opcode & 0x38) == 0x08 ||
            //             (opcode & 0x38) == 0x30) {

            //             // Skip one extra byte on RST 08H/30H, since on the Aq these
            //             // system calls absorb the byte following this instruction.
            //             tmpBreakpoint++;
            //         }

            //     } else if (opcode == 0xED) {
            //         opcode = emuState.memRead(emuState.z80ctx.PC + 1);
            //         if (opcode == 0xB9 || // CPDR
            //             opcode == 0xB1 || // CPIR
            //             opcode == 0xBA || // INDR
            //             opcode == 0xB2 || // INIR
            //             opcode == 0xB8 || // LDDR
            //             opcode == 0xB0 || // LDIR
            //             opcode == 0xBB || // OTDR
            //             opcode == 0xB3) { // OTIR
            //         }
            //         tmpBreakpoint = emuState.z80ctx.PC + 2;
            //     }
            //     emuState.tmpBreakpoint = tmpBreakpoint;
            //     if (tmpBreakpoint >= 0) {
            //         emuState.emuMode = EmuState::Em_Running;
            //     } else {
            //         emuState.emuMode = EmuState::Em_Step;
            //     }
            // }
        }

        ImGui::SameLine();
        if (ImGui::Button("Step Out")) {
            emuState.haltAfterRet = 0;
            emuState.emuMode      = EmuState::Em_Running;
        }
#endif

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, emuState.emuMode == EmuState::Em_Running ? (ImVec4)ImColor(0, 128, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        if (ImGui::Button("Go")) {
            emuState.emuMode = EmuState::Em_Running;
        }
        ImGui::PopStyleColor();
        ImGui::EndDisabled();

        ImGui::Separator();

        // {
        //     uint16_t    addr = emuState.z80ctx.PC;
        //     std::string name;
        //     if (asmListing.findNearestSymbol(addr, name)) {
        //         ImGui::Text("%s ($%04X + %u)", name.c_str(), addr, emuState.z80ctx.PC - addr);
        //         ImGui::Separator();
        //     }
        // }

        // {
        //     char tmp1[64];
        //     char tmp2[64];
        //     emuState.z80ctx.tstates = 0;

        //     bool prevEnableBp          = emuState.enableBreakpoints;
        //     emuState.enableBreakpoints = false;
        //     Z80Debug(&emuState.z80ctx, tmp1, tmp2);
        //     emuState.enableBreakpoints = prevEnableBp;

        //     ImGui::Text("         %-12s %s", tmp1, tmp2);
        // }

        {
            auto vaddr = emuState.cpu.addr_translate2(emuState.cpu.pc);
            if (vaddr < 0) {
                ImGui::TextUnformatted("Unmapped virtual address");
            } else {
                auto data = emuState.memRead(vaddr);
                if (data < 0) {
                    ImGui::TextUnformatted("Illegal virtual address");
                } else {
                    auto str = instrToString(data, emuState.cpu.pc);
                    ImGui::Text("%08X %s", (unsigned)data, str.c_str());
                }
            }
        }

        ImGui::Separator();

        auto drawReg = [&](const std::string &name, uint32_t val) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%-3s", name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("0x%08X", val);
        };

        if (ImGui::BeginTable("RegTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Reg", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            static const char *regs[] = {
                "x0/zero", "x1/ra", "x2/sp", "x3/gp",
                "x4/tp", "x5/t0", "x6/t1", "x7/t2",
                "x8/s0/fp", "x9/s1", "x10/a0", "x11/a1",
                "x12/a2", "x13/a3", "x14/a4", "x15/a5",
                "x16/a6", "x17/a7", "x18/s2", "x19/s3",
                "x20/s4", "x21/s5", "x22/s6", "x23/s7",
                "x24/s8", "x25/s9", "x26/s10", "x27/s11",
                "x28/t3", "x29/t4", "x30/t5", "x31/t6"};
            drawReg("pc", emuState.cpu.pc);

            for (int i = 1; i < 32; i++) {
                drawReg(regs[i], emuState.cpu.regs[i]);
            }

            uint32_t mstatus = 0;
            {
                if (emuState.cpu.mstatus_mie)
                    mstatus |= (1 << 3);
                if (emuState.cpu.mstatus_mpie)
                    mstatus |= (1 << 7);
                if (emuState.cpu.mstatus_mpp)
                    mstatus |= (3 << 11);
                if (emuState.cpu.mstatus_mprv)
                    mstatus |= (1 << 17);
            }

            drawReg("mstatus", mstatus);
            drawReg("mie", emuState.cpu.mie);
            drawReg("mtvec", emuState.cpu.mtvec);
            drawReg("mscratch", emuState.cpu.mscratch);
            drawReg("mepc", emuState.cpu.mepc);
            drawReg("mcause", emuState.cpu.mcause);
            drawReg("mtval", emuState.cpu.mtval);
            drawReg("mip", emuState.cpu.mip);
            drawReg("mcycle", emuState.cpu.mcycle & 0xFFFFFFFF);
            drawReg("mcycleh", emuState.cpu.mcycle >> 32);
            drawReg("masid", emuState.cpu.masid);

            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("m_mode");
                ImGui::TableNextColumn();
                ImGui::Text("%u", emuState.cpu.m_mode ? 1 : 0);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

static int memRead(const ImU8 *data, size_t off) {
    (void)data;
    auto paddr = emuState.cpu.addr_translate2(off);
    if (paddr < 0)
        return -1;
    auto val = emuState.memRead(paddr & 0xFFFFFFFC);
    if (val < 0)
        return -1;

    switch (off & 3) {
        case 0: return val & 0xFF;
        case 1: return (val >> 8) & 0xFF;
        case 2: return (val >> 16) & 0xFF;
        case 3: return (val >> 24) & 0xFF;
    }
    return -1;
}
static void memWrite(ImU8 *data, size_t off, ImU8 d) {
    (void)data;
    (void)off;
    (void)d;
    // emuState.memWrite((uint16_t)off, d);
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
        memAreas.emplace_back("Virtual memory", nullptr, 0x100000000ULL);
        memAreas.emplace_back("Main RAM", emuState.ram, sizeof(emuState.ram));
        memAreas.emplace_back("System ROM", emuState.rom, sizeof(emuState.rom));
        memAreas.emplace_back("Text RAM", emuState.video.textram, sizeof(emuState.video.textram));
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

        memEdit.readFn  = (config.memEditMemSelect == 0) ? memRead : nullptr;
        memEdit.writeFn = (config.memEditMemSelect == 0) ? memWrite : nullptr;
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
        if (ImGui::BeginTable("Table", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
            ImGui::TableSetupColumn("En", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Symbol", 0);
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
                    ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 10);
                    ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U32, &bp.addr, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite);
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::BeginCombo(fmtstr("##name%d", row_n).c_str(), bp.name.c_str())) {
                        //     for (auto &sym : asmListing.symbolsStrAddr) {
                        //         if (ImGui::Selectable(fmtstr("%04X %s", sym.second, sym.first.c_str()).c_str())) {
                        //             bp.name  = sym.first;
                        //             bp.value = sym.second;
                        //         }
                        //     }
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
                            // case EmuState::WatchType::Hex8: {
                            //     uint8_t val = emuState.memRead(w.addr);
                            //     if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U8, &val, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            //         emuState.memWrite(w.addr, val);
                            //     }
                            //     break;
                            // }
                            // case EmuState::WatchType::DecU8: {
                            //     uint8_t val = emuState.memRead(w.addr);
                            //     if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U8, &val, nullptr, nullptr, "%u", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            //         emuState.memWrite(w.addr, val);
                            //     }
                            //     break;
                            // }
                            // case EmuState::WatchType::DecS8: {
                            //     int8_t val = emuState.memRead(w.addr);
                            //     if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_S8, &val, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            //         emuState.memWrite(w.addr, val);
                            //     }
                            //     break;
                            // }
                            // case EmuState::WatchType::Hex16: {
                            //     uint16_t val = emuState.memRead(w.addr) | (emuState.memRead(w.addr + 1) << 8);
                            //     if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &val, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            //         emuState.memWrite(w.addr, val & 0xFF);
                            //         emuState.memWrite(w.addr + 1, (val >> 8) & 0xFF);
                            //     }
                            //     break;
                            // }
                            // case EmuState::WatchType::DecU16: {
                            //     uint16_t val = emuState.memRead(w.addr) | (emuState.memRead(w.addr + 1) << 8);
                            //     if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &val, nullptr, nullptr, "%u", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            //         emuState.memWrite(w.addr, val & 0xFF);
                            //         emuState.memWrite(w.addr + 1, (val >> 8) & 0xFF);
                            //     }
                            //     break;
                            // }
                            // case EmuState::WatchType::DecS16: {
                            //     int16_t val = emuState.memRead(w.addr) | (emuState.memRead(w.addr + 1) << 8);
                            //     if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_S16, &val, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            //         emuState.memWrite(w.addr, val & 0xFF);
                            //         emuState.memWrite(w.addr + 1, (val >> 8) & 0xFF);
                            //     }
                            //     break;
                            // }
                            default: break;
                        }
                        // ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &w.value, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite);
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 10);
                        if (ImGui::InputScalar(fmtstr("##addr%d", row_n).c_str(), ImGuiDataType_U16, &w.addr, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                            // auto sym = asmListing.symbolsAddrStr.find(w.addr);
                            // if (sym != asmListing.symbolsAddrStr.end()) {
                            //     w.name = sym->second;
                            // } else {
                            //     w.name = "";
                            // }
                        }
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(-1);
                        if (ImGui::BeginCombo(fmtstr("##name%d", row_n).c_str(), w.name.c_str())) {
                            // for (auto &sym : asmListing.symbolsStrAddr) {
                            //     if (ImGui::Selectable(fmtstr("%04X %s", sym.second, sym.first.c_str()).c_str())) {
                            //         w.name = sym.first;
                            //         w.addr = sym.second;
                            //     }
                            // }
                            ImGui::EndCombo();
                        }

                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 11);

                        static const char *types[] = {
                            "Hex 8", "Dec U8", "Dec S8",
                            "Hex 16", "Dec U16", "Dec S16",
                            "Hex 32", "Dec U32", "Dec S32"};
                        if (ImGui::BeginCombo(fmtstr("##type%d", row_n).c_str(), types[(int)w.type])) {
                            for (int i = 0; i < 9; i++) {
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
