#pragma once

#define csr_swap(csr, val) ({                                                  \
    unsigned long __v = (unsigned long)(val);                                  \
    __asm__ __volatile__("csrrw%i1 %0, " #csr ", %1" : "=rK"(__v) : "r"(__v)); \
    __v;                                                                       \
})

#define csr_read(csr) ({                                \
    register unsigned long __v;                         \
    __asm__ __volatile__("csrr %0, " #csr : "=r"(__v)); \
    __v;                                                \
})

#define csr_write(csr, val) ({                                  \
    unsigned long __v = (unsigned long)(val);                   \
    __asm__ __volatile__("csrw%i0 " #csr ", %0" : : "rK"(__v)); \
})

#define csr_read_set(csr, val) ({                                              \
    unsigned long __v = (unsigned long)(val);                                  \
    __asm__ __volatile__("csrrs%i1 %0, " #csr ", %1" : "=r"(__v) : "rK"(__v)); \
    __v;                                                                       \
})

#define csr_set(csr, val) ({                                    \
    unsigned long __v = (unsigned long)(val);                   \
    __asm__ __volatile__("csrs%i0 " #csr ", %0" : : "rK"(__v)); \
})

#define csr_read_clear(csr, val) ({                                            \
    unsigned long __v = (unsigned long)(val);                                  \
    __asm__ __volatile__("csrrc%i1 %0, " #csr ", %1" : "=r"(__v) : "rK"(__v)); \
    __v;                                                                       \
})

#define csr_clear(csr, val) ({                                  \
    unsigned long __v = (unsigned long)(val);                   \
    __asm__ __volatile__("csrc%i0 " #csr ", %0" : : "rK"(__v)); \
})
