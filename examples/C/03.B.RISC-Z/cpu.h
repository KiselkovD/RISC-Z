#ifndef __CPU_H__
#define __CPU_H__

const char *cpuinfo(void);

void cycle();

// OPCODES
#define OP_ADD 0b0110011

#endif // CPU_H__
