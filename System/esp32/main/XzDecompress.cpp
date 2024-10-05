#include "XzDecompress.h"
#include "xz_decompress.h"

static const char *TAG = "XzDecompress";

static void xzError(char *x) {
    ESP_LOGE(TAG, "XZ decompression error: %s", x);
}

std::string xzhDecompressToString(const void *inBuf, unsigned inSize) {
    // Get uncompressed size from header
    auto outSize = *(uint32_t *)inBuf;
    auto inData  = (uint8_t *)((uintptr_t)inBuf + 4);
    inSize -= 4;

    std::string result;
    result.resize(outSize);

    int inUsed;
    if (xz_decompress(inData, inSize, nullptr, nullptr, (uint8_t *)result.data(), &inUsed, xzError) != 0) {
        return "";
    }
    return result;
}

std::vector<uint8_t> xzhDecompress(const void *inBuf, unsigned inSize) {
    // Get uncompressed size from header
    auto outSize = *(uint32_t *)inBuf;
    auto inData  = (uint8_t *)((uintptr_t)inBuf + 4);
    inSize -= 4;

    std::vector<uint8_t> result;
    result.resize(outSize);

    int inUsed;
    if (xz_decompress(inData, inSize, nullptr, nullptr, (uint8_t *)result.data(), &inUsed, xzError) != 0) {
        result.clear();
    }
    return result;
}
