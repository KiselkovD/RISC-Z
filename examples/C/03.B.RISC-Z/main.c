#include <stdio.h>
#include "cpu.h"
#include "memory.h"

int main(int argc, const char *argv[])
{
    #ifdef DEBUG
    puts("DEBUG");
    #endif

    #ifdef NDEBUG
    puts("RELEASE");
    #endif

    puts("Hello, World!");
    printf("CPU Info: %s\n", cpuinfo());

    // load dumps
    FILE *code_file = fopen(argv[1], "rb");
    fread(mem_access(TEXT_OFFSET), 1, TEXT_SIZE, code_file);
    fclose(code_file);

    while(rz_cycle());

    return 0;
}
