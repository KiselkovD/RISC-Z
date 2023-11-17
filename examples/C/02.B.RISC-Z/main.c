#include <stdio.h>
#include "risc-z.h"

int main()
{
    puts("Hello, World!");
    printf("CPU Info: %s\n", cpuinfo());
    return 0;
}
