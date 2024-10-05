#pragma once

#include "Common.h"

std::string          xzhDecompressToString(const void *inBuf, unsigned inSize);
std::vector<uint8_t> xzhDecompress(const void *inBuf, unsigned inSize);
