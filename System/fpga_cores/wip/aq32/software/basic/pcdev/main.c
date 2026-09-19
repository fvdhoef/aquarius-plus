#include "basic.h"
#include "editor/editbuf.h"
#include "basic/bytecode/bytecode.h"
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

static uint8_t editbuf_buf[128 * 1024];
struct editbuf editbuf;

static struct termios oldt, newt;

static void term_init(void) {
    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

static void term_restore(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static void sigint_handler(int) {
    exit(1);
}

int main(int argc, const char **argv) {
    term_init();
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    signal(SIGINT, sigint_handler);
    atexit(term_restore);

    const char *filename = NULL;

    if (argc == 2) {
        filename = argv[1];
    } else {
        filename = "../test/test.bas";
    }

    editbuf_init(&editbuf, editbuf_buf, sizeof(editbuf_buf));
    if (!editbuf_load(&editbuf, filename)) {
        fprintf(stderr, "Error loading: %s\n", filename);
        return 1;
    }

    printf("Loaded %s: %u lines\n", filename, editbuf_get_line_count(&editbuf));

    printf("Compiling...\n");
    int result = basic_compile(&editbuf);
    if (result != 0) {
        puts(basic_get_error_str(result));
        exit(1);
    }

    bytecode_dump();
    printf("Running...\n");

    result = basic_run();
    if (result != 0) {
        puts(basic_get_error_str(result));
        exit(1);
    } else {
        printf("Program finished.\n");
    }
    return 0;
}
