/*
      Program: sdcc_test.c
  Compiles to: build/sdcc_test.caq
    Execution: run "sdcc_test.caq"

      Authors: Frank van der Hoef
               Sean Harrington

     Abstract: This program demonstrates a simple C program that creates four different types,
               then prints their value, address location (hex and integer), and their RAM size
               in bytes.

*/

#include <stdio.h>                      // Include the standard IO library, needed for printf() method
#include <stdint.h>                     // Include the standard integer library
#include <float.h>                      // Include the floating point library
#include "regs.h"                       // Include the registry header file for Aquarius+ (Is this needed for this simple program?)

int a = 5;                              // Create a simple integer variable
unsigned long b = 30000;                // Create an unsigned long integer variable
float var = 12.34784;                   // Create a float variable
char greeting[] = "Hello World!";       // Create a character array (string)

int main(void) {

    // Add a couple of newlines before starting
    printf("\n\n");
    // Show our value for integer a, it's address in RAM, and it's size
    printf("                  a = %u\n", a);
    printf("      (addr of)  &a = 0x%04x | %u\n", &a, &a);
    printf("          sizeof(a) = %u (bytes)\n", sizeof(a));
    printf("\n");
    // Show our value for unsigned long integer b, it's address in RAM, and it's size
    printf("                  b = %u\n", b);
    printf("      (addr of)  &b = 0x%04x | %u\n", &b, &b);
    printf("          sizeof(b) = %u (bytes)\n", sizeof(b));
    printf("\n");
    // Show our value for integer var, it's address in RAM, and it's size
    printf("                var = %.5f\n", var);
    printf("    (addr of)  &var = 0x%04x | %u\n", &var, &var);
    printf("        sizeof(var) = %u (bytes)\n", sizeof(var));
    printf("\n");
    // Show our value for char array greeting, it's address in RAM, and it's size
    printf("           greeting = %s\n", greeting);
    printf("(addr of) &greeting = 0x%04x | %u\n", &greeting, &greeting);
    printf("   sizeof(greeting) = %u (bytes)\n", sizeof(greeting));
    printf("\n");

    // Returning a value to exit the main loop
    return 0;
}
