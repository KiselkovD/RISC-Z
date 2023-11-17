#include <stdio.h>
#include "risc-z.h"

int main()
{
    #ifdef DEBUG
    puts("DEBUG");
    #endif

    #ifdef NDEBUG
    puts("RELEASE");
    #endif

    puts("Hello, World!");
    printf("CPU Info: %s\n", cpuinfo());
    return 0;
}
