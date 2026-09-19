#include "common.h"
#include "esp.h"
#include "csr.h"

// extern const unsigned char samples_bin[];
// extern const unsigned int  samples_bin_len;

// uint64_t get_mcycle() {
//     uint32_t h, l;
//     while (1) {
//         h = csr_read(mcycleh);
//         l = csr_read(mcycle);
//         if (h == csr_read(mcycleh))
//             break;
//     }

//     return ((uint64_t)h << 32U) | (uint64_t)l;
// }

typedef union {
    struct {
        int8_t   lx, ly;
        int8_t   rx, ry;
        uint8_t  lt, rt;
        uint16_t buttons;
    } fields;
    uint64_t u64;
} gamepad_data_t;

static_assert(sizeof(gamepad_data_t) == 8);

int main(void) {

    uint64_t prev_keys = 0;
    while (1) {
        uint64_t keys = KEYS;
        if (prev_keys != keys) {
            prev_keys = keys;
            printf("%08X%08X\n", (unsigned)(keys >> 32), (unsigned)(keys & 0xFFFFFFFFU));
        }
    }

    // uint64_t prev = -1;
    // while (1) {
    //     gamepad_data_t data;
    //     data.u64 = GAMEPAD1;

    //     if (prev != data.u64) {
    //         prev = data.u64;
    //         printf(
    //             "l=%4d,%4d r=%4d,%4d lt=%3u rt=%3u buttons=%04x\n",
    //             data.fields.lx, data.fields.ly,
    //             data.fields.rx, data.fields.ry,
    //             data.fields.lt, data.fields.rt,
    //             data.fields.buttons);
    //     }
    // }

    // uint16_t prev = 0;
    // while (1) {
    //     uint16_t val = HANDCTRL;
    //     if (prev != val) {
    //         prev = val;
    //         printf("%04X\n", val);
    //     }
    // }

    return 0;
}
