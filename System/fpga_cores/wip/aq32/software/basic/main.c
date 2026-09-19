#include "common.h"
#include "editor/editor.h"
#include "basic/basic.h"
#include "help/help.h"
#include "video_save.h"
#include "ctype2.h"

static void cmd_run_start(void);

static void cmd_view_output_screen();

static void cmd_help_index(void);
static void cmd_help_contents(void);
static void cmd_help_topic(void);
static void cmd_help_reopen(void);
static void cmd_help_about(void);

static struct editbuf editbuf;

static const struct menu_item menu_file_items[] = {
    {.title = "&New", .shortcut = KEY_MOD_CTRL | 'N', .status = "Removes currently loaded file from memory", .handler = cmd_file_new},
    {.title = "&Open...", .shortcut = KEY_MOD_CTRL | 'O', .status = "Loads new file into memory", .handler = cmd_file_open},
    {.title = "&Save", .shortcut = KEY_MOD_CTRL | 'S', .status = "Saves current file", .handler = cmd_file_save},
    {.title = "Save &As...", .shortcut = KEY_MOD_SHIFT | KEY_MOD_CTRL | 'S', .status = "Saves current file with specified name", .handler = cmd_file_save_as},
    {.title = "-"},
    {.title = "E&xit", .shortcut = KEY_MOD_CTRL | 'Q', .status = "Exits editor and returns to system", .handler = cmd_file_exit},
    {.title = NULL},
};

static const struct menu_item menu_edit_items[] = {
    {.title = "Cu&t", .shortcut = KEY_MOD_CTRL | 'X', .status = "Deletes and copies selected text to clipboard", .handler = cmd_edit_cut, .is_active_handler = cmd_edit_cut_copy_active},
    {.title = "&Copy", .shortcut = KEY_MOD_CTRL | 'C', .status = "Copies selected text to clipboard", .handler = cmd_edit_copy, .is_active_handler = cmd_edit_cut_copy_active},
    {.title = "&Paste", .shortcut = KEY_MOD_CTRL | 'V', .status = "Inserts clipboard contents at cursor", .handler = cmd_edit_paste},
    {.title = "-"},
    {.title = "&Select all", .shortcut = KEY_MOD_CTRL | 'A', .status = "Selects all text in document", .handler = cmd_edit_select_all},
    {.title = "-"},
    {.title = "&Find...", .shortcut = KEY_MOD_CTRL | 'F', .status = "Finds specified text", .handler = cmd_edit_find},
    {.title = "&Repeat Last Find", .shortcut = CH_F3, .status = "Finds next occurence of previously specified text", .handler = cmd_edit_find_next},
    // {.title = "&Replace", .shortcut = KEY_MOD_CTRL | 'H'},
    // {.title = "-"},
    // {.title = "Format document", .shortcut = KEY_MOD_SHIFT | KEY_MOD_ALT | 'F'},
    // {.title = "Format selection"},
    // {.title = "Trim trailing whitespace"},
    {.title = NULL},
};

static const struct menu_item menu_view_items[] = {
    {.title = "&Output Screen", .shortcut = CH_F4, .status = "Displays output screen", .handler = cmd_view_output_screen},
    {.title = NULL},
};

static const struct menu_item menu_run_items[] = {
    {.title = "&Start", .shortcut = CH_F5, .status = "Runs current program", .handler = cmd_run_start},
    {.title = NULL},
};

static const struct menu_item menu_help_items[] = {
    {.title = "&Index", .shortcut = KEY_MOD_SHIFT | CH_F1, .status = "Displays help index", .handler = cmd_help_index},
    {.title = "&Contents", .shortcut = KEY_MOD_CTRL | CH_F1, .status = "Display help table of contents", .handler = cmd_help_contents},
    {.title = "&Topic", .shortcut = CH_F1, .status = "Displays information about the keyword the cursor is on", .handler = cmd_help_topic},
    {.title = "&Reopen Last Topic", .shortcut = KEY_MOD_ALT | CH_F1, .status = "Display last help page", .handler = cmd_help_reopen},
    {.title = "-"},
    {.title = "&About...", .status = "Displays product version", .handler = cmd_help_about},
    {.title = NULL},
};

const struct menu menubar_menus[] = {
    {.title = "&File", .items = menu_file_items},
    {.title = "&Edit", .items = menu_edit_items},
    {.title = "&View", .items = menu_view_items},
    {.title = "&Run", .items = menu_run_items},
    {.title = "&Help", .items = menu_help_items},
    {.title = NULL},
};

static void wait_keypress(void) {
    int key;
    while ((key = KEYBUF) >= 0);
    while (1) {
        while ((key = KEYBUF) < 0);
        if ((key & KEY_IS_SCANCODE) == 0) {
            break;
        }
    }
}

static void show_basic_error(int result) {
    editor_set_cursor((location_t){basic_get_error_line(), 0});
    reinit_video();
    editor_redraw_screen();
    dialog_message("Error", basic_get_error_str(result));
}

static void cmd_run_start(void) {
    scr_status_msg("Compiling...");
    int result = basic_compile(&editbuf);
    if (result != 0) {
        show_basic_error(result);
        return;
    }
    restore_video();
    result = basic_run();
    save_video();
    if (result != 0) {
        show_basic_error(result);
        return;
    }

    scr_status_msg("Press any key to continue");
    int key;
    while ((key = KEYBUF) >= 0);
    while (1) {
        while ((key = KEYBUF) < 0);
        if ((key & KEY_IS_SCANCODE) == 0) {
            break;
        }
    }
    reinit_video();
}

static void cmd_view_output_screen(void) {
    restore_video();
    wait_keypress();
    reinit_video();
}

static void cmd_help_index(void) {
    help(HELP_TOPIC_INDEX);
}

static void cmd_help_contents(void) {
    help(HELP_TOPIC_TOC);
}

static bool get_keyword_under_cursor(char *keyword, size_t keyword_maxlen) {
    location_t loc_cursor = editor_get_cursor();

    const uint8_t *p_line;
    int            line_len = editbuf_get_line(&editbuf, loc_cursor.line, &p_line);
    if (line_len < 0 || loc_cursor.pos < 0 || loc_cursor.pos >= line_len)
        return false;

    const uint8_t *p_line_end = p_line + line_len;
    const uint8_t *p_cursor   = p_line + loc_cursor.pos;
    if (!is_alpha(p_cursor[0]) && p_cursor[0] != '$')
        return false;

    // Go to start of keyword
    while (p_cursor > p_line && is_alpha(p_cursor[-1]))
        p_cursor--;

    unsigned idx = 0;
    while (p_cursor < p_line_end && (is_alpha(p_cursor[0]) || p_cursor[0] == '$')) {
        if (idx >= keyword_maxlen - 1)
            return false;

        keyword[idx++] = p_cursor[0];
        if (p_cursor[0] == '$')
            break;
        p_cursor++;
    }
    keyword[idx] = 0;
    return true;
}

static void cmd_help_topic(void) {
    char keyword[16];
    if (!get_keyword_under_cursor(keyword, sizeof(keyword)))
        return;

    help(keyword);
    // dialog_message("Help topic", keyword);
}

static void cmd_help_reopen(void) {
    help_reopen();
}

static void cmd_help_about(void) {
    dialog_message("About", "Aquarius32 BASIC v" PROJECT_VERSION " by Frank van den Hoef");
}

static __attribute__((section(".noinit"))) uint8_t buf_edit[128 * 1024];

int main(int argc, const char **argv) {
    save_video();
    editbuf_init(&editbuf, buf_edit, sizeof(buf_edit));
    editor(&editbuf, (argc == 2) ? argv[1] : NULL);
    return 0;
}
