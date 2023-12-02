#include "common.h"
#include "tokenizer.h"
#include "parser.h"

#ifdef __SDCC
#include "screen.h"
#else
#include <sys/stat.h>
#endif

static int8_t fd_out = -1;

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

    puts("Aquarius+ Development Studio - C-flat (Cb) Compiler V1.0 by Frank van den Hoef\n");

#ifndef __SDCC
    // Get path from command line argument
    if (argc != 2) {
        printf("Syntax: compiler <inputfile.cb>\n");
        exit(1);
    }
    path = argv[1];
#else
    // Path is located in buffer at $FF00
    path = (const char *)0xFF00;
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

    // We're done, exit the program
    puts("Done!\n");
    exit_program();
    return 0;
}
