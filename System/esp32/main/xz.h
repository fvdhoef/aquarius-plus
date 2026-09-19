#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum xz_ret {
    XZ_SUCCESS,       // Operation finished successfully
    XZ_FORMAT_ERROR,  // File format was not recognized (wrong magic bytes)
    XZ_OPTIONS_ERROR, // This implementation doesn't support the requested compression options
    XZ_DATA_ERROR,    // Compressed data is corrupt
    XZ_BUF_ERROR,     // Output buffer too small
    XZ_INTERNAL_OK,   // Only used internally
};

struct xz_buf {
    const uint8_t *in;
    unsigned       in_pos;
    unsigned       in_size;
    uint8_t       *out;
    unsigned       out_pos;
    unsigned       out_size;
};

enum xz_ret xz_decompress(const uint8_t *in, int in_size, uint8_t *out);

#ifdef __cplusplus
}
#endif
