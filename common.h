#pragma once

#if _WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#if _WIN32
#    include "getopt.h"
#    define strdup _strdup
#else
#    include <unistd.h>
#endif

#define CPU_FREQ (3579545)
