// Modified from v0.50 of https://github.com/ocornut/imgui_club/blob/main/imgui_memory_editor/imgui_memory_editor.h

#pragma once

#include "Common.h"
#include "imgui.h"

struct MemoryEditor {
    MemoryEditor() {
    }

    struct Sizes {
        int   addrDigitsCount       = 0;
        float lineHeight            = 0.0f;
        float glyphWidth            = 0.0f;
        float hexCellWidth          = 0.0f;
        float spacingBetweenMidCols = 0.0f;
        float posHexStart           = 0.0f;
        float posHexEnd             = 0.0f;
        float posAsciiStart         = 0.0f;
        float posAsciiEnd           = 0.0f;
        float windowWidth           = 0.0f;

        Sizes() {
        }
    };

    enum DataFormat {
        DataFormat_Bin = 0,
        DataFormat_Dec = 1,
        DataFormat_Hex = 2,
        DataFormat_COUNT
    };

    void        gotoAddrAndHighlight(size_t addr_min, size_t addr_max);
    void        calcSizes(Sizes &s, size_t mem_size, size_t base_display_addr);
    void        drawWindow(const char *title, void *mem_data, size_t mem_size, size_t base_display_addr = 0x0000);
    void        drawContents(void *mem_data_void, size_t mem_size, size_t base_display_addr = 0x0000);
    void        drawOptionsLine(const Sizes &s, void *mem_data, size_t mem_size, size_t base_display_addr);
    void        drawPreviewLine(const Sizes &s, void *mem_data_void, size_t mem_size, size_t base_display_addr);
    const char *dataTypeGetDesc(ImGuiDataType data_type) const;
    size_t      dataTypeGetSize(ImGuiDataType data_type) const;
    const char *formatBinary(const uint8_t *buf, int width) const;
    void        drawPreviewData(size_t addr, const ImU8 *mem_data, size_t mem_size, ImGuiDataType data_type, DataFormat data_format, char *out_buf, size_t out_buf_size) const;

    // Settings
    bool  open                                        = true;                        // set to false when drawWindow() was closed. ignore if not using drawWindow().
    bool  readOnly                                    = false;                       // disable any editing.
    int   cols                                        = 16;                          // number of columns to display.
    bool  optShowOptions                              = true;                        // display options button/context menu. when disabled, options will be locked unless you provide your own UI for them.
    bool  optShowDataPreview                          = false;                       // display a footer previewing the decimal/binary/hex/float representation of the currently selected bytes.
    int   optMidColsCount                             = 8;                           // set to 0 to disable extra spacing between every mid-cols.
    int   optAddrDigitsCount                          = 0;                           // number of addr digits to display (default calculated based on maximum displayed addr).
    float optFooterExtraHeight                        = 0.0f;                        // space to reserve at the bottom of the widget to add custom widgets
    ImU32 highlightColor                              = IM_COL32(255, 255, 255, 50); // background color of highlighted bytes.
    int (*readFn)(const ImU8 *data, size_t off)       = nullptr;                     // optional handler to read bytes.
    void (*writeFn)(ImU8 *data, size_t off, ImU8 d)   = nullptr;                     // optional handler to write bytes.
    bool (*highlightFn)(const ImU8 *data, size_t off) = nullptr;                     // optional handler to return Highlight property (to support non-contiguous highlighting).

    // [Internal State]
    bool          contentsWidthChanged = false;
    size_t        dataPreviewAddr      = (size_t)-1;
    size_t        dataEditingAddr      = (size_t)-1;
    bool          dataEditingTakeFocus = false;
    char          dataInputBuf[32]     = {0};
    char          addrInputBuf[32]     = {0};
    size_t        gotoAddr             = (size_t)-1;
    size_t        highlightMin         = (size_t)-1;
    size_t        highlightMax         = (size_t)-1;
    ImGuiDataType previewDataType      = ImGuiDataType_U16;
};
