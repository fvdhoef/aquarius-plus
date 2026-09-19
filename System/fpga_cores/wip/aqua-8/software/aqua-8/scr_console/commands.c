#include "commands.h"
#include "state.h"
#include "esp.h"
#include "console.h"
#include "aq8lua/aq8lua.h"

void cmd_help(const char *topic) {
    if (topic[0] == 0) {
        console_putline("\fCCOMMANDS\f6");
        console_putline("\f7load\f6 <filename>   Load a cartridge");
        console_putline("\f7save\f6 <filename>   Save a cartridge");
        console_putline("\f7run\f6 (or Ctrl-R)   Run");
        console_putline("\f7resume\f6            Resume halted program");
        console_putline("\f7reboot\f6            Reboot the system");
        console_putline("\f7ls\f6                List directory");
        console_putline("\f7cd\f6 <dirname>      Change directory");
        console_putline("\f7cd\f6 ..             Go up a directory");
        console_putline("\f7mkdir\f6 <dirname>   Create directory");
        console_putline("\f7help\f6 <topic>      Get help on topic");
        console_putline("");
        console_putline("\f7Help topics:");
        console_putline("\fCgfx data audio system math lua");
        console_putline("");
        console_putline("Press \f7ESC\f6 to toggle editor view");
    } else {
        console_puts("\fDTopic '\f6");
        console_puts(topic);
        console_putline("\fD' not found");
    }
}

static void print_cwd(void) {
    char tmp[64];
    esp_getcwd(tmp, sizeof(tmp));
    console_puts("\fCDirectory: ");
    console_putline(tmp);
}

void cmd_ls(const char *args) {
    (void)args;
    bool list_all = (strcmp(args, "*") == 0);

    const char *path = "";
    int         dd   = esp_opendir(path);
    if (dd < 0) {
        console_putline("Error listing path");
        return;
    }

    game_state.color = 6;

    char fn[256];

    int lines_output = 1;
    print_cwd();

    struct esp_stat st;
    while (1) {
        int res = esp_readdir(dd, &st, fn, sizeof(fn));
        if (res < 0)
            break;

        unsigned fnlen = strlen(fn);

        bool hidden_file = true;
        if (st.attr & DE_ATTR_DIR) {
            hidden_file = false;
        } else if (fnlen >= 5 && strcmp(&fn[fnlen - 4], ".aq8") == 0) {
            hidden_file = false;
        }
        if (hidden_file && !list_all)
            continue;

        console_printf("%02u-%02u-%02u %02u:%02u ", ((st.date >> 9) + 80) % 100, (st.date >> 5) & 15, st.date & 31, (st.time >> 11) & 31, (st.time >> 5) & 63);
        if (st.attr & DE_ATTR_DIR) {
            console_puts("<DIR> ");
        } else {
            if (st.size <= 99999) {
                console_printf("%5lu ", st.size);
            } else if (st.size < 1024 * 1024) {
                console_printf("%4luK ", st.size >> 10);
            } else {
                console_printf("%4luM ", st.size >> 20);
            }
        }

        if (st.attr & DE_ATTR_DIR) {
            console_puts("\fE");
        } else if (hidden_file) {
            console_puts("\f5");
        }
        console_putline(fn);

        lines_output++;
        if (lines_output >= 22) {
            lines_output = 0;
            console_puts("\fC--MORE--\f6");
            console_getc();
            int ch;
            while ((ch = console_getc()) <= 0);
            console_puts("\r        \r");
            if (ch == 27)
                break;
        }
    }
    esp_closedir(dd);
}

void cmd_cd(const char *path) {
    if (path[0] != 0) {
        if (esp_chdir(path) < 0) {
            console_putline("Directory not found");
            return;
        }
    }
    print_cwd();
}

void cmd_load(const char *path) {
    unsigned path_len = strlen(path);
    if (path_len == 0) {
        console_putline("No name specified");
        return;
    }

    // Check for correct extension
    char tmp[256];
    if (path_len >= 5 && strcmp(&path[path_len - 4], ".aq8") == 0) {
        // Correct extension
        snprintf(tmp, sizeof(tmp), "%s", path);
    } else {
        // Add path
        snprintf(tmp, sizeof(tmp), "%s.aq8", path);
    }

    int result = state_load_cart(tmp);
    if (result < 0) {
        console_putline("Could not load");
    } else {
        console_printf("Loaded %s (%d chars)\r\n", tmp, result);
    }
}

void cmd_save(const char *path) {
    unsigned path_len = strlen(path);
    if (path_len == 0) {
        // Use existing name
        console_putline("No name specified");
        return;
    }

    // Check for correct extension
    char tmp[256];
    if (path_len >= 5 && strcmp(&path[path_len - 4], ".aq8") == 0) {
        // Correct extension
        snprintf(tmp, sizeof(tmp), "%s", path);
    } else {
        // Add path
        snprintf(tmp, sizeof(tmp), "%s.aq8", path);
    }

    int result = state_save_cart(tmp);
    if (result < 0) {
        console_putline("Save failed");
    } else {
        console_printf("Saved %s\r\n", tmp);
    }
}

void cmd_reboot(const char *args) {
    __irq_disable();
    ((void (*)(void))0)();
    while (1) {
    }
}

void cmd_run(const char *args) {
    (void)args;

    aq8lua_shutdown();
    aq8lua_init();

    const uint8_t *buf;
    unsigned       size = editbuf_get_buf(edit_state.code_edit.editbuf, &buf);
    if (aq8lua_run("C", buf, size)) {
        aq8lua_gameloop();
    }
}

void do_lua(const char *line) {
    aq8lua_init();
    aq8lua_run("I", line, strlen(line));
}
