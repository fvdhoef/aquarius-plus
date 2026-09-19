#include "common.h"
#include "esp.h"

extern const unsigned char samples_bin[];
extern const unsigned int  samples_bin_len;

int main(void) {
    const uint32_t *p         = (const uint32_t *)samples_bin;
    unsigned        remaining = samples_bin_len / 4;

    PCM->rate = 918;

#if 0
    while (remaining) {
        while (PCM->status >= 1000);
        __asm volatile("nop");      // Why is this necessary??

        remaining--;
        __asm volatile("" ::: "memory");
        PCM->data = *(p++);
    }
#else
    while (remaining) {
        unsigned space = 1023 - PCM->status;
        if (space > remaining)
            space = remaining;

        remaining -= space;
        while (space--)
            PCM->data = *(p++);
    }

#endif

    return 0;
}
