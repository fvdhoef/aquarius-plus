#include "Aq32Video.h"
#include "imgui.h"

Aq32Video::Aq32Video() {
    memset(videoPalette, 0, sizeof(videoPalette));
    memset(textRam, 0, sizeof(textRam));
    memset(videoRam, 0, sizeof(videoRam));
    memset(charRam, 0, sizeof(charRam));
}

void Aq32Video::reset() {
    videoCtrl    = 0;
    videoLine    = 0;
    videoIrqLine = 0;
    videoScrX1   = 0;
    videoScrY1   = 0;
    videoScrX2   = 0;
    videoScrY2   = 0;
}

void Aq32Video::renderer(unsigned &idx, uint32_t data, bool hFlip, unsigned palette, unsigned zDepth, bool zDepthInit) {
    for (int i = 0; i < 8; i++, idx++) {
        idx &= 511;

        unsigned colIdx =
            hFlip
                ? ((data >> (i * 4)) & 0xF)
                : ((data >> ((7 - i) * 4)) & 0xF);

        bool isTransparent = colIdx == 0;

        if (!zDepthInit && (isTransparent || zDepth < lineZ[idx]))
            continue;

        lineGfx[idx] = palette | colIdx;
        lineZ[idx]   = isTransparent ? 0 : zDepth;
    }
}

void Aq32Video::drawLine(int line) {
    if (line < 0 || line >= activeHeight)
        return;

    // Render text
    uint8_t lineText[1024];
    {
        bool mode80 = (videoCtrl & VCTRL_TEXT_MODE80) != 0;

        for (int i = 0; i < activeWidth; i++) {
            // Draw text character
            uint16_t val    = mode80
                                  ? textRam[(line / 8) * 80 + (i / 8)]
                                  : textRam[(line / 8) * 40 + ((i / 2) / 8)];
            uint8_t  ch     = val & 0xFF;
            uint8_t  color  = val >> 8;
            uint8_t  charBm = charRam[ch * 8 + (line & 7)];
            lineText[i]     = (charBm & (1 << (7 - ((mode80 ? i : (i / 2)) & 7)))) ? (color >> 4) : (color & 0xF);
        }
    }

    // Render bitmap/tile layer
    {
        bool tileMode = (videoCtrl & VCTRL_GFX_TILEMODE) != 0;
        bool bmWrap   = (videoCtrl & VCTRL_BM_WRAP) != 0;

        if ((videoCtrl & VCTRL_GFX_EN) == 0) {
            for (int i = 0; i < 320; i++) {
                lineGfx[i] = 0;
                lineZ[i]   = 0;
            }

        } else {
            unsigned numLayers = 1;
            if (tileMode && (videoCtrl & VCTRL_LAYER2_EN) != 0)
                numLayers = 2;

            for (unsigned layer = 0; layer < numLayers; layer++) {
                unsigned scrX     = layer == 0 ? videoScrX1 : videoScrX2;
                unsigned scrY     = layer == 0 ? videoScrY1 : videoScrY2;
                unsigned tileLine = (line + scrY);
                unsigned idx      = (0 - (scrX & 7)) & 511;
                unsigned row      = ((tileLine & 255) >> 3) & 31;
                unsigned col      = scrX >> 3;

                unsigned bmLine = tileLine;
                if (bmWrap) {
                    if (bmLine >= 400)
                        bmLine -= 400;
                    else if (bmLine >= 200)
                        bmLine -= 200;
                } else {
                    bmLine = tileLine & 255;
                }

                for (int i = 0; i < 41; i++) {
                    bool     hFlip   = false;
                    uint8_t  palette = 0x10;
                    unsigned zDepth  = 2;
                    unsigned patOffs = 0;

                    if (tileMode) {
                        // Tilemap is 64x32 (2 bytes per entry)
                        unsigned tilemapOffset = (layer == 0 ? 0x7000 : 0x6000) | (row << 7) | (col << 1);

                        // Fetch tilemap entry
                        uint16_t entry = (videoRam[tilemapOffset | 1] << 8) | videoRam[tilemapOffset];

                        unsigned tileIdx = entry & 1023;
                        bool     prio    = (entry & (1 << 10)) != 0;
                        hFlip            = (entry & (1 << 11)) != 0;
                        bool vFlip       = (entry & (1 << 12)) != 0;
                        palette          = (entry >> 9) & 0x70;

                        patOffs = (tileIdx << 5) | ((tileLine & 7) << 2);
                        if (vFlip)
                            patOffs ^= (7 << 2);

                        zDepth = ((layer + 2) << 1) | (prio ? 1 : 0);

                    } else {
                        patOffs = bmLine * 160 + col * 4;
                    }

                    uint32_t data =
                        (videoRam[(patOffs + 0) & 0x7FFF] << 24) |
                        (videoRam[(patOffs + 1) & 0x7FFF] << 16) |
                        (videoRam[(patOffs + 2) & 0x7FFF] << 8) |
                        (videoRam[(patOffs + 3) & 0x7FFF] << 0);

                    if (!tileMode && !bmWrap && (col >= 40 || row >= 25)) {
                        data = 0;
                    }

                    renderer(idx, data, hFlip, palette, zDepth, layer == 0);

                    // Next column
                    col = (col + 1) & 63;

                    if (bmWrap && col >= 40)
                        col = 0;
                }
            }

            // if ((videoCtrl & VCTRL_GFX_TILEMODE) == 0) {
            //     unsigned idx = (-(videoScrX1 & 7)) & 511;
            //     unsigned col = videoScrX1 >> 3;

            //     // Bitmap mode 4bpp
            //     int bmline = (line + videoScrY1) % 200;
            //     for (int i = 0; i < 41; i++) {
            //         unsigned patOffs = (bmline * 40 + col) * 4;

            //         // Next column
            //         col = (col + 1) % 40;

            //         for (int n = 0; n < 4; n++) {
            //             uint8_t data = videoRam[patOffs + n];
            //             {
            //                 uint8_t val = data >> 4;
            //                 val |= 0x10;
            //                 lineGfx[idx++] = val;
            //                 idx &= 511;
            //             }
            //             {
            //                 uint8_t val = data & 0xF;
            //                 val |= 0x10;
            //                 lineGfx[idx++] = val;
            //                 idx &= 511;
            //             }
            //         }
            //     }

            // } else {

            // }
        }

        // Render sprites
        if ((videoCtrl & VCTRL_SPR_EN) != 0) {
            for (int i = 0; i < 256; i++) {
                uint32_t pos  = spritePos[i];
                uint32_t attr = spriteAttr[i];
                unsigned posX = pos & 511;
                unsigned posY = (pos >> 16) & 255;

                // Check if sprite is visible on this line
                bool h16     = (attr & (1 << 10)) != 0;
                int  sprLine = (line - posY) & 0xFF;
                if (sprLine >= (h16 ? 16 : 8))
                    continue;

                unsigned tileIdx = attr & 1023;
                bool     hFlip   = (attr & (1 << 11)) != 0;
                bool     vFlip   = (attr & (1 << 12)) != 0;
                uint8_t  palette = (attr >> 9) & 0x70;
                uint8_t  depth   = (attr >> 15) & 6;

                unsigned idx = posX;

                if (vFlip)
                    sprLine ^= (h16 ? 15 : 7);

                tileIdx ^= (sprLine >> 3);

                unsigned patOffs = (tileIdx << 5) | ((sprLine & 7) << 2);

                for (int n = 0; n < 4; n++) {
                    int m = n;
                    if (hFlip)
                        m ^= 3;

                    uint8_t data = videoRam[patOffs + m];

                    if (!hFlip) {
                        unsigned colIdx = (data >> 4);
                        if (colIdx != 0 && depth >= lineZ[idx]) {
                            lineZ[idx]   = depth;
                            lineGfx[idx] = palette | colIdx;
                        }
                        idx = (idx + 1) & 511;
                    }
                    {
                        unsigned colIdx = (data & 0xF);
                        if (colIdx != 0 && depth >= lineZ[idx]) {
                            lineZ[idx]   = depth;
                            lineGfx[idx] = palette | colIdx;
                        }
                        idx = (idx + 1) & 511;
                    }
                    if (hFlip) {
                        unsigned colIdx = (data >> 4);
                        if (colIdx != 0 && depth >= lineZ[idx]) {
                            lineZ[idx]   = depth;
                            lineGfx[idx] = palette | colIdx;
                        }
                        idx = (idx + 1) & 511;
                    }
                }
            }
        }
    }

    // Compose layers
    {
        uint16_t *pd = &screen[line * activeWidth];

        for (int i = 0; i < activeWidth; i++) {
            bool textPriority = (videoCtrl & VCTRL_TEXT_PRIO) != 0;
            bool textEnable   = (videoCtrl & VCTRL_TEXT_EN) != 0;

            uint8_t colIdx = 0;
            if (textEnable && !textPriority)
                colIdx = lineText[i];
            if (!textEnable || textPriority || (lineGfx[i / 2] & 0xF) != 0)
                colIdx = lineGfx[i / 2];
            if (textEnable && textPriority && (lineText[i] & 0xF) != 0)
                colIdx = lineText[i];

            pd[i] = videoPalette[colIdx & 0x3F];
        }
    }
}

void Aq32Video::dbgDrawIoRegs() {
    ImGui::Text("VCTRL   : 0x%02X", videoCtrl);
    ImGui::Text("  TEXT_ENABLE  : %u", (videoCtrl & VCTRL_TEXT_EN) ? 1 : 0);
    ImGui::Text("  TEXT_MODE80  : %u", (videoCtrl & VCTRL_TEXT_MODE80) ? 1 : 0);
    ImGui::Text("  TEXT_PRIO    : %u", (videoCtrl & VCTRL_TEXT_PRIO) ? 1 : 0);
    ImGui::Text("  GFX_EN       : %u", (videoCtrl & VCTRL_GFX_EN) ? 1 : 0);
    ImGui::Text("  GFX_TILEMODE : %u", (videoCtrl & VCTRL_GFX_TILEMODE) ? 1 : 0);
    ImGui::Text("  SPR_EN       : %u", (videoCtrl & VCTRL_SPR_EN) ? 1 : 0);
    ImGui::Text("  LAYER2_EN    : %u", (videoCtrl & VCTRL_LAYER2_EN) ? 1 : 0);
    ImGui::Text("VLINE   : %u", videoLine);
    ImGui::Text("VIRQLINE: %u", videoIrqLine);
    ImGui::Text("VSCRX1  : %u", videoScrX1);
    ImGui::Text("VSCRY1  : %u", videoScrY1);
    ImGui::Text("VSCRX2  : %u", videoScrX2);
    ImGui::Text("VSCRY2  : %u", videoScrY2);
}

void Aq32Video::dbgDrawSpriteRegs() {
    if (ImGui::BeginTable("Table", 10, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tile", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Depth", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Pal", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("VF", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("HF", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("H16");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(256);
        while (clipper.Step()) {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%2d", row_n);
                ImGui::TableNextColumn();
                ImGui::Text("%3d", spritePos[row_n] & 511);
                ImGui::TableNextColumn();
                ImGui::Text("%3d", (spritePos[row_n] >> 16) & 255);
                ImGui::TableNextColumn();
                ImGui::Text("%3d", spriteAttr[row_n] & 1023);
                ImGui::TableNextColumn();
                ImGui::Text("%d", (spriteAttr[row_n] >> 16) & 3);
                ImGui::TableNextColumn();
                ImGui::Text("%d", (spriteAttr[row_n] >> 13) & 7);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted((spriteAttr[row_n] & (1 << 12)) ? "X" : "");
                ImGui::TableNextColumn();
                ImGui::TextUnformatted((spriteAttr[row_n] & (1 << 11)) ? "X" : "");
                ImGui::TableNextColumn();
                ImGui::TextUnformatted((spriteAttr[row_n] & (1 << 10)) ? "X" : "");
            }
        }
        ImGui::EndTable();
    }
}

void Aq32Video::dbgDrawPaletteRegs() {
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
        clipper.Begin(128);
        while (clipper.Step()) {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                int r = (videoPalette[row_n] >> 8) & 0xF;
                int g = (videoPalette[row_n] >> 4) & 0xF;
                int b = (videoPalette[row_n] >> 0) & 0xF;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%2d", row_n);
                ImGui::TableNextColumn();
                ImGui::Text("%d", row_n / 16);
                ImGui::TableNextColumn();
                ImGui::Text("%2d", row_n & 15);
                ImGui::TableNextColumn();
                ImGui::Text("%03X", videoPalette[row_n]);
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
