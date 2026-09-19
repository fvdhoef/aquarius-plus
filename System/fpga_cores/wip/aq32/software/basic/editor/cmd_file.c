#include "editor.h"
#include "editor_internal.h"
#include "esp.h"

static bool check_modified(void) {
    if (!editbuf_get_modified(state.editbuf))
        return true;

    int result = dialog_confirm(NULL, "Save the changes made to the current document?");
    if (result < 0) {
        return false;
    }
    if (result > 0) {
        // Save document
        cmd_file_save();
        if (editbuf_get_modified(state.editbuf))
            return false;
    }
    return true;
}

static int load_file(const char *path) {
    scr_status_msg("Loading file...");

    reset_state();
    if (!editbuf_load(state.editbuf, path)) {
        dialog_message("Error", "Error loading file!");
        return -1;
    }
    strncpy(state.filename, path, sizeof(state.filename));
    return 0;
}

static void save_file(const char *path) {
    scr_status_msg("Saving file...");

    if (!editbuf_save(state.editbuf, path)) {
        dialog_message("Error", "Error saving file!");
        return;
    }
    snprintf(state.filename, sizeof(state.filename), "%s", path);
}

void cmd_file_new(void) {
    if (check_modified())
        reset_state();
}

void cmd_file_open(void) {
    char tmp[256];
    if (dialog_open(tmp, sizeof(tmp))) {
        editor_redraw_screen();
        if (check_modified()) {
            scr_status_msg("Loading file...");
            load_file(tmp);
        }
    }
}

void cmd_file_save(void) {
    if (!state.filename[0]) {
        // File never saved, use 'save as' command instead.
        cmd_file_save_as();
        return;
    }
    if (!editbuf_get_modified(state.editbuf))
        return;

    save_file(state.filename);
}

void cmd_file_save_as(void) {
    char tmp[64];
    strcpy(tmp, state.filename);
    if (!dialog_save(tmp, sizeof(tmp)))
        return;

    bool do_save = true;

    struct esp_stat st;
    if (esp_stat(tmp, &st) == 0) {
        // File exists
        editor_redraw_screen();
        if (dialog_confirm(NULL, "Overwrite existing file?") <= 0)
            do_save = false;
    }

    if (do_save) {
        save_file(tmp);
    }
}

#include "video_save.h"

void cmd_file_exit(void) {
    if (check_modified()) {
        restore_video_text();
        exit(0);
    }
}
