#pragma once

#include <stdint.h>

// #define ALLOW_UNALIGNED
#ifdef ALLOW_UNALIGNED
static inline uint16_t read_u16(const uint8_t *p) { return *(uint16_t *)p; }
static inline uint32_t read_u32(const uint8_t *p) { return *(uint32_t *)p; }
static inline uint64_t read_u64(const uint8_t *p) { return *(uint64_t *)p; }
static inline void     write_u16(uint8_t *p, uint16_t val) { *(uint16_t *)p = val; }
static inline void     write_u32(uint8_t *p, uint32_t val) { *(uint32_t *)p = val; }
static inline void     write_u64(uint8_t *p, uint64_t val) { *(uint64_t *)p = val; }
#else
static inline uint16_t read_u16(const uint8_t *p) {
    return p[0] | (p[1] << 8);
}
static inline uint32_t read_u32(const uint8_t *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}
static inline uint64_t read_u64(const uint8_t *p) {
    return (
        (uint64_t)p[0] |
        ((uint64_t)p[1] << 8) |
        ((uint64_t)p[2] << 16) |
        ((uint64_t)p[3] << 24) |
        ((uint64_t)p[4] << 32) |
        ((uint64_t)p[5] << 40) |
        ((uint64_t)p[6] << 48) |
        ((uint64_t)p[7] << 56));
}
static inline void write_u16(uint8_t *p, uint16_t val) {
    p[0] = val & 0xFF;
    p[1] = (val >> 8) & 0xFF;
}
static inline void write_u32(uint8_t *p, uint32_t val) {
    p[0] = val & 0xFF;
    p[1] = (val >> 8) & 0xFF;
    p[2] = (val >> 16) & 0xFF;
    p[3] = (val >> 24) & 0xFF;
}
static inline void write_u64(uint8_t *p, uint64_t val) {
    p[0] = val & 0xFF;
    p[1] = (val >> 8) & 0xFF;
    p[2] = (val >> 16) & 0xFF;
    p[3] = (val >> 24) & 0xFF;
    p[4] = (val >> 32) & 0xFF;
    p[5] = (val >> 40) & 0xFF;
    p[6] = (val >> 48) & 0xFF;
    p[7] = (val >> 56) & 0xFF;
}
#endif
