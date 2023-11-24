#include <stdint.h>
#include <stdio.h>

#include "misc.h"
#include "cpu.h"
#include "memory.h"

rz_register_t r_pc, r_x[32];

const char *cpuinfo(void)
{
    return "RISC-Z.32.2023";
}

void cycle()
{
    rz_instruction_t current = *((rz_instruction_t *)mem_access(r_pc));
    rz_instruction_t opcode = current & 0b1111111;
    switch(opcode)
    {
        case OP_ADD:
             puts("Add!");
        break;
        default:
            fprintf(stderr, "Invalid instruction %08X with opcode %08X\n", current, opcode);
            ;
            //
    }
    r_pc += sizeof(rz_instruction_t);
}
