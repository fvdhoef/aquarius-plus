#pragma once

#include "common.h"
#include "vfs.h"

#define MOUNT_POINT "/sdcard"

void sdcard_init(void);

extern struct vfs sdcard_vfs;
