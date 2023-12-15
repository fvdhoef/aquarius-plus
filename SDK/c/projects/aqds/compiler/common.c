#include "common.h"
#include "tokenizer.h"

#define MAX_FILE_DEPTH 8
static struct file_ctx file_ctxs[MAX_FILE_DEPTH];
static uint8_t         file_ctx_idx;
struct file_ctx       *cur_file_ctx;

char        basename[32];
const char *filename_cb;
char        tmpbuf[256];

void exit_program(void) {
#ifdef __SDCC
    puts("\nPress enter to quit.\n");
    while (1) {
        uint8_t scancode = IO_KEYBUF;
        if (scancode == '\r')
            break;
    }

    // Go back to file manager
    __asm__("jp 0xF806");
#else
    exit(1);
#endif
}

#ifdef __SDCC
void *malloc(size_t size) {
    static uint8_t *endp;
    if (endp == NULL) {
        endp = getheap();
    }

    uint16_t remaining = (uint8_t *)0xF000 - endp;
    if (size > remaining) {
        error_out_of_memory();
    }

    uint8_t *result = endp;
    endp += size;
    return result;
}
#endif

void error(const char *fmt, ...) {
    if (cur_file_ctx)
        printf("\n%s:%u Error: ", cur_file_ctx->path, cur_file_ctx->linenr);
    else
        printf("\nError: ");

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    printf("\n");
    exit_program();
}

void error_out_of_memory(void) { error("Out of memory"); }
void error_syntax(void) { error("Syntax error"); }
void error_eof(void) { error("Unexpected end-of-file"); }
void error_sym_not_found(const char *name) { error("Symbol '%s' not found", name); }

void check_esp_result(int16_t result) {
    if (result < 0) {
        if (cur_file_ctx)
            printf("\n%s:%u Error: ", cur_file_ctx->path, cur_file_ctx->linenr);
        else
            printf("\nError: ");

        switch (result) {
            case ERR_NOT_FOUND: puts("File / directory not found"); break;
            case ERR_TOO_MANY_OPEN: puts("Too many open files / directories"); break;
            case ERR_PARAM: puts("Invalid parameter"); break;
            case ERR_EOF: puts("End of file / directory"); break;
            case ERR_EXISTS: puts("File already exists"); break;
            default:
            case ERR_OTHER: puts("Other puts"); break;
            case ERR_NO_DISK: puts("No disk"); break;
            case ERR_NOT_EMPTY: puts("Not empty"); break;
            case ERR_WRITE_PROTECT: puts("Write protected SD-card"); break;
        }
        exit_program();
    }
}

void determine_basename(const char *path) {
    filename_cb             = NULL;
    int         len         = strlen(path);
    const char *p           = path + len;
    const char *base_startp = NULL;
    const char *base_endp   = NULL;

    // Find start of extension
    while (p != path) {
        p--;
        if (*p == '/' || *p == '\\' || *p == '.')
            break;
    }
    if (p == path) {
        // No extension, no directories
        base_startp = path;
        base_endp   = path + len;
    } else if (*p == '/' || *p == '\\') {
        // No extension, has directories
        base_startp = p + 1;
        base_endp   = path + len;
    } else if (*p == '.') {
        base_endp = p;
        while (p != path) {
            p--;
            if (*p == '/' || *p == '\\') {
                base_startp = p + 1;
                break;
            }
        }
        if (!base_startp) {
            base_startp = path;
        }
    }

    filename_cb = base_startp;
    p           = base_startp;
    char *pd    = basename;
    for (int i = 0; i < (int)sizeof(basename) - 1; i++) {
        if (p < base_endp) {
            *(pd++) = *(p++);
        }
    }
    *pd = 0;

    p  = path;
    pd = tmpbuf;
    while (p < base_startp) {
        *(pd++) = *(p++);
    }
    *pd = 0;
}

void push_file(const char *path) {
    printf("- Opening %s\n", path);

    // Open file and check for success
    int8_t fd = esp_open(path, FO_RDONLY);
    check_esp_result(fd);

    // Keep track of file context for error reporting (line number and file path)
    {
        if (cur_file_ctx != NULL) {
            file_ctx_idx++;
            if (file_ctx_idx >= MAX_FILE_DEPTH) {
                error("Max file depth reached");
            }
        }
        cur_file_ctx         = &file_ctxs[file_ctx_idx];
        cur_file_ctx->linenr = 0;
        cur_file_ctx->fd     = fd;

        // Copy path into file context
        char *p = cur_file_ctx->path;
        for (uint8_t i = 0; i < sizeof(cur_file_ctx->path) - 1; i++) {
            char ch = path[i];
            if (ch == 0)
                break;
            *(p++) = ch;
        }
        *p = 0;
    }
}

bool pop_file(void) {
    // Close file
    printf("- Closing %s\n", cur_file_ctx->path);
    esp_close(cur_file_ctx->fd);

    // Pop file context
    if (file_ctx_idx == 0) {
        cur_file_ctx = NULL;
        return true;
    } else {
        cur_file_ctx = &file_ctxs[--file_ctx_idx];
    }
    return false;
}

void expect_tok(uint8_t token) {
    if (get_token() != token) {
        if (token > ' ')
            error("Expected token '%c'\n", token);
        else
            error("Expected token %u\n", token);
    }
}

void expect_tok_ack(uint8_t token) {
    expect_tok(token);
    ack_token();
}
