#include "DisplayOverlay.h"
#include "FPGA.h"
#include "Menus.h"

class DisplayOverlayInt : public DisplayOverlay {
public:
    bool     overlayVisible = false;
    Menu    *currentMenu    = nullptr;
    uint16_t textBuf[1024];
    uint16_t palette[16] = {0xF111, 0xFF11, 0xF1F1, 0xFFF1, 0xF22E, 0xFF1F, 0xF3CC, 0xFFFF, 0xFCCC, 0xF3BB, 0xFC2C, 0xF419, 0xFFF7, 0xF2D4, 0xFB22, 0x0333};

    void init() override {
        xTaskCreate(_task, "dispovl", 8192, this, 1, nullptr);
    }

    static void _task(void *param) { static_cast<DisplayOverlayInt *>(param)->task(); }

    void clearScreen() override {
        for (int i = 0; i < 1024; i++) {
            textBuf[i] = 0xFF00;
        }
    }

    void drawBorder(int x, int y, int w, int h, unsigned colBorder, unsigned colFill, int selectedRow, unsigned colFillSel) {
        uint16_t col1 = makeAttr(colBorder, colFill) << 8;
        uint16_t col2 = makeAttr(colFill, colBorder) << 8;

        uint16_t col3 = makeAttr(colBorder, colFillSel) << 8;
        uint16_t col4 = makeAttr(colFillSel, colBorder) << 8;

        uint16_t *p = &textBuf[y * 40 + x];

        // Top border
        {
            uint16_t *p2 = p;
            *(p2++)      = col1 | 183;
            for (int i = 0; i < w - 2; i++)
                *(p2++) = col1 | 163;
            *(p2++) = col1 | 235;
            p += 40;
        }

        // Middle lines
        for (int j = 0; j < h - 2; j++) {

            uint16_t *p2 = p;
            *(p2++)      = ((j == selectedRow) ? col3 : col1) | 181;
            for (int i = 0; i < w - 2; i++)
                *(p2++) = ((j == selectedRow) ? col3 : col1) | ' ';
            *(p2++) = ((j == selectedRow) ? col4 : col2) | 181;
            p += 40;
        }

        // Bottom border
        {
            uint16_t *p2 = p;
            *(p2++)      = col1 | 245;
            for (int i = 0; i < w - 2; i++)
                *(p2++) = col2 | 175;
            *(p2++) = col1 | 250;
        }
    }

    void drawStr(int x, int y, uint8_t attr, const char *str) override {
        uint16_t   *pd = &textBuf[y * 40 + x];
        const char *ps = str;
        while (*ps) {
            *(pd++) = (attr << 8) | *ps;
            ps++;
        }
    }

    void drawFmt(int x, int y, uint8_t attr, const char *fmt, ...) override {
        char    tmp[41];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(tmp, sizeof(tmp), fmt, ap);
        va_end(ap);
        drawStr(x, y, attr, tmp);
    }

    void setVisible(bool show) override {
        overlayVisible = show;

        static const uint16_t emptyPal[16] = {0};
        FPGA::instance().setOverlayPalette(overlayVisible ? palette : emptyPal);
    }

    bool isVisible() override {
        return overlayVisible;
    }

    void render() override {
        FPGA::instance().setOverlayText(textBuf);
    }

    void task() {
        // Load font
        extern const uint8_t ovlFontStart[] asm("_binary_ovl_font_chr_start");
        FPGA::instance().setOverlayFont(ovlFontStart);

        overlayVisible = true;
        setVisible(overlayVisible);

        while (1) {
            getMainMenu()->show();

            overlayVisible = false;
            setVisible(overlayVisible);
        }
    }
};

DisplayOverlay *getDisplayOverlay() {
    static DisplayOverlayInt obj;
    return &obj;
}
