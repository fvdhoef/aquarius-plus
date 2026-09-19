#include "common.h"
#include "tokenizer.h"
#include "parser.h"
#include "symbols.h"

#ifdef __SDCC
#include "screen.h"
#else
#include <sys/stat.h>
#endif

static int8_t fd_out = -1;

#ifdef __SDCC
static char *get_pgm_path(void) __naked { __asm__("jp 0xF809"); }
static char *get_pgm_arg(void) __naked { __asm__("jp 0xF80C"); }
#endif

void output_puts(const char *str, int len) {
    if (len <= 0)
        len = strlen(str);
    esp_write(fd_out, str, len);
}

int main(
#ifdef __SDCC
    void
#else
    int argc, const char **argv
#endif
) {
#ifdef __SDCC
    // Initialize screen output
    scr_init();
#endif
    const char *path;

    puts("Aquarius+ Development Studio - C-flat (Cb) Compiler V0.2 by Frank van den Hoef\n");

#ifndef __SDCC
    // Get path from command line argument
    if (argc != 2) {
        printf("Syntax: compiler <inputfile.cb>\n");
        exit(1);
    }
    path = argv[1];
#else
    // Get path
    path = get_pgm_arg();
#endif

    // Determine base name (filename without extension) and path of assembler file (temporarily stored in tmpbuf)
    determine_basename(path);

    // Output header
    printf("Compiling %s\n", path);

    // Change directory
    if (*tmpbuf) {
        printf("- Changing directory to: %s\n", tmpbuf);
#ifndef __SDCC
        chdir(tmpbuf);
#else
        esp_chdir(tmpbuf);
#endif
    }

    // Create output directory
    printf("- Creating output directory: out\n");
#ifndef __SDCC
    mkdir("out", 0777);
#else
    esp_mkdir("out");
#endif

    // Open assembly output
    sprintf(tmpbuf, "out/%s.asm", basename);
    printf("- Creating assembly output: %s\n", tmpbuf);
    fd_out = esp_open(tmpbuf, FO_CREATE | FO_TRUNC | FO_WRONLY);
    check_esp_result(fd_out);

    // Parse source code
    push_file(filename_cb);
    parse();

    // Close files
    esp_close(fd_out);

#ifdef DEBUG_OUTPUT
    symbols_dump();
#endif

    // We're done, exit the program
    puts("Done!\n");
#ifdef __SDCC
    {
        puts("\nPress enter to start assembler.\n");
        while (1) {
            uint8_t scancode = IO_KEYBUF;
            if (scancode == '\r')
                break;
        }

        const char *pgm = "/aqds/assembler.bin";
        memcpy(get_pgm_path(), pgm, strlen(pgm) + 1);

        sprintf(tmpbuf, "out/%s.asm", basename);
        memcpy(get_pgm_arg(), tmpbuf, 128);
        __asm__("jp 0xF803");
    }
#endif
    return 0;
}
