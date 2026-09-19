#include "Z80Core.h"
#include "imgui.h"
#include "tinyfiledialogs.h"

Z80Core::Z80Core() {
    z80ctx.ioRead    = _z80IoRead;
    z80ctx.ioWrite   = _z80IoWrite;
    z80ctx.ioParam   = reinterpret_cast<uintptr_t>(this);
    z80ctx.instrRead = _z80InstrRead;
    z80ctx.memRead   = _z80MemRead;
    z80ctx.memWrite  = _z80MemWrite;
    z80ctx.memParam  = reinterpret_cast<uintptr_t>(this);
}

void Z80Core::reset() {
    Z80RESET(&z80ctx);
    emuMode = Em_Running;
}

void Z80Core::loadConfig(cJSON *root) {
    showCpuState        = getBoolValue(root, "showCpuState", false);
    showBreakpoints     = getBoolValue(root, "showBreakpoints", false);
    showAssemblyListing = getBoolValue(root, "showAssemblyListing", false);
    showCpuTrace        = getBoolValue(root, "showCpuTrace", false);
    showWatch           = getBoolValue(root, "showWatch", false);
    stopOnHalt          = getBoolValue(root, "stopOnHalt", false);

    auto cfgBreakpoints = cJSON_GetObjectItem(root, "breakpoints");
    if (cJSON_IsArray(cfgBreakpoints)) {
        cJSON *breakpoint;
        cJSON_ArrayForEach(breakpoint, cfgBreakpoints) {
            if (!cJSON_IsObject(breakpoint))
                continue;

            Breakpoint bp;
            bp.addr    = getIntValue(breakpoint, "addr", 0);
            bp.name    = getStringValue(breakpoint, "name", "");
            bp.enabled = getBoolValue(breakpoint, "enabled", false);
            bp.type    = (BpType)getIntValue(breakpoint, "type", 0);
            bp.onR     = getBoolValue(breakpoint, "onR", false);
            bp.onW     = getBoolValue(breakpoint, "onW", false);
            bp.onX     = getBoolValue(breakpoint, "onX", false);
            breakpoints.push_back(bp);
        }
    }
    enableBreakpoints = getBoolValue(root, "enableBreakpoints", false);
    traceEnable       = getBoolValue(root, "traceEnable", false);
    traceDepth        = getIntValue(root, "traceDepth", 16);

    auto cfgWatches = cJSON_GetObjectItem(root, "watches");
    if (cJSON_IsArray(cfgWatches)) {
        cJSON *watch;
        cJSON_ArrayForEach(watch, cfgWatches) {
            if (!cJSON_IsObject(watch))
                continue;

            Watch w;
            w.addr = getIntValue(watch, "addr", 0);
            w.name = getStringValue(watch, "name", "");
            w.type = (WatchType)getIntValue(watch, "type", 0);
            watches.push_back(w);
        }
    }
}

void Z80Core::saveConfig(cJSON *root) {
    cJSON_AddBoolToObject(root, "showCpuState", showCpuState);
    cJSON_AddBoolToObject(root, "showBreakpoints", showBreakpoints);
    cJSON_AddBoolToObject(root, "showAssemblyListing", showAssemblyListing);
    cJSON_AddBoolToObject(root, "showCpuTrace", showCpuTrace);
    cJSON_AddBoolToObject(root, "showWatch", showWatch);
    cJSON_AddBoolToObject(root, "stopOnHalt", stopOnHalt);

    cJSON_AddBoolToObject(root, "enableBreakpoints", enableBreakpoints);
    cJSON_AddBoolToObject(root, "traceEnable", traceEnable);
    cJSON_AddNumberToObject(root, "traceDepth", traceDepth);

    auto cfgBreakpoints = cJSON_AddArrayToObject(root, "breakpoints");
    for (auto &bp : breakpoints) {
        auto breakpoint = cJSON_CreateObject();

        cJSON_AddNumberToObject(breakpoint, "addr", bp.addr);
        cJSON_AddStringToObject(breakpoint, "name", bp.name.c_str());
        cJSON_AddBoolToObject(breakpoint, "enabled", bp.enabled);
        cJSON_AddNumberToObject(breakpoint, "type", (int)bp.type);
        cJSON_AddBoolToObject(breakpoint, "onR", bp.onR);
        cJSON_AddBoolToObject(breakpoint, "onW", bp.onW);
        cJSON_AddBoolToObject(breakpoint, "onX", bp.onX);
        cJSON_AddItemToArray(cfgBreakpoints, breakpoint);
    }

    auto cfgWatches = cJSON_AddArrayToObject(root, "watches");
    for (auto &w : watches) {
        auto watch = cJSON_CreateObject();
        cJSON_AddNumberToObject(watch, "addr", w.addr);
        cJSON_AddStringToObject(watch, "name", w.name.c_str());
        cJSON_AddNumberToObject(watch, "type", (int)w.type);
        cJSON_AddItemToArray(cfgWatches, watch);
    }
}

bool Z80Core::checkExecuteBreakpoints() {
    lastBp        = -1;
    lastBpAddress = -1;

    for (int i = 0; i < (int)breakpoints.size(); i++) {
        auto &bp = breakpoints[i];
        if (bp.enabled && bp.type == BpType::Mem && bp.onX && z80ctx.PC == bp.addr && bp.addr != lastBpAddress) {
            lastBp           = i;
            lastBpAddress    = bp.addr;
            lastBpAccessType = 3;
            return true;
        }
    }
    return false;
}

int Z80Core::emulate() {
    if (!enableDebugger) {
        emuMode       = Em_Running;
        lastBpAddress = -1;
        tmpBreakpoint = -1;
        haltAfterRet  = -1;
    }
    if (emuMode == Em_Halted) {
        tmpBreakpoint = -1;
        haltAfterRet  = -1;
        return 1;
    }

    if (enableDebugger) {
        // Break on temporary breakpoint?
        if (tmpBreakpoint == z80ctx.PC) {
            tmpBreakpoint = -1;
            emuMode       = Em_Halted;
            return 0;
        }

        // Check breakpoints
        if (enableBreakpoints) {
            if (emuMode == Em_Running && z80ctx.PC != lastBpAddress && checkExecuteBreakpoints()) {
                emuMode = Em_Halted;
                return 0;
            }
        }

        if (haltAfterRet >= 0) {
            uint8_t opcode = memRead(z80ctx.PC);
            if (opcode == 0xCD ||          // CALL nn
                (opcode & 0xC7) == 0xC4) { // CALL c,nn
                haltAfterRet++;

            } else if (
                opcode == 0xC9 ||          // RET
                (opcode & 0xC7) == 0xC7) { // RET cc
                if (--haltAfterRet < 0) {
                    haltAfterRet = -1;
                    emuMode      = Em_Step;
                }
            }
        }
    }

    if (emuMode == Em_Running) {
        lastBp        = -1;
        lastBpAddress = -1;
    }

    // Generate interrupt if needed
    if (hasIrq())
        Z80INT(&z80ctx, 0xFF);

    // Emulate 1 instruction
    z80ctx.tstates = 0;
    Z80Execute(&z80ctx);
    int delta = z80ctx.tstates;

    if (emuMode != Em_Running && lastBp < 0) {
        checkExecuteBreakpoints();
    }

    if (emuMode == Em_Step)
        emuMode = Em_Halted;

    if (enableDebugger && traceEnable) {
        // Add next instruction to trace
        cpuTrace.emplace_back();
        auto &entry = cpuTrace.back();
        entry.pc    = z80ctx.PC;
        entry.r1    = z80ctx.R1;
        entry.r2    = z80ctx.R2;
        decodeInstruction(entry.bytes, entry.instrStr);

        // Limit trace depth
        while ((int)cpuTrace.size() > traceDepth)
            cpuTrace.pop_front();
    }
    return delta;
}

void Z80Core::decodeInstruction(char hex[32], char instr[32]) {
    // Prevent triggering breakpoints
    bool prevEnableBp = enableBreakpoints;
    enableBreakpoints = false;

    // Decode instruction
    int tstates = z80ctx.tstates;
    Z80Debug(&z80ctx, hex, instr);
    z80ctx.tstates = tstates;

    // Reenable breakpoint
    enableBreakpoints = prevEnableBp;
}

void Z80Core::halt() {
    lastBpAddress = -1;
    tmpBreakpoint = -1;
    haltAfterRet  = -1;
    emuMode       = Em_Halted;
}

void Z80Core::stepInto() {
    tmpBreakpoint = -1;
    haltAfterRet  = -1;
    emuMode       = Em_Step;
}

void Z80Core::stepOver() {
    tmpBreakpoint = -1;

    if (z80ctx.halted) {
        // Step over HALT instruction
        z80ctx.halted = 0;
        z80ctx.PC++;

    } else {
        uint8_t opcode = memRead(z80ctx.PC);
        if (opcode == 0xCD ||          // CALL nn
            (opcode & 0xC7) == 0xC4) { // CALL c,nn

            tmpBreakpoint = z80ctx.PC + 3;
            emuMode       = Em_Running;

        } else if ((opcode & 0xC7) == 0xC7) { // RST
            tmpBreakpoint = z80ctx.PC + 1;
            if ((opcode & 0x38) == 0x08 ||
                (opcode & 0x38) == 0x30) {

                // Skip one extra byte on RST 08H/30H, since on the Aq these
                // system calls absorb the byte following this instruction.
                tmpBreakpoint++;
            }
            emuMode = Em_Running;

        } else if (opcode == 0xED) {
            opcode = memRead(z80ctx.PC + 1);
            if (opcode == 0xB9 || // CPDR
                opcode == 0xB1 || // CPIR
                opcode == 0xBA || // INDR
                opcode == 0xB2 || // INIR
                opcode == 0xB8 || // LDDR
                opcode == 0xB0 || // LDIR
                opcode == 0xBB || // OTDR
                opcode == 0xB3) { // OTIR
            }
            tmpBreakpoint = z80ctx.PC + 2;
            emuMode       = Em_Running;

        } else {
            stepInto();
        }
    }
}

void Z80Core::stepOut() {
    haltAfterRet = 0;
    emuMode      = Em_Running;
}

void Z80Core::go() {
    tmpBreakpoint = -1;
    haltAfterRet  = -1;
    emuMode       = Em_Running;
}

uint8_t Z80Core::z80InstrRead(uint16_t addr) {
    return memRead(addr);
}

uint8_t Z80Core::z80MemRead(uint16_t addr) {
    if (enableBreakpoints) {
        for (int i = 0; i < (int)breakpoints.size(); i++) {
            auto &bp = breakpoints[i];
            if (bp.enabled && bp.onR && bp.type == BpType::Mem && addr == bp.addr && bp.addr != lastBpAddress) {
                emuMode          = Em_Halted;
                lastBp           = i;
                lastBpAddress    = bp.addr;
                lastBpAccessType = 1;
            }
        }
    }
    return memRead(addr);
}

void Z80Core::z80MemWrite(uint16_t addr, uint8_t data) {
    if (enableBreakpoints) {
        for (int i = 0; i < (int)breakpoints.size(); i++) {
            auto &bp = breakpoints[i];
            if (bp.enabled && bp.onW && bp.type == BpType::Mem && addr == bp.addr && bp.addr != lastBpAddress) {
                emuMode          = Em_Halted;
                lastBp           = i;
                lastBpAddress    = bp.addr;
                lastBpAccessType = 2;
            }
        }
    }
    memWrite(addr, data);
}

uint8_t Z80Core::z80IoRead(uint16_t addr) {
    if (enableBreakpoints) {
        for (int i = 0; i < (int)breakpoints.size(); i++) {
            auto &bp = breakpoints[i];
            if (bp.enabled && bp.onR && ((bp.type == BpType::Io8 && (addr & 0xFF) == (bp.addr & 0xFF)) || (bp.type == BpType::Io16 && addr == bp.addr))) {
                emuMode = Em_Halted;
            }
        }
    }
    return ioRead(addr);
}

void Z80Core::z80IoWrite(uint16_t addr, uint8_t data) {
    if (enableBreakpoints) {
        for (int i = 0; i < (int)breakpoints.size(); i++) {
            auto &bp = breakpoints[i];
            if (bp.enabled && bp.onW && ((bp.type == BpType::Io8 && (addr & 0xFF) == (bp.addr & 0xFF)) || (bp.type == BpType::Io16 && addr == bp.addr))) {
                emuMode = Em_Halted;
            }
        }
    }
    ioWrite(addr, data);
}

void Z80Core::dbgMenu() {
    ImGui::MenuItem("CPU state", "", &showCpuState);
    ImGui::MenuItem("Breakpoints", "", &showBreakpoints);
    ImGui::MenuItem("Assembly listing", "", &showAssemblyListing);
    ImGui::MenuItem("CPU trace", "", &showCpuTrace);
    ImGui::MenuItem("Watch", "", &showWatch);
    ImGui::MenuItem("Stop on HALT instruction", "", &stopOnHalt);
}

void Z80Core::dbgWindows() {
    if (emuMode == Em_Halted)
        showCpuState = true;
    if (showCpuState)
        dbgWndCpuState(&showCpuState);
    if (showBreakpoints)
        dbgWndBreakpoints(&showBreakpoints);
    if (showAssemblyListing)
        dbgWndAssemblyListing(&showAssemblyListing);
    if (showCpuTrace)
        dbgWndCpuTrace(&showCpuTrace);
    if (showWatch)
        dbgWndWatch(&showWatch);
}

void Z80Core::dbgWndCpuState(bool *p_open) {
    if (ImGui::Begin("CPU state", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::PushStyleColor(ImGuiCol_Button, emuMode == Em_Halted ? (ImVec4)ImColor(192, 0, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        ImGui::BeginDisabled(emuMode != Em_Running);
        {
            ImGui::SetNextItemShortcut(ImGuiMod_Shift | ImGuiKey_F5, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
            if (ImGui::Button("Halt"))
                halt();
            ImGui::EndDisabled();
        }
        ImGui::PopStyleColor();

        ImGui::BeginDisabled(emuMode == Em_Running);
        {
            ImGui::SameLine();
            ImGui::SetNextItemShortcut(ImGuiKey_F11, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
            if (ImGui::Button("Step Into"))
                stepInto();

            ImGui::SameLine();
            ImGui::SetNextItemShortcut(ImGuiKey_F10, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
            if (ImGui::Button("Step Over"))
                stepOver();

            ImGui::SameLine();
            ImGui::SetNextItemShortcut(ImGuiMod_Shift | ImGuiKey_F10, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
            if (ImGui::Button("Step Out"))
                stepOut();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, emuMode == Em_Running ? (ImVec4)ImColor(0, 128, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
            ImGui::SetNextItemShortcut(ImGuiKey_F5, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Tooltip);
            if (ImGui::Button("Go"))
                go();
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();
        ImGui::Separator();

        // Symbol
        {
            uint16_t    addr = z80ctx.PC;
            std::string name;
            if (asmListing.findNearestSymbol(addr, name)) {
                ImGui::Text("%s ($%04X + %u)", name.c_str(), addr, z80ctx.PC - addr);
                ImGui::Separator();
            }
        }

        // Decoded instruction
        {
            char hex[32];
            char instr[32];
            decodeInstruction(hex, instr);
            ImGui::Text("         %-12s %s", hex, instr);
        }
        ImGui::Separator();

        auto drawAddrVal = [&](const std::string &name, uint16_t val) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%-3s", name.c_str());
            ImGui::TableNextColumn();

            if (emuMode == Em_Running) {
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
                data[i] = memRead(val + i);
            ImGui::Text(
                "%02X %02X %02X %02X %02X %02X %02X %02X",
                memRead(val + 0),
                memRead(val + 1),
                memRead(val + 2),
                memRead(val + 3),
                memRead(val + 4),
                memRead(val + 5),
                memRead(val + 6),
                memRead(val + 7));

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

            drawAddrVal("PC", z80ctx.PC);
            drawAddrVal("SP", z80ctx.R1.wr.SP);
            drawAF("AF", z80ctx.R1.wr.AF);
            drawAddrVal("BC", z80ctx.R1.wr.BC);
            drawAddrVal("DE", z80ctx.R1.wr.DE);
            drawAddrVal("HL", z80ctx.R1.wr.HL);
            drawAddrVal("IX", z80ctx.R1.wr.IX);
            drawAddrVal("IY", z80ctx.R1.wr.IY);
            drawAF("AF'", z80ctx.R2.wr.AF);
            drawAddrVal("BC'", z80ctx.R2.wr.BC);
            drawAddrVal("DE'", z80ctx.R2.wr.DE);
            drawAddrVal("HL'", z80ctx.R2.wr.HL);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("IR");
            ImGui::TableNextColumn();
            ImGui::Text("%04X", (z80ctx.I << 8) | z80ctx.R);
            ImGui::TableNextColumn();
            ImGui::Text("IM %u  Interrupts %3s", z80ctx.IM, z80ctx.IFF1 ? "On" : "Off");
            ImGui::TableNextColumn();

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void Z80Core::dbgWndBreakpoints(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Breakpoints", p_open, 0)) {

        ImGui::Checkbox("Enable breakpoints", &enableBreakpoints);
        ImGui::SameLine(ImGui::GetWindowWidth() - 25);
        if (ImGui::Button("+")) {
            breakpoints.emplace_back();
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
            clipper.Begin((int)breakpoints.size());
            int eraseIdx = -1;

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    auto &bp = breakpoints[row_n];
                    ImGui::TableNextRow();

                    if (row_n == lastBp)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32((ImVec4)ImColor(128, 0, 128)));

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

                    if (row_n == lastBp && lastBpAccessType == 1)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor(255, 0, 255)));

                    ImGui::Checkbox(fmtstr("##onR%d", row_n).c_str(), &bp.onR);
                    ImGui::TableNextColumn();

                    if (row_n == lastBp && lastBpAccessType == 2)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor(255, 0, 255)));

                    ImGui::Checkbox(fmtstr("##onW%d", row_n).c_str(), &bp.onW);
                    ImGui::TableNextColumn();

                    if (row_n == lastBp && lastBpAccessType == 3)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor(255, 0, 255)));

                    ImGui::Checkbox(fmtstr("##onX%d", row_n).c_str(), &bp.onX);
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 9);

                    static const char *types[] = {"Mem", "IO 8", "IO 16"};
                    if (ImGui::BeginCombo(fmtstr("##type%d", row_n).c_str(), types[(int)bp.type])) {
                        for (int i = 0; i < 3; i++)
                            if (ImGui::Selectable(types[i]))
                                bp.type = (BpType)i;

                        ImGui::EndCombo();
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Button(fmtstr("X##del%d", row_n).c_str())) {
                        eraseIdx = row_n;
                    }
                }
            }
            if (eraseIdx >= 0) {
                breakpoints.erase(breakpoints.begin() + eraseIdx);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void Z80Core::dbgWndAssemblyListing(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Assembly listing", p_open, 0)) {
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
            bool         updateScroll = (emuMode == Em_Halted && lastPC != z80ctx.PC);
            if (updateScroll) {
                lastPC = z80ctx.PC;
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
                    if (emuMode == Em_Halted && z80ctx.PC == line.addr) {
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

                    if (line.addr >= 0 && line.addr == z80ctx.PC) {
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

void Z80Core::dbgWndCpuTrace(bool *p_open) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(700, 200), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("CPU trace", p_open, 0)) {
        ImGui::Checkbox("Enable tracing", &traceEnable);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 8);

        const int minDepth = 16, maxDepth = 16384;
        ImGui::DragInt("Trace depth", &traceDepth, 1, minDepth, maxDepth);
        traceDepth = std::max(minDepth, std::min(traceDepth, maxDepth));

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
            clipper.Begin((int)cpuTrace.size());

            auto regColumn = [](uint16_t value, uint16_t prevValue) {
                ImGui::TableNextColumn();

                if (prevValue != value)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor(0, 128, 0)));

                ImGui::Text("%04X", value);
            };

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    auto &entry     = cpuTrace[row_n];
                    auto &prevEntry = row_n < 1 ? entry : cpuTrace[row_n - 1];

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%4d", row_n - (traceDepth - 1));
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

void Z80Core::dbgWndWatch(bool *p_open) {
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
            int              numWatches = (int)watches.size();
            clipper.Begin(numWatches + 1);
            int eraseIdx = -1;

            while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    ImGui::TableNextRow();

                    if (row_n < numWatches) {
                        auto &w = watches[row_n];
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 7);
                        switch (w.type) {
                            case WatchType::Hex8: {
                                uint8_t val = memRead(w.addr);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U8, &val, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    memWrite(w.addr, val);
                                }
                                break;
                            }
                            case WatchType::DecU8: {
                                uint8_t val = memRead(w.addr);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U8, &val, nullptr, nullptr, "%u", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    memWrite(w.addr, val);
                                }
                                break;
                            }
                            case WatchType::DecS8: {
                                int8_t val = memRead(w.addr);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_S8, &val, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    memWrite(w.addr, val);
                                }
                                break;
                            }
                            case WatchType::Hex16: {
                                uint16_t val = memRead(w.addr) | (memRead(w.addr + 1) << 8);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &val, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    memWrite(w.addr, val & 0xFF);
                                    memWrite(w.addr + 1, (val >> 8) & 0xFF);
                                }
                                break;
                            }
                            case WatchType::DecU16: {
                                uint16_t val = memRead(w.addr) | (memRead(w.addr + 1) << 8);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U16, &val, nullptr, nullptr, "%u", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    memWrite(w.addr, val & 0xFF);
                                    memWrite(w.addr + 1, (val >> 8) & 0xFF);
                                }
                                break;
                            }
                            case WatchType::DecS16: {
                                int16_t val = memRead(w.addr) | (memRead(w.addr + 1) << 8);
                                if (ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_S16, &val, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AlwaysOverwrite)) {
                                    memWrite(w.addr, val & 0xFF);
                                    memWrite(w.addr + 1, (val >> 8) & 0xFF);
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
                                    w.type = (WatchType)i;
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
                            watches.emplace_back();
                        }
                    }
                }
            }
            if (eraseIdx >= 0) {
                watches.erase(watches.begin() + eraseIdx);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void Z80Core::addrPopup(uint16_t addr) {
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
            tmpBreakpoint = addr;
            emuMode       = Em_Running;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Add breakpoint")) {
            Breakpoint bp;
            bp.enabled = true;
            bp.addr    = addr;
            bp.onR     = false;
            bp.onW     = false;
            bp.onX     = true;
            breakpoints.push_back(bp);
            ImGui::CloseCurrentPopup();
            listingReloaded();
        }
        if (showInMemEdit && ImGui::MenuItem("Show in memory editor")) {
            showInMemEdit(addr);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::BeginMenu("Add watch")) {
            int i = -1;

            if (ImGui::MenuItem("Hex 8"))
                i = (int)WatchType::Hex8;
            if (ImGui::MenuItem("Dec U8"))
                i = (int)WatchType::DecU8;
            if (ImGui::MenuItem("Dec S8"))
                i = (int)WatchType::DecS8;
            if (ImGui::MenuItem("Hex 16"))
                i = (int)WatchType::Hex16;
            if (ImGui::MenuItem("Dec U16"))
                i = (int)WatchType::DecU16;
            if (ImGui::MenuItem("Dec S16"))
                i = (int)WatchType::DecS16;

            if (i >= 0) {
                Watch w;
                w.addr = addr;
                w.type = (WatchType)i;
                watches.push_back(w);
                ImGui::CloseCurrentPopup();
                listingReloaded();
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

void Z80Core::listingReloaded() {
    // Update watches
    for (auto &w : watches) {
        if (!asmListing.findSymbolAddr(w.name, w.addr)) {
            if (!asmListing.findSymbolName(w.addr, w.name)) {
                w.name = "";
            }
        }
    }

    // Update breakpoints
    for (auto &bp : breakpoints) {
        if (!asmListing.findSymbolAddr(bp.name, bp.addr)) {
            if (!asmListing.findSymbolName(bp.addr, bp.name)) {
                bp.name = "";
            }
        }
    }
}