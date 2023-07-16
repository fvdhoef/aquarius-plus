#include <aqplus.h>

static struct stat st;
static uint8_t     filename[64];
static uint8_t    *tram = (uint8_t *)0x3000;
static uint8_t    *cram = (uint8_t *)0x3400;
static uint8_t    *tp;
static uint8_t    *cp;
static uint8_t     col;
static int         fileidx[36];

static const char *keys = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void draw_str(const char *str) {
    while (*str) {
        *(tp++) = *(str++);
    }
}

static void draw_window(const char *path) {
    tp             = tram;
    cp             = cram;
    col            = 0x60;
    uint8_t border = 0x60;

    *(tp++) = ' ';
    *(tp++) = '<';
    draw_str(path);
    *(tp++) = '>';
    while (tp < tram + 40) {
        *(tp++) = ' ';
    }
    while (cp < cram + 40) {
        *(cp++) = col;
    }

    tp = tram + 40;
    cp = cram + 40;

    col     = 0x70;
    *(tp++) = 0xDE;
    *(cp++) = border;
    for (uint8_t i = 0; i < 38; i++) {
        *(tp++) = 0xAC;
        *(cp++) = border;
    }
    *(tp++) = 0xCE;
    *(cp++) = border;

    for (uint8_t j = 0; j < 22; j++) {
        *(tp++) = 0xD6;
        *(cp++) = border;
        for (uint8_t i = 0; i < 38; i++) {
            *(tp++) = ' ';
            *(cp++) = col;
        }
        *(tp++) = 0xD6;
        *(cp++) = border;
    }

    *(tp++) = 0xCF;
    *(cp++) = border;
    for (uint8_t i = 0; i < 38; i++) {
        *(tp++) = 0xAC;
        *(cp++) = border;
    }
    *(tp++) = 0xDF;
    *(cp++) = border;
}

static void scandir(void) {
    getcwd(filename, sizeof(filename));
    draw_window(filename);

    for (int i = 0; i < 36; i++) {
        fileidx[i] = -1;
    }

    char *tpl = tram + 3 * 40 + 2;
    tp        = tpl;

    uint8_t idx  = 0;
    int     fidx = -1;

    int8_t dd = opendir(".");
    while (readdir(dd, &st, filename, sizeof(filename)) == 0) {
        fidx++;
        if (filename[0] == '.')
            continue;
        if ((st.attr & 1) == 0) {
            char *p  = filename;
            bool  ok = false;
            while (*p) {
                if (p[0] == '.' && (toupper(p[1]) == 'P') && (toupper(p[2]) == 'T') && (p[3] == '3') && p[4] == '\0') {
                    p[0] = '\0';

                    if (p - filename > 15) {
                        filename[15] = '\0';
                    }

                    ok = true;
                    break;
                }
                p++;
            }
            if (!ok) {
                continue;
            }
        }

        *(tp++) = keys[idx];
        *(tp++) = (st.attr & 1) ? '<' : ' ';
        draw_str(filename);
        *(tp++) = (st.attr & 1) ? '>' : ' ';

        fileidx[idx] = fidx;

        idx++;

        if (idx == 18) {
            tpl = tram + 3 * 40 + 20;
        } else {
            tpl += 40;
        }

        tp = tpl;

        if (idx == 36)
            break;
    }
    closedir(dd);

    tp = tram + 23 * 40 + 2;
    draw_str("0-Z = select file    RTN = dir up");
}

static void getfile(int idx) {
    int8_t dd = opendir(".");
    while (idx >= 0) {
        readdir(dd, &st, filename, sizeof(filename));
        idx--;
    }
    closedir(dd);
}

static void playsong(void) {
    void    *load_addr = getheap();
    uint16_t max_size  = 0xC000 - (uint16_t)load_addr;
    if (st.size > max_size)
        return;

    load_binary(filename, load_addr, max_size);

    pt3play_init(load_addr);
    // while (1) {
    //     // Wait for end-of-frame (line 216)
    //     video_wait_eof();

    //     // Play 1 frame of PT3 data. Break out of loop if end-of-song.
    // if (pt3play_play())
    // break;
    // }

    // Mute any remaining sound
    pt3play_mute();
}

int main(void) {
    // chdir("songs1");

    while (1) {
        scandir();

        while (1) {
            int ch;
            while ((ch = getchar()) == 0) {
            }

            if (ch == '\r') {
                chdir("..");
                break;
            }

            ch = toupper(ch);

            uint8_t idx = 0xFF;
            if (ch >= '0' && ch <= '9')
                idx = ch - '0';
            else if (ch >= 'A' && ch <= 'Z')
                idx = 10 + (ch - 'A');

            if (idx < 36 && fileidx[idx] >= 0) {
                getfile(fileidx[idx]);
                puts(filename);
                if (st.attr & 1) {
                    chdir(filename);
                } else {
                    playsong();
                }
                break;
            }
        }
    }

    // pt3play_init(ingarden_pt3);

    // while (1) {
    //     // Wait for end of frame (line 216)
    //     IO_VIRQLINE = 216;
    //     IO_IRQSTAT  = 2;
    //     while ((IO_IRQSTAT & 2) == 0) {
    //     }

    //     if (pt3play_play())
    //         break;
    // }
    // pt3play_mute();

    return 0;
}
