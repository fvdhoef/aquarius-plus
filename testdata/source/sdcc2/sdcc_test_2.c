#include <stdio.h>
#include <stdint.h>
#include "regs.h"

int a = 5;
int b = 3;

int main(void) {
    int var = 1234;

    printf("Hello Aquarius!\n");

    printf("sizeof(int)=%u\n", sizeof(int));
    printf("sizeof(long)=%u\n", sizeof(long));
    printf("a=%u\n", a);
    printf("&a=%04x\n", &a);
    printf("b=%u\n", b);
    printf("&b=%04x\n", &b);
    printf("var=%u\n", var);
    printf("&var=%04x\n", &var);

    return 0;
}
