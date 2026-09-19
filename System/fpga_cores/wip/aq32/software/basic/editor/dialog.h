#pragma once

#include "common.h"

bool dialog_open(char *filename, size_t filename_max);
bool dialog_save(char *filename, size_t filename_max);
int  dialog_confirm(const char *title, const char *text);
void dialog_message(const char *title, const char *text);
bool dialog_edit_field(const char *title, char *buf, size_t buf_size);
