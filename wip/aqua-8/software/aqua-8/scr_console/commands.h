#pragma once

#include "common.h"

void cmd_help(const char *args);
void cmd_ls(const char *args);
void cmd_cd(const char *args);
void cmd_load(const char *args);
void cmd_save(const char *args);
void cmd_reboot(const char *args);
void cmd_run(const char *args);

void do_lua(const char *line);
