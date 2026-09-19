#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void aq8lua_shutdown(void);
void aq8lua_init(void);
bool aq8lua_run(const char *name, const void *buf, unsigned size);
void aq8lua_gameloop(void);

#ifdef __cplusplus
}
#endif
