#include "MemoryEditor.h"

#ifdef _MSC_VER
#    define _PRISizeT "I"
#    define ImSnprintf _snprintf
#else
#    define _PRISizeT "z"
#    define ImSnprintf snprintf
#endif

void MemoryEditor::gotoAddrAndHighlight(size_t addr_min, size_t addr_max) {
    gotoAddr     = addr_min;
    highlightMin = addr_min;
    highlightMax = addr_max;
}

void MemoryEditor::calcSizes(Sizes &s, size_t mem_size, size_t base_display_addr) {
    ImGuiStyle &style = ImGui::GetStyle();
    s.addrDigitsCount = optAddrDigitsCount;
    if (s.addrDigitsCount == 0)
        for (size_t n = base_display_addr + mem_size - 1; n > 0; n >>= 4)
            s.addrDigitsCount++;
    s.lineHeight            = ImGui::GetTextLineHeight();
    s.glyphWidth            = ImGui::CalcTextSize("F").x + 1;       // We assume the font is mono-space
    s.hexCellWidth          = (float)(int)(s.glyphWidth * 2.5f);    // "FF " we include trailing space in the width to easily catch clicks everywhere
    s.spacingBetweenMidCols = (float)(int)(s.hexCellWidth * 0.25f); // Every optMidColsCount columns we add a bit of extra spacing
    s.posHexStart           = (s.addrDigitsCount + 2) * s.glyphWidth;
    s.posHexEnd             = s.posHexStart + (s.hexCellWidth * cols);
    s.posAsciiStart         = s.posHexEnd;
    s.posAsciiEnd           = s.posHexEnd;
    s.posAsciiStart         = s.posHexEnd + s.glyphWidth * 1;
    if (optMidColsCount > 0)
        s.posAsciiStart += (float)((cols + optMidColsCount - 1) / optMidColsCount) * s.spacingBetweenMidCols;
    s.posAsciiEnd = s.posAsciiStart + cols * s.glyphWidth;
    s.windowWidth = s.posAsciiEnd + style.ScrollbarSize + style.WindowPadding.x * 2 + s.glyphWidth;
}

// Standalone Memory Editor window
void MemoryEditor::drawWindow(const char *title, void *mem_data, size_t mem_size, size_t base_display_addr) {
    Sizes s;
    calcSizes(s, mem_size, base_display_addr);
    ImGui::SetNextWindowSize(ImVec2(s.windowWidth, s.windowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.windowWidth, FLT_MAX));

    open = true;
    if (ImGui::Begin(title, &open, ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            ImGui::OpenPopup("context");
        drawContents(mem_data, mem_size, base_display_addr);
        if (contentsWidthChanged) {
            calcSizes(s, mem_size, base_display_addr);
            ImGui::SetWindowSize(ImVec2(s.windowWidth, ImGui::GetWindowSize().y));
        }
    }
    ImGui::End();
}

// Memory Editor contents only
void MemoryEditor::drawContents(void *mem_data_void, size_t mem_size, size_t base_display_addr) {
    if (cols < 1)
        cols = 1;

    ImU8 *mem_data = (ImU8 *)mem_data_void;
    Sizes s;
    calcSizes(s, mem_size, base_display_addr);
    ImGuiStyle &style = ImGui::GetStyle();

    // We begin into our scrolling region with the 'ImGuiWindowFlags_NoMove' in order to prevent click from moving the window.
    // This is used as a facility since our main click detection code doesn't assign an ActiveId so the click would normally be caught as a window-move.
    const float height_separator = style.ItemSpacing.y;
    float       footer_height    = optFooterExtraHeight;
    if (optShowOptions)
        footer_height += height_separator + ImGui::GetFrameHeightWithSpacing() * 1;
    if (optShowDataPreview)
        footer_height += height_separator + ImGui::GetFrameHeightWithSpacing() * 1 + ImGui::GetTextLineHeightWithSpacing() * 3;
    ImGui::BeginChild("##scrolling", ImVec2(0, -footer_height), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav);
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    // We are not really using the clipper API correctly here, because we rely on visible_start_addr/visible_end_addr for our scrolling function.
    const int        line_total_count = (int)((mem_size + cols - 1) / cols);
    ImGuiListClipper clipper;
    clipper.Begin(line_total_count, s.lineHeight);

    bool data_next = false;

    if (readOnly || dataEditingAddr >= mem_size)
        dataEditingAddr = (size_t)-1;
    if (dataPreviewAddr >= mem_size)
        dataPreviewAddr = (size_t)-1;

    size_t preview_data_type_size = optShowDataPreview ? dataTypeGetSize(previewDataType) : 0;

    size_t data_editing_addr_next = (size_t)-1;
    if (dataEditingAddr != (size_t)-1) {
        // Move cursor but only apply on next frame so scrolling with be synchronized (because currently we can't change the scrolling while the window is being rendered)
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) && (ptrdiff_t)dataEditingAddr >= (ptrdiff_t)cols) {
            data_editing_addr_next = dataEditingAddr - cols;
        } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)) && (ptrdiff_t)dataEditingAddr < (ptrdiff_t)mem_size - cols) {
            data_editing_addr_next = dataEditingAddr + cols;
        } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) && (ptrdiff_t)dataEditingAddr > (ptrdiff_t)0) {
            data_editing_addr_next = dataEditingAddr - 1;
        } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)) && (ptrdiff_t)dataEditingAddr < (ptrdiff_t)mem_size - 1) {
            data_editing_addr_next = dataEditingAddr + 1;
        }
    }

    // Draw vertical separator
    ImVec2 window_pos = ImGui::GetWindowPos();
    draw_list->AddLine(ImVec2(window_pos.x + s.posAsciiStart - s.glyphWidth, window_pos.y), ImVec2(window_pos.x + s.posAsciiStart - s.glyphWidth, window_pos.y + 9999), ImGui::GetColorU32(ImGuiCol_Border));

    const ImU32 color_text     = ImGui::GetColorU32(ImGuiCol_Text);
    const ImU32 color_disabled = ImGui::GetColorU32(ImGuiCol_TextDisabled);

    const char *format_address    = "%0*" _PRISizeT "X: ";
    const char *format_data       = "%0*" _PRISizeT "X";
    const char *format_byte       = "%02X";
    const char *format_byte_space = "%02X ";

    while (clipper.Step()) {
        for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) { // display only visible lines
            size_t addr = (size_t)((uint64_t)line_i * (uint64_t)cols);
            ImGui::Text(format_address, s.addrDigitsCount, base_display_addr + addr);

            // Draw Hexadecimal
            for (int n = 0; n < cols && addr < mem_size; n++, addr++) {
                float byte_pos_x = s.posHexStart + s.hexCellWidth * n;
                if (optMidColsCount > 0)
                    byte_pos_x += (float)(n / optMidColsCount) * s.spacingBetweenMidCols;
                ImGui::SameLine(byte_pos_x);

                // Draw highlight
                bool is_highlight_from_user_range = (addr >= highlightMin && addr < highlightMax);
                bool is_highlight_from_user_func  = (highlightFn && highlightFn(mem_data, addr));
                bool is_highlight_from_preview    = (addr >= dataPreviewAddr && addr < dataPreviewAddr + preview_data_type_size);
                if (is_highlight_from_user_range || is_highlight_from_user_func || is_highlight_from_preview) {
                    ImVec2 pos                      = ImGui::GetCursorScreenPos();
                    float  highlight_width          = s.glyphWidth * 2;
                    bool   is_next_byte_highlighted = (addr + 1 < mem_size) && ((highlightMax != (size_t)-1 && addr + 1 < highlightMax) || (highlightFn && highlightFn(mem_data, addr + 1)));
                    if (is_next_byte_highlighted || (n + 1 == cols)) {
                        highlight_width = s.hexCellWidth;
                        if (optMidColsCount > 0 && n > 0 && (n + 1) < cols && ((n + 1) % optMidColsCount) == 0)
                            highlight_width += s.spacingBetweenMidCols;
                    }
                    draw_list->AddRectFilled(pos, ImVec2(pos.x + highlight_width, pos.y + s.lineHeight), highlightColor);
                }

                if (dataEditingAddr == addr) {
                    // Display text input on current byte
                    bool data_write = false;
                    ImGui::PushID((void *)addr);
                    if (dataEditingTakeFocus) {
                        ImGui::SetKeyboardFocusHere(0);
                        ImSnprintf(addrInputBuf, sizeof(addrInputBuf), format_data, s.addrDigitsCount, base_display_addr + addr);

                        int b = readFn ? readFn(mem_data, addr) : mem_data[addr];
                        if (b < 0)
                            ImSnprintf(dataInputBuf, sizeof(dataInputBuf), "-- ");
                        else
                            ImSnprintf(dataInputBuf, sizeof(dataInputBuf), format_byte, b);
                    }
                    struct UserData {
                        // FIXME: We should have a way to retrieve the text edit cursor position more easily in the API, this is rather tedious. This is such a ugly mess we may be better off not using InputText() at all here.
                        static int Callback(ImGuiInputTextCallbackData *data) {
                            UserData *user_data = (UserData *)data->UserData;
                            if (!data->HasSelection())
                                user_data->CursorPos = data->CursorPos;
                            if (data->SelectionStart == 0 && data->SelectionEnd == data->BufTextLen) {
                                // When not editing a byte, always refresh its InputText content pulled from underlying memory data
                                // (this is a bit tricky, since InputText technically "owns" the master copy of the buffer we edit it in there)
                                data->DeleteChars(0, data->BufTextLen);
                                data->InsertChars(0, user_data->CurrentBufOverwrite);
                                data->SelectionStart = 0;
                                data->SelectionEnd   = 2;
                                data->CursorPos      = 0;
                            }
                            return 0;
                        }
                        char CurrentBufOverwrite[3]; // Input
                        int  CursorPos;              // Output
                    };
                    UserData user_data;
                    user_data.CursorPos = -1;
                    {
                        int b = readFn ? readFn(mem_data, addr) : mem_data[addr];
                        if (b < 0)
                            ImSnprintf(user_data.CurrentBufOverwrite, sizeof(user_data.CurrentBufOverwrite), "--");
                        else
                            ImSnprintf(user_data.CurrentBufOverwrite, sizeof(user_data.CurrentBufOverwrite), format_byte, readFn ? readFn(mem_data, addr) : mem_data[addr]);
                    }
                    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_CallbackAlways;
                    flags |= ImGuiInputTextFlags_AlwaysOverwrite;
                    ImGui::SetNextItemWidth(s.glyphWidth * 2);
                    if (ImGui::InputText("##data", dataInputBuf, IM_ARRAYSIZE(dataInputBuf), flags, UserData::Callback, &user_data))
                        data_write = data_next = true;
                    else if (!dataEditingTakeFocus && !ImGui::IsItemActive())
                        dataEditingAddr = data_editing_addr_next = (size_t)-1;
                    dataEditingTakeFocus = false;
                    if (user_data.CursorPos >= 2)
                        data_write = data_next = true;
                    if (data_editing_addr_next != (size_t)-1)
                        data_write = data_next = false;
                    unsigned int data_input_value = 0;
                    if (data_write && sscanf(dataInputBuf, "%X", &data_input_value) == 1) {
                        if (writeFn)
                            writeFn(mem_data, addr, (ImU8)data_input_value);
                        else
                            mem_data[addr] = (ImU8)data_input_value;
                    }
                    ImGui::PopID();
                } else {
                    // NB: The trailing space is not visible but ensure there's no gap that the mouse cannot click on.
                    int b = readFn ? readFn(mem_data, addr) : mem_data[addr];

                    {
                        if (b < 0)
                            ImGui::TextDisabled("-- ");
                        else
                            ImGui::Text(format_byte_space, b);
                    }
                    if (!readOnly && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                        dataEditingTakeFocus   = true;
                        data_editing_addr_next = addr;
                    }
                }
            }

            // Draw ASCII values
            ImGui::SameLine(s.posAsciiStart);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            addr       = line_i * cols;
            ImGui::PushID(line_i);
            if (ImGui::InvisibleButton("ascii", ImVec2(s.posAsciiEnd - s.posAsciiStart, s.lineHeight))) {
                dataEditingAddr = dataPreviewAddr = addr + (size_t)((ImGui::GetIO().MousePos.x - pos.x) / s.glyphWidth);
                dataEditingTakeFocus              = true;
            }
            ImGui::PopID();
            for (int n = 0; n < cols && addr < mem_size; n++, addr++) {
                if (addr == dataEditingAddr) {
                    draw_list->AddRectFilled(pos, ImVec2(pos.x + s.glyphWidth, pos.y + s.lineHeight), ImGui::GetColorU32(ImGuiCol_FrameBg));
                    draw_list->AddRectFilled(pos, ImVec2(pos.x + s.glyphWidth, pos.y + s.lineHeight), ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                }
                unsigned char c         = readFn ? readFn(mem_data, addr) : mem_data[addr];
                char          display_c = (c < 32 || c >= 128) ? '.' : c;
                draw_list->AddText(pos, (display_c == c) ? color_text : color_disabled, &display_c, &display_c + 1);
                pos.x += s.glyphWidth;
            }
        }
    }
    ImGui::PopStyleVar(2);
    ImGui::EndChild();

    // Notify the main window of our ideal child content size (FIXME: we are missing an API to get the contents size from the child)
    ImGui::SetCursorPosX(s.windowWidth);

    if (data_next && dataEditingAddr + 1 < mem_size) {
        dataEditingAddr = dataPreviewAddr = dataEditingAddr + 1;
        dataEditingTakeFocus              = true;
    } else if (data_editing_addr_next != (size_t)-1) {
        dataEditingAddr = dataPreviewAddr = data_editing_addr_next;
        dataEditingTakeFocus              = true;
    }

    const bool lock_show_data_preview = optShowDataPreview;
    if (optShowOptions) {
        ImGui::Separator();
        drawOptionsLine(s, mem_data, mem_size, base_display_addr);
    }

    if (lock_show_data_preview) {
        ImGui::Separator();
        drawPreviewLine(s, mem_data, mem_size, base_display_addr);
    }
}

void MemoryEditor::drawOptionsLine(const Sizes &s, void *mem_data, size_t mem_size, size_t base_display_addr) {
    IM_UNUSED(mem_data);
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui::Checkbox("Data Preview", &optShowDataPreview);
    ImGui::SameLine(0, 30);

    ImGui::Text(
        "Jump to address (%0*" _PRISizeT "X..%0*" _PRISizeT "X)",
        s.addrDigitsCount, base_display_addr, s.addrDigitsCount, base_display_addr + mem_size - 1);
    ImGui::SameLine();
    ImGui::SetNextItemWidth((s.addrDigitsCount + 1) * s.glyphWidth + style.FramePadding.x * 2.0f);
    if (ImGui::InputText("##addr", addrInputBuf, IM_ARRAYSIZE(addrInputBuf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
        size_t goto_addr;
        if (sscanf(addrInputBuf, "%" _PRISizeT "X", &goto_addr) == 1) {
            gotoAddr     = goto_addr - base_display_addr;
            highlightMin = highlightMax = (size_t)-1;
        }
    }

    if (gotoAddr != (size_t)-1) {
        if (gotoAddr < mem_size) {
            ImGui::BeginChild("##scrolling");
            ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + (gotoAddr / cols) * ImGui::GetTextLineHeight());
            ImGui::EndChild();
            dataEditingAddr = dataPreviewAddr = gotoAddr;
            dataEditingTakeFocus              = true;
        }
        gotoAddr = (size_t)-1;
    }
}

void MemoryEditor::drawPreviewLine(const Sizes &s, void *mem_data_void, size_t mem_size, size_t base_display_addr) {
    IM_UNUSED(base_display_addr);
    ImU8       *mem_data = (ImU8 *)mem_data_void;
    ImGuiStyle &style    = ImGui::GetStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Preview as:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth((s.glyphWidth * 10.0f) + style.FramePadding.x * 2.0f + style.ItemInnerSpacing.x);
    if (ImGui::BeginCombo("##combo_type", dataTypeGetDesc(previewDataType), ImGuiComboFlags_HeightLargest)) {
        for (int n = 0; n <= ImGuiDataType_U32; n++)
            if (ImGui::Selectable(dataTypeGetDesc((ImGuiDataType)n), previewDataType == n))
                previewDataType = (ImGuiDataType)n;
        ImGui::EndCombo();
    }

    char  buf[128]  = "";
    float x         = s.glyphWidth * 6.0f;
    bool  has_value = dataPreviewAddr != (size_t)-1;
    if (has_value)
        drawPreviewData(dataPreviewAddr, mem_data, mem_size, previewDataType, DataFormat_Dec, buf, (size_t)IM_ARRAYSIZE(buf));
    ImGui::Text("Dec");
    ImGui::SameLine(x);
    ImGui::TextUnformatted(has_value ? buf : "N/A");
    if (has_value)
        drawPreviewData(dataPreviewAddr, mem_data, mem_size, previewDataType, DataFormat_Hex, buf, (size_t)IM_ARRAYSIZE(buf));
    ImGui::Text("Hex");
    ImGui::SameLine(x);
    ImGui::TextUnformatted(has_value ? buf : "N/A");
    if (has_value)
        drawPreviewData(dataPreviewAddr, mem_data, mem_size, previewDataType, DataFormat_Bin, buf, (size_t)IM_ARRAYSIZE(buf));
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    ImGui::Text("Bin");
    ImGui::SameLine(x);
    ImGui::TextUnformatted(has_value ? buf : "N/A");
}

// Utilities for Data Preview
const char *MemoryEditor::dataTypeGetDesc(ImGuiDataType data_type) const {
    const char *descs[] = {"Int8", "Uint8", "Int16", "Uint16", "Int32", "Uint32"};
    IM_ASSERT(data_type >= 0 && data_type <= ImGuiDataType_U32);
    return descs[data_type];
}

size_t MemoryEditor::dataTypeGetSize(ImGuiDataType data_type) const {
    const size_t sizes[] = {1, 1, 2, 2, 4, 4};
    IM_ASSERT(data_type >= 0 && data_type <= ImGuiDataType_U32);
    return sizes[data_type];
}

const char *MemoryEditor::formatBinary(const uint8_t *buf, int width) const {
    IM_ASSERT(width <= 64);
    size_t      out_n = 0;
    static char out_buf[64 + 8 + 1];
    int         n = width / 8;
    for (int j = n - 1; j >= 0; --j) {
        for (int i = 0; i < 8; ++i)
            out_buf[out_n++] = (buf[j] & (1 << (7 - i))) ? '1' : '0';
        out_buf[out_n++] = ' ';
    }
    IM_ASSERT(out_n < IM_ARRAYSIZE(out_buf));
    out_buf[out_n] = 0;
    return out_buf;
}

// [Internal]
void MemoryEditor::drawPreviewData(size_t addr, const ImU8 *mem_data, size_t mem_size, ImGuiDataType data_type, DataFormat data_format, char *out_buf, size_t out_buf_size) const {
    uint8_t buf[16];
    size_t  elem_size = dataTypeGetSize(data_type);
    size_t  size      = addr + elem_size > mem_size ? mem_size - addr : elem_size;
    if (readFn)
        for (int i = 0, n = (int)size; i < n; ++i)
            buf[i] = readFn(mem_data, addr + i);
    else
        memcpy(buf, mem_data + addr, size);

    if (data_format == DataFormat_Bin) {
        uint8_t binbuf[8];
        memcpy(binbuf, buf, size);
        ImSnprintf(out_buf, out_buf_size, "%s", formatBinary(binbuf, (int)size * 8));
        return;
    }

    out_buf[0] = 0;
    switch (data_type) {
        case ImGuiDataType_S8: {
            int8_t int8 = 0;
            memcpy(&int8, buf, size);
            if (data_format == DataFormat_Dec) {
                ImSnprintf(out_buf, out_buf_size, "%hhd", int8);
                return;
            }
            if (data_format == DataFormat_Hex) {
                ImSnprintf(out_buf, out_buf_size, "0x%02x", int8 & 0xFF);
                return;
            }
            break;
        }
        case ImGuiDataType_U8: {
            uint8_t uint8 = 0;
            memcpy(&uint8, buf, size);
            if (data_format == DataFormat_Dec) {
                ImSnprintf(out_buf, out_buf_size, "%hhu", uint8);
                return;
            }
            if (data_format == DataFormat_Hex) {
                ImSnprintf(out_buf, out_buf_size, "0x%02x", uint8 & 0XFF);
                return;
            }
            break;
        }
        case ImGuiDataType_S16: {
            int16_t int16 = 0;
            memcpy(&int16, buf, size);
            if (data_format == DataFormat_Dec) {
                ImSnprintf(out_buf, out_buf_size, "%hd", int16);
                return;
            }
            if (data_format == DataFormat_Hex) {
                ImSnprintf(out_buf, out_buf_size, "0x%04x", int16 & 0xFFFF);
                return;
            }
            break;
        }
        case ImGuiDataType_U16: {
            uint16_t uint16 = 0;
            memcpy(&uint16, buf, size);
            if (data_format == DataFormat_Dec) {
                ImSnprintf(out_buf, out_buf_size, "%hu", uint16);
                return;
            }
            if (data_format == DataFormat_Hex) {
                ImSnprintf(out_buf, out_buf_size, "0x%04x", uint16 & 0xFFFF);
                return;
            }
            break;
        }
        case ImGuiDataType_S32: {
            int32_t int32 = 0;
            memcpy(&int32, buf, size);
            if (data_format == DataFormat_Dec) {
                ImSnprintf(out_buf, out_buf_size, "%d", int32);
                return;
            }
            if (data_format == DataFormat_Hex) {
                ImSnprintf(out_buf, out_buf_size, "0x%08x", int32);
                return;
            }
            break;
        }
        case ImGuiDataType_U32: {
            uint32_t uint32 = 0;
            memcpy(&uint32, buf, size);
            if (data_format == DataFormat_Dec) {
                ImSnprintf(out_buf, out_buf_size, "%u", uint32);
                return;
            }
            if (data_format == DataFormat_Hex) {
                ImSnprintf(out_buf, out_buf_size, "0x%08x", uint32);
                return;
            }
            break;
        }
        default:
            break;
    }
    IM_ASSERT(0); // Shouldn't reach
}

#undef _PRISizeT
#undef ImSnprintf
