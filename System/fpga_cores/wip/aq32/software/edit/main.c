#include "common.h"
#include "editor/editor.h"
#include "video_save.h"

static void cmd_help_about(void);

static struct editbuf                              editbuf;
static __attribute__((section(".noinit"))) uint8_t buf_edit[400 * 1024];

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

static const struct menu_item menu_help_items[] = {
    {.title = "&About...", .status = "Displays product version", .handler = cmd_help_about},
    {.title = NULL},
};

const struct menu menubar_menus[] = {
    {.title = "&File", .items = menu_file_items},
    {.title = "&Edit", .items = menu_edit_items},
    {.title = "&Help", .items = menu_help_items},
    {.title = NULL},
};

static void cmd_help_about(void) {
    dialog_message("About", "Aquarius32 text editor v" PROJECT_VERSION " by Frank van den Hoef");
}

int main(int argc, const char **argv) {
    save_video();
    editbuf_init(&editbuf, buf_edit, sizeof(buf_edit));
    editor(&editbuf, (argc == 2) ? argv[1] : NULL);
    return 0;
}
