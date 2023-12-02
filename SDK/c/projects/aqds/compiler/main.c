#include "common.h"
#include "tokenizer.h"

#ifdef __SDCC
#include "screen.h"
#else
#include <sys/stat.h>
#endif

int8_t fd_out = -1;

void parse_file(const char *path) {
    printf("- Parsing %s\n", path);
    push_file(path);

    while (1) {
        int token = get_token();
        if (token == TOK_EOF)
            break;

        if (token == TOK_IDENTIFIER) {
            printf("  - Identifier: %s\n", tok_strval);
        } else if (token == TOK_CONSTANT) {
            printf("  - Constant: %d\n", tok_value);
        } else if (token == TOK_STRING_LITERAL) {
            printf("  - String literal: %s\n", tok_strval);
        } else {
            printf("  - Token: %d %c\n", token, token > ' ' ? token : ' ');
        }
    }

    printf("- Parsing %s done\n", cur_file_ctx->path);
    pop_file();
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

    // Determine base name (filename without extension) and path of assembler file (temporarily stored in linebuf)
    determine_basename(path);

    // Output header
    printf("Compiling %s\n", path);

    // Change directory
    if (*linebuf) {
        printf("- Changing directory to: %s\n", linebuf);
#ifndef __SDCC
        chdir(linebuf);
#else
        esp_chdir(linebuf);
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
    sprintf(linebuf, "out/%s.asm", basename);
    printf("- Creating assembly output: %s\n", linebuf);
    fd_out = esp_open(linebuf, FO_CREATE | FO_TRUNC | FO_WRONLY);
    check_esp_result(fd_out);

    // Parse source code
    parse_file(filename_cb);

    // Close files
    esp_close(fd_out);

    // We're done, exit the program
    puts("Done!\n");
    exit_program();
    return 0;
}
