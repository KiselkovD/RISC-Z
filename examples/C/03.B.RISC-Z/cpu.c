#include <stdint.h>
#include <stdio.h>

#include "misc.h"
#include "cpu.h"
#include "memory.h"

rz_register_t r_pc, r_x[32];

#define FUNC3_OFFS 12
#define FUNC7_OFFS 24
#define OPCODE_MASK 0b1111111u
#define FUNC3_MASK  (0b111u << FUNC3_OFFS)
#define FUNC7_MASK  (0b1111111u << FUNC7_OFFS)

/**
 * @brief Opcodes to determine instruction format
 * 
 */
enum rz_formats : unsigned { // C23
    LUI_FORMAT    = 0b0110111u,
    AUIPC_FORMAT  = 0b0010111u,
    J_FORMAT  = 0b1101111u,
    JALR_FORMAT = 0b1100111u, // JALR
    R_FORMAT  = 0b0110011u,
    S_FORMAT  = 0b0100011u,
    L_FORMAT =  0b0000011u, // Load, I-format
    I_FORMAT = 0b0010011u, // ADDI..ANDI, SLLI, SRLI, SRAI
    MEM_FORMAT = 0b0001111u, // e.g. FENCE
    SYS_FORMAT = 0b1110011u, // e.g. ECALL, EBREAK
};

/**
 * @brief R-format CODE | F3 | F7
 * 
 */
 enum rz_r_codes : unsigned {
    ADD_CODE = R_FORMAT | ( 0b000u << FUNC3_OFFS ) | ( 0b0000000u << FUNC7_OFFS ),
    SUB_CODE = R_FORMAT | ( 0b000u << FUNC3_OFFS ) | ( 0b0100000u << FUNC7_OFFS ),
    XOR_CODE = R_FORMAT | ( 0b100u << FUNC3_OFFS ) | ( 0b0000000u << FUNC7_OFFS ),
 };

 /**
  * @brief I-format code | F3 [ | F7 ]
  * 
  */
enum rz_i_codes : unsigned {
    ADDI_CODE = I_FORMAT | ( 0b000u << FUNC3_OFFS ),
    SLLI_CODE = I_FORMAT | ( 0b001u << FUNC3_OFFS ),
};

/**
 * @brief U-format code
 * 
 */
enum rz_u_codes : unsigned {
    LUI_CODE = LUI_FORMAT,
    AUIPC_CODE = AUIPC_FORMAT,
};

enum rz_sys_codes : unsigned {
    ECALL_CODE = SYS_FORMAT,
    EBREAK_CODE = SYS_FORMAT | ( 1u << 20 ),
};

/**
 * @brief Represent RISC-V machine code for LE host-machines
 * 
 */
typedef union {
    rz_register_t whole;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned rs2: 5;
        unsigned f7: 7;
    } r;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned imm0_11: 12;
    } i;
    struct {
        unsigned op: 7;
        unsigned imm0_4: 5;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned rs2: 5;
        unsigned imm5_11: 7;
    } s;
    struct {
        unsigned op: 7;
        unsigned imm11: 1;
        unsigned imm1_4: 4;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned rs2: 5;
        unsigned imm5_10: 6;
        unsigned imm12: 1;
    } b;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned imm12_31: 20;
    } u;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned imm12_19: 8;
        unsigned imm11: 1;
        unsigned imm1_10: 10;
        unsigned imm20: 1;
    } j;
} rz_instruction_t;

const char *cpuinfo(void)
{
    return "RISC-Z.32.2023";
}

bool rz_r_cycle(rz_instruction_t instr) {
    switch(instr.whole & (OPCODE_MASK | FUNC3_MASK | FUNC7_MASK)) {
        case ADD_CODE:
            r_x[instr.r.rd] = r_x[instr.r.rs1] + r_x[instr.r.rs2];
        break;
        case SUB_CODE:
            r_x[instr.r.rd] = r_x[instr.r.rs1] - r_x[instr.r.rs2];
        break;
        case XOR_CODE:
            r_x[instr.r.rd] = r_x[instr.r.rs1] ^ r_x[instr.r.rs2];
        break;
        default:
            return false;
    }
    return true;
}

bool rz_i_cycle(rz_instruction_t instr) {
    switch(instr.whole & (OPCODE_MASK | FUNC3_MASK)) {
        case ADDI_CODE:
            r_x[instr.i.rd] = r_x[instr.i.rs1] + instr.i.imm0_11;
        break;
        case SLLI_CODE:
            r_x[instr.i.rd] = r_x[instr.i.rs1] << instr.r.rs2;
        break;
        break;
        default:
            return false;
    }
    return true;
}

bool rz_sys_cycle(rz_instruction_t instr) {
    switch(instr.whole & (1u << 20)){
        case 0:
            return true; // return rz_ecall();
        case 1:
        default:
            return false;
    }
}

bool rz_cycle() {
    rz_instruction_t instr = *((rz_instruction_t *)mem_access(r_pc));
    r_pc += sizeof(rz_instruction_t);

    switch(instr.whole & OPCODE_MASK) {
        case R_FORMAT:
            return rz_r_cycle(instr);
        break;
        case I_FORMAT:
            return rz_i_cycle(instr);
        break;
        case L_FORMAT:
        break;
        case S_FORMAT:
        break;
        case LUI_FORMAT:
            r_x[instr.u.rd] = (instr.u.imm12_31 << 12) | (r_x[instr.u.rd] & 0b111111111111);
            return true;
        break;
        case AUIPC_FORMAT:
            r_x[instr.u.rd] += (instr.u.imm12_31 << 12);
            return true;
        break;

        case J_FORMAT:
        break;
        case JALR_FORMAT:
        break;

        case MEM_FORMAT:
        break;
        case SYS_FORMAT:
            return rz_sys_cycle(instr);
        break;
        default:
            fprintf(stderr, "Invalid instruction %08X format, opcode %08X\n", instr.whole, instr.whole & OPCODE_MASK);
            return false;
    }
    return true;
}
