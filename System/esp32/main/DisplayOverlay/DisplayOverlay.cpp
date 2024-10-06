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
        uint16_t col         = makeAttr(colBorder, colFill) << 8;
        uint16_t colSelected = makeAttr(colBorder, colFillSel) << 8;

        uint16_t *p = &textBuf[y * 40 + x];

        // Top border
        {
            uint16_t *p2 = p;
            *(p2++)      = col | 16;
            for (int i = 0; i < w - 2; i++)
                *(p2++) = col | 17;
            *(p2++) = col | 18;
            p += 40;
        }

        // Middle lines
        for (int j = 0; j < h - 2; j++) {

            uint16_t *p2 = p;
            *(p2++)      = ((j == selectedRow) ? colSelected : col) | 19;
            for (int i = 0; i < w - 2; i++)
                *(p2++) = ((j == selectedRow) ? colSelected : col) | ' ';
            *(p2++) = ((j == selectedRow) ? colSelected : col) | 20;
            p += 40;
        }

        // Bottom border
        {
            uint16_t *p2 = p;
            *(p2++)      = col | 21;
            for (int i = 0; i < w - 2; i++)
                *(p2++) = col | 22;
            *(p2++) = col | 23;
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

    void fill(int x, int y, int w, int h, uint8_t attr, uint8_t ch) override {
        uint16_t *pd = &textBuf[y * 40 + x];

        for (int j = 0; j < h; j++) {
            auto pd2 = pd;
            for (int i = 0; i < w; i++)
                *(pd2++) = (attr << 8) | ch;
            pd += 40;
        }
    }

    void setAttr(int x, int y, uint8_t attr) override {
        textBuf[y * 40 + x] = (attr << 8) | (textBuf[y * 40 + x] & 0xFF);
    }

    void setVisible(bool show) override {
        overlayVisible = show;

        static const uint16_t emptyPal[16] = {0};
        getFPGA()->setOverlayPalette(overlayVisible ? palette : emptyPal);
    }

    bool isVisible() override {
        return overlayVisible;
    }

    void render() override {
        getFPGA()->setOverlayText(textBuf);
    }

    void task() {
        // Load font
        extern const uint8_t ovlFontStart[] asm("_binary_ovl_font_chr_start");
        getFPGA()->setOverlayFont(ovlFontStart);

        overlayVisible = false;
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
