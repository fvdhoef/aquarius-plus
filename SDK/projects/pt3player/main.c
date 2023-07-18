/*
PT3 Player (SDCC version)
by Frank van den Hoef

Abstract - 
PT3 Player builds on the original application design from Bruce Abbott for the Aquarius Micro Expander for an easy-to-use AY music player, allowing users to easily navigate the folders of PT3 files and simply start playing music. It featured a simple "equalizer" visualization that reacted to the amplitude value for each of the music tracks. The new version for the Aquarius+ rewrites the original Z80 assembly code as C code, designed for the SDCC compiler, and reworks the CH376 USB interface code to utilize the Aquarius+ ESP32-managed SD card storage.

Revision History -
2023-07-16 - v0.1  - Initial version, FvdH

*/

// The aqplus.h file collects all the necessary components for C development on the Aquarius+ using the SDCC compiler into a single file. It is located in the SDK > include folder.
#include <aqplus.h>

// Here we're defining our "global" program variables, used in many of the functions/methods below.
static struct stat st;                              // This custom data structure is used to hold the STATUS of the ESP32 SD device
static uint8_t     filename[64];                    // This array of unsigned 8-bit integers stores the characters of the filename for the current song
static uint8_t    *tram = (uint8_t *)0x3000;        // This is a pointer to the RAM location for the CHARRAM, referenced by offset from the starting point
static uint8_t    *cram = (uint8_t *)0x3400;        // This is a pointer to the RAM location for the COLRAM, referenced by offset from the starting point
static uint8_t    *tp;                              // This is a pointer to the current position for the CHARRAM
static uint8_t    *cp;                              // This is a pointer to the current position for the COLRAM
static uint8_t     col;                             // This is an unsigned 8-bit integer that stores the currently used COLOR
static int         fileidx[36];                     // This array of integers stores up to 36 song names in the current drive path, referenced by keys 0-9 and A-Z

// This function draws characters to the screen
static void draw_str(const char *str) {             // Take the stream of char values indicated by the pointer value passed in...
    while (*str) {                                  // ...if it's not zero...
        *(tp++) = *(str++);                         // ...write it to the current CHARRAM position, then increment both the position and the next char pointers
    }
}

// This function draws a blank application "window" to Aquarius+ screen RAM.
static void draw_window(const char *path) {         // Take the pathname as a series of chars.
    tp             = tram;                          // Set the text position to the start of CHARRAM.
    cp             = cram;                          // Set the color position to the start of COLRAM.
    col            = 0x60;                          // Set the color to BG = BLACK, FG = CYAN.
    uint8_t border = 0x60;                          // Set the border to BG = BLACK, FG = CYAN.

    *(tp++) = ' ';                                  // Put a SPACE char into the text position and increment. This also sets the BORDER character.
    *(tp++) = '<';                                  // Put a < char into the text position and increment.
    draw_str(path);                                 // Insert the path name using the draw_str function.
    *(tp++) = '>';                                  // Put a > char into the text position and increment.

    while (tp < tram + 40) {                        // Start in a loop to fill the rest of the row with SPACES...
        *(tp++) = ' ';                              // ...set the current position CHAR to SPACE...
    }                                               // ...and loop until it's the end of the top line of CHARS.

    while (cp < cram + 40) {                        // Now do a similar loop for the color of the top line...
        *(cp++) = col;                              // ...filling it with CYAN on BLACK (set above)...
    }                                               // ...and also loop until it's the end of the top line of COLOR.

    tp = tram + 40;                                 // Reset the text position to the start of the second row.
    cp = cram + 40;                                 // Reset the color position to the start of the second row.

    col     = 0x70;                                 // Set the color to BG = BLACK, FG = WHITE
    *(tp++) = 0xDE;                                 // Set the CHAR at the current text position to the UPPER LEFT corner character (222)
    *(cp++) = border;                               // Set this CHAR to the border color (CYAN on BLACK).

    for (uint8_t i = 0; i < 38; i++) {              // Run a loop to draw the horizontal line across the top of the screen.
        *(tp++) = 0xAC;                             // Set the CHAR at the current text position to the HORIZONTAL line character (172)
        *(cp++) = border;                           // Set this CHAR to the border color (CYAN on BLACK).
    }

    *(tp++) = 0xCE;                                 // Set the CHAR at the current text position to the UPPER RIGHT corner character (206)
    *(cp++) = border;                               // Set this CHAR to the border color (CYAN on BLACK).

    for (uint8_t j = 0; j < 22; j++) {              // Run a loop to draw the side vertical lines down the screen.
        *(tp++) = 0xD6;                             // Set the CHAR at the current text position to the VERTICAL line character (214).
        *(cp++) = border;                           // Set this CHAR to the border color (CYAN on BLACK).
        for (uint8_t i = 0; i < 38; i++) {          // Run this inner loop to fill the middle section with blanks...
            *(tp++) = ' ';                          // ...set the CHAR to be a space, and increment to the next text position...
            *(cp++) = col;                          // ...and set the COLOR to be CYAN on BLACK.
        }
        *(tp++) = 0xD6;                             // Set the CHAR at the current text position to the VERTICAL line character (214).
        *(cp++) = border;                           // Set this CHAR to the border color (CYAN on BLACK).
    }

    *(tp++) = 0xCF;                                 // Set the CHAR at the current text position to the LOWER LEFT corner character (207)
    *(cp++) = border;                               // Set this CHAR to the border color (CYAN on BLACK).

    for (uint8_t i = 0; i < 38; i++) {              // Run a loop to draw the horizontal line across the bottom of the screen.
        *(tp++) = 0xAC;                             // Set the CHAR at the current text position to the HORIZONTAL line character (172)
        *(cp++) = border;                           // Set this CHAR to the border color (CYAN on BLACK).
    }

    *(tp++) = 0xDF;                                 // Set the CHAR at the current text position to the LOWER RIGHT corner character (223)
    *(cp++) = border;                               // Set this CHAR to the border color (CYAN on BLACK).
}

// This method reads the list of PT3 files in the current path and creates a list of those files (up to 36 max) on screen.
static void scandir(void) {
    getcwd(filename, sizeof(filename));             // 
    draw_window(filename);                          // Redraw a blank window with the current path

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
                if (p[0] == '.' && 
                    (toupper(p[1]) == 'P') && 
                    (toupper(p[2]) == 'T') && 
                    (p[3] == '3') && 
                    p[4] == '\0') {
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

        char key;
        if (idx < 10) {
            key = '0' + idx;
        } else {
            key = 'A' + idx - 10;
        }
        *(tp++) = key;

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

static uint8_t playsong(void) {
    void    *load_addr = getheap();
    uint16_t max_size  = 0xC000 - (uint16_t)load_addr;
    if (st.size > max_size)
        return 0;

    draw_window(filename);
    tp = tram + 23 * 40 + 2;
    draw_str("SPACE = next song    RTN = stop song");

    uint8_t result = KEY_SPACE;

    load_binary(filename, load_addr, max_size);
    pt3play_init(load_addr);
    while (1) {
        // Wait for end-of-frame (line 216)
        video_wait_eof();

        // Play 1 frame of PT3 data. Break out of loop if end-of-song.
        if (pt3play_play())
            break;

        kb_scan();
        if (kb_pressed(KEY_SPACE)) {
            result = KEY_SPACE;
            break;
        }

        if (kb_pressed(KEY_RETURN)) {
            result = KEY_RETURN;
            break;
        }

        tp = tram + 40 * 4 + 4;

        // This section updates the visualization graphics
        {
            uint8_t *cp = cram + 19 * 40 + 13;

            uint8_t va = pt3play_ayregs.ampl_a;
            uint8_t vb = pt3play_ayregs.ampl_b;
            uint8_t vc = pt3play_ayregs.ampl_c;

            for (uint8_t i = 1; i < 16; i++) {
                uint8_t col;

                col   = (va >= i) ? 0x11 : 0xFF;
                cp[0] = col;
                cp[1] = col;
                cp[2] = col;

                col   = (vb >= i) ? 0x22 : 0xFF;
                cp[5] = col;
                cp[6] = col;
                cp[7] = col;

                col    = (vc >= i) ? 0x44 : 0xFF;
                cp[10] = col;
                cp[11] = col;
                cp[12] = col;

                cp -= 40;
            }
        }
    }

    // Mute any remaining sound
    pt3play_mute();

    return result;
}

int main(void) {
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
                // puts(filename);
                if (st.attr & 1) {
                    chdir(filename);
                } else {
                    while (playsong() == KEY_SPACE) {
                        idx++;
                        if (idx >= 36 || fileidx[idx] < 0)
                            idx = 0;
                        getfile(fileidx[idx]);
                    }
                }
                break;
            }
        }
    }
    return 0;
}
