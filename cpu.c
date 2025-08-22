#include <stdint.h> // Стандартные целочисленные типы с фиксированным размером
#include <stdio.h>  // Стандартный ввод-вывод (printf, fprintf)
#include <string.h> // Функции для работы с памятью (memset и др.)
#include <stdlib.h> // Стандартные функции (malloc, free и др.)
#include <assert.h> // Макрос assert для отладки

#include "misc.h"   // Пользовательские типы (rz_register_t и др.)
#include "cpu.h"    // Интерфейс CPU
#include "memory.h" // Интерфейс памяти
#include "ecall.h"

#define FUNC3_OFFS 12                         // Смещение поля func3 в инструкции (12 бит)
#define FUNC7_OFFS 24                         // Смещение поля func7 в инструкции (24 бита)
#define OPCODE_MASK 0b1111111u                // Маска для выделения 7-битного кода операции (opcode)
#define FUNC3_MASK (0b111u << FUNC3_OFFS)     // Маска для выделения поля func3
#define FUNC7_MASK (0b1111111u << FUNC7_OFFS) // Маска для выделения поля func7

const char *rz_cpu_info(const rz_cpu_p pcpu)
{
    return pcpu->info;
}

// Определения форматов инструкций
enum rz_formats : unsigned
{
    LUI_FORMAT = 0b0110111u,
    AUIPC_FORMAT = 0b0010111u,
    J_FORMAT = 0b1101111u,
    JALR_FORMAT = 0b1100111u,
    R_FORMAT = 0b0110011u,
    S_FORMAT = 0b0100011u,
    L_FORMAT = 0b0000011u,
    I_FORMAT = 0b0010011u,
    MEM_FORMAT = 0b0001111u,
    SYS_FORMAT = 0b1110011u,
    B_FORMAT = 0b1100011u,
};

// Коды инструкций R-формата с соответствующими func3 и func7
enum rz_r_codes : unsigned
{
    ADD_CODE = R_FORMAT | (0b000u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SUB_CODE = R_FORMAT | (0b000u << FUNC3_OFFS) | (0b0100000u << FUNC7_OFFS),
    XOR_CODE = R_FORMAT | (0b100u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SLL_CODE = R_FORMAT | (0b001u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SLT_CODE = R_FORMAT | (0b010u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SLTU_CODE = R_FORMAT | (0b011u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SRL_CODE = R_FORMAT | (0b101u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SRA_CODE = R_FORMAT | (0b101u << FUNC3_OFFS) | (0b0100000u << FUNC7_OFFS),
    OR_CODE = R_FORMAT | (0b110u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    AND_CODE = R_FORMAT | (0b111u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
};

// Коды инструкций I-формата с func3 и func7 (для сдвигов)
enum rz_i_codes : unsigned
{
    ADDI_CODE = I_FORMAT | (0b000u << FUNC3_OFFS),
    SLLI_CODE = I_FORMAT | (0b001u << FUNC3_OFFS),
    SLTI_CODE = I_FORMAT | (0b010u << FUNC3_OFFS),
    SLTIU_CODE = I_FORMAT | (0b011u << FUNC3_OFFS),
    XORI_CODE = I_FORMAT | (0b100u << FUNC3_OFFS),
    ORI_CODE = I_FORMAT | (0b110u << FUNC3_OFFS),
    ANDI_CODE = I_FORMAT | (0b111u << FUNC3_OFFS),
    SRLI_CODE = I_FORMAT | (0b101u << FUNC3_OFFS) | (0b0000000u << FUNC7_OFFS),
    SRAI_CODE = I_FORMAT | (0b101u << FUNC3_OFFS) | (0b0100000u << FUNC7_OFFS),
    JALR_CODE = JALR_FORMAT | (0b000u << FUNC3_OFFS),
};

/**
 * @brief U-format code
 */
enum rz_u_codes : unsigned
{
    LUI_CODE = LUI_FORMAT,
    AUIPC_CODE = AUIPC_FORMAT,
};

/**
 * @brief J-format code
 */
enum rz_j_codes : unsigned
{
    JAL_CODE = J_FORMAT,
};

/**
 * @brief B-format code
 */
enum rz_b_codes : unsigned
{
    BEQ_CODE = B_FORMAT | (0b000u << FUNC3_OFFS),
    BNE_CODE = B_FORMAT | (0b001u << FUNC3_OFFS),
    BLT_CODE = B_FORMAT | (0b100u << FUNC3_OFFS),
    BGE_CODE = B_FORMAT | (0b101u << FUNC3_OFFS),
    BLTU_CODE = B_FORMAT | (0b110u << FUNC3_OFFS),
    BGEU_CODE = B_FORMAT | (0b111u << FUNC3_OFFS),
};

/**
 * @brief L-format code (load instructions)
 */
enum rz_l_codes : unsigned
{
    LB_CODE = L_FORMAT | (0b000u << FUNC3_OFFS),
    LH_CODE = L_FORMAT | (0b001u << FUNC3_OFFS),
    LW_CODE = L_FORMAT | (0b010u << FUNC3_OFFS),
    LBU_CODE = L_FORMAT | (0b100u << FUNC3_OFFS),
    LHU_CODE = L_FORMAT | (0b101u << FUNC3_OFFS),
};

/**
 * @brief S-format code (store instructions)
 */
enum rz_s_codes : unsigned
{
    SB_CODE = S_FORMAT | (0b000u << FUNC3_OFFS),
    SH_CODE = S_FORMAT | (0b001u << FUNC3_OFFS),
    SW_CODE = S_FORMAT | (0b010u << FUNC3_OFFS),
};

// Коды для MEM-формата (FENCE и FENCE.I)
enum rz_mem_codes : unsigned
{
    FENCE_CODE = MEM_FORMAT | (0b000u << FUNC3_OFFS),
    FENCE_I_CODE = MEM_FORMAT | (0b001u << FUNC3_OFFS),
};

// Коды для sys-формата
enum rz_sys_codes : unsigned
{
    ECALL_CODE = SYS_FORMAT,
    EBREAK_CODE = SYS_FORMAT | (1u << 20),
};

// Объединение для декодирования инструкций
typedef union
{
    rz_register_t whole;
    struct
    {
        unsigned op : 7;
        unsigned rd : 5;
        unsigned f3 : 3;
        unsigned rs1 : 5;
        unsigned rs2 : 5;
        unsigned f7 : 7;
    } r;
    struct
    {
        unsigned op : 7;
        unsigned rd : 5;
        unsigned f3 : 3;
        unsigned rs1 : 5;
        unsigned imm0_11 : 12;
    } i;
    struct
    {
        unsigned op : 7;
        unsigned imm0_4 : 5;
        unsigned f3 : 3;
        unsigned rs1 : 5;
        unsigned rs2 : 5;
        unsigned imm5_11 : 7;
    } s;
    struct
    {
        unsigned op : 7;
        unsigned imm11 : 1;
        unsigned imm1_4 : 4;
        unsigned f3 : 3;
        unsigned rs1 : 5;
        unsigned rs2 : 5;
        unsigned imm5_10 : 6;
        unsigned imm12 : 1;
    } b;
    struct
    {
        unsigned op : 7;
        unsigned rd : 5;
        unsigned imm12_31 : 20;
    } u;
    struct
    {
        unsigned op : 7;
        unsigned rd : 5;
        unsigned imm12_19 : 8;
        unsigned imm11 : 1;
        unsigned imm1_10 : 10;
        unsigned imm20 : 1;
    } j;
} rz_instruction_t;

// Структура CPU
// struct rz_cpu_s
//{
//    const char *info;
//    rz_register_t r_pc, r_x[32];
//};

// Функция создания CPU
rz_cpu_p rz_create_cpu(void)
{
    rz_cpu_p pcpu = malloc(sizeof(rz_cpu_t));
    pcpu->info = "RISC-Z.32.2023";

    pcpu->r_pc = TEXT_OFFSET;

    memset(pcpu->r_x, 0, sizeof(pcpu->r_x));
    pcpu->r_x[2] = STACK_OFFSET + STACK_SIZE - (unsigned)sizeof(rz_register_t);
    pcpu->r_x[3] = DATA_OFFSET;

    return pcpu;
}

// Функция освобождения CPU
void rz_free_cpu(rz_cpu_p pcpu)
{
    free(pcpu);
}

// Функция знакового расширения
static inline rz_register_t sign_extend(unsigned some_bits, int how_many_bits)
{
    rz_register_t result = some_bits;
    if (1 << (how_many_bits - 1) & result)
        result |= -1 << how_many_bits;
    return result;
}

// Обработка R-формата (арифметика регистр-регистр)
bool rz_r_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_r_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    unsigned opcode_f3_f7 = instr.whole & (OPCODE_MASK | FUNC3_MASK | FUNC7_MASK);
    switch (opcode_f3_f7)
    {
    case ADD_CODE:
        printf("  ADD x%d = x%d + x%d\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] + pcpu->r_x[instr.r.rs2];
        break;
    case SUB_CODE:
        printf("  SUB x%d = x%d - x%d\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] - pcpu->r_x[instr.r.rs2];
        break;
    case XOR_CODE:
        printf("  XOR x%d = x%d ^ x%d\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] ^ pcpu->r_x[instr.r.rs2];
        break;
    case SLL_CODE:
    {
        unsigned shamt = pcpu->r_x[instr.r.rs2] & 0x1F;
        printf("  SLL x%d = x%d << %u\n", instr.r.rd, instr.r.rs1, shamt);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] << shamt;
    }
    break;
    case SLT_CODE:
        printf("  SLT x%d = (x%d < x%d) ? 1 : 0\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = ((int32_t)pcpu->r_x[instr.r.rs1] < (int32_t)pcpu->r_x[instr.r.rs2]) ? 1 : 0;
        break;
    case SLTU_CODE:
        printf("  SLTU x%d = (x%d < x%d) ? 1 : 0\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = (pcpu->r_x[instr.r.rs1] < pcpu->r_x[instr.r.rs2]) ? 1 : 0;
        break;
    case SRL_CODE:
    {
        unsigned shamt = pcpu->r_x[instr.r.rs2] & 0x1F;
        printf("  SRL x%d = x%d >> %u (logical)\n", instr.r.rd, instr.r.rs1, shamt);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] >> shamt;
    }
    break;
    case SRA_CODE:
    {
        unsigned shamt = pcpu->r_x[instr.r.rs2] & 0x1F;
        printf("  SRA x%d = x%d >> %u (arithmetic)\n", instr.r.rd, instr.r.rs1, shamt);
        pcpu->r_x[instr.r.rd] = (rz_register_t)((int32_t)pcpu->r_x[instr.r.rs1] >> shamt);
    }
    break;
    case OR_CODE:
        printf("  OR x%d = x%d | x%d\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] | pcpu->r_x[instr.r.rs2];
        break;
    case AND_CODE:
        printf("  AND x%d = x%d & x%d\n", instr.r.rd, instr.r.rs1, instr.r.rs2);
        pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] & pcpu->r_x[instr.r.rs2];
        break;
    default:
        printf("  Unknown R-format instruction\n");
        return false;
    }
    return true;
}

// Обработка I-формата (арифметика с немедленным)
bool rz_i_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_i_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    unsigned func3 = (instr.whole & FUNC3_MASK) >> FUNC3_OFFS;
    unsigned func7 = (instr.whole & FUNC7_MASK) >> FUNC7_OFFS;

    switch (instr.whole & OPCODE_MASK)
    {
    case I_FORMAT:
        switch (func3)
        {
        case 0b000: // ADDI
            printf("  ADDI x%d = x%d + %d\n", instr.i.rd, instr.i.rs1, sign_extend(instr.i.imm0_11, 12));
            pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] + sign_extend(instr.i.imm0_11, 12);
            break;
        case 0b001: // SLLI
            if (func7 == 0b0000000)
            {
                unsigned shamt = instr.i.imm0_11 & 0x1F;
                printf("  SLLI x%d = x%d << %u\n", instr.i.rd, instr.i.rs1, shamt);
                pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] << shamt;
            }
            else
            {
                printf("  Unknown SLLI func7\n");
                return false;
            }
            break;
        case 0b010: // SLTI
            printf("  SLTI x%d = (x%d < %d) ? 1 : 0\n", instr.i.rd, instr.i.rs1, sign_extend(instr.i.imm0_11, 12));
            pcpu->r_x[instr.i.rd] = ((int32_t)pcpu->r_x[instr.i.rs1] < (int32_t)sign_extend(instr.i.imm0_11, 12)) ? 1 : 0;
            break;
        case 0b011: // SLTIU
            printf("  SLTIU x%d = (x%d < %u) ? 1 : 0\n", instr.i.rd, instr.i.rs1, (unsigned)(instr.i.imm0_11 & 0xFFF));
            pcpu->r_x[instr.i.rd] = (pcpu->r_x[instr.i.rs1] < (rz_register_t)(instr.i.imm0_11 & 0xFFF)) ? 1 : 0;
            break;
        case 0b100: // XORI
            printf("  XORI x%d = x%d ^ %d\n", instr.i.rd, instr.i.rs1, sign_extend(instr.i.imm0_11, 12));
            pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] ^ sign_extend(instr.i.imm0_11, 12);
            break;
        case 0b101: // SRLI or SRAI
            if (func7 == 0b0000000)
            {
                unsigned shamt = instr.i.imm0_11 & 0x1F;
                printf("  SRLI x%d = x%d >> %u (logical)\n", instr.i.rd, instr.i.rs1, shamt);
                pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] >> shamt;
            }
            else if (func7 == 0b0100000)
            {
                unsigned shamt = instr.i.imm0_11 & 0x1F;
                printf("  SRAI x%d = x%d >> %u (arithmetic)\n", instr.i.rd, instr.i.rs1, shamt);
                pcpu->r_x[instr.i.rd] = (rz_register_t)((int32_t)pcpu->r_x[instr.i.rs1] >> shamt);
            }
            else
            {
                printf("  Unknown SRLI/SRAI func7\n");
                return false;
            }
            break;
        case 0b110: // ORI
            printf("  ORI x%d = x%d | %d\n", instr.i.rd, instr.i.rs1, sign_extend(instr.i.imm0_11, 12));
            pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] | sign_extend(instr.i.imm0_11, 12);
            break;
        case 0b111: // ANDI
            printf("  ANDI x%d = x%d & %d\n", instr.i.rd, instr.i.rs1, sign_extend(instr.i.imm0_11, 12));
            pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] & sign_extend(instr.i.imm0_11, 12);
            break;
        default:
            printf("  Unknown I-format func3\n");
            return false;
        }
        break;
    default:
        printf("  Unknown I-format opcode\n");
        return false;
    }
    return true;
}

// Обработка JAL (Jump and Link)
bool rz_j_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_j_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    int32_t imm = 0;
    imm |= (instr.j.imm20 << 20);
    imm |= (instr.j.imm12_19 << 12);
    imm |= (instr.j.imm11 << 11);
    imm |= (instr.j.imm1_10 << 1);

    if (imm & (1 << 20))
    {
        imm |= ~((1 << 21) - 1);
    }

    printf("  JAL x%d, offset %d\n", instr.j.rd, imm);
    pcpu->r_x[instr.j.rd] = pcpu->r_pc + sizeof(rz_instruction_t);
    pcpu->r_pc = pcpu->r_pc + imm;

    return true;
}

// обработка JALR (Jump and Link Register)
bool rz_jalr_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_jalr_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    unsigned func3 = (instr.whole & FUNC3_MASK) >> FUNC3_OFFS;
    if (func3 != 0b000)
    {
        printf("  Unsupported JALR func3: %u\n", func3);
        return false;
    }

    int32_t imm = sign_extend(instr.i.imm0_11, 12);

    rz_register_t target = (pcpu->r_x[instr.i.rs1] + imm) & ~1u;
    printf("  JALR x%d, x%d + %d = 0x%08X\n", instr.i.rd, instr.i.rs1, imm, target);
    pcpu->r_x[instr.i.rd] = pcpu->r_pc + sizeof(rz_instruction_t);
    pcpu->r_pc = target;

    return true;
}

// Обработка B-формата (ветвления)
bool rz_b_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_b_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    int32_t imm = 0;
    imm |= (instr.b.imm12 << 12);
    imm |= (instr.b.imm11 << 11);
    imm |= (instr.b.imm1_4 << 1);
    imm |= (instr.b.imm5_10 << 5);

    if (imm & (1 << 12))
    {
        imm |= ~((1 << 13) - 1);
    }

    unsigned func3 = (instr.whole & FUNC3_MASK) >> FUNC3_OFFS;
    bool taken = false;

    switch (func3)
    {
    case 0b000: // BEQ
        taken = (pcpu->r_x[instr.b.rs1] == pcpu->r_x[instr.b.rs2]);
        printf("  BEQ x%d, x%d, offset %d : %s\n", instr.b.rs1, instr.b.rs2, imm, taken ? "taken" : "not taken");
        break;
    case 0b001: // BNE
        taken = (pcpu->r_x[instr.b.rs1] != pcpu->r_x[instr.b.rs2]);
        printf("  BNE x%d, x%d, offset %d : %s\n", instr.b.rs1, instr.b.rs2, imm, taken ? "taken" : "not taken");
        break;
    case 0b100: // BLT
        taken = ((int32_t)pcpu->r_x[instr.b.rs1] < (int32_t)pcpu->r_x[instr.b.rs2]);
        printf("  BLT x%d, x%d, offset %d : %s\n", instr.b.rs1, instr.b.rs2, imm, taken ? "taken" : "not taken");
        break;
    case 0b101: // BGE
        taken = ((int32_t)pcpu->r_x[instr.b.rs1] >= (int32_t)pcpu->r_x[instr.b.rs2]);
        printf("  BGE x%d, x%d, offset %d : %s\n", instr.b.rs1, instr.b.rs2, imm, taken ? "taken" : "not taken");
        break;
    case 0b110: // BLTU
        taken = (pcpu->r_x[instr.b.rs1] < pcpu->r_x[instr.b.rs2]);
        printf("  BLTU x%d, x%d, offset %d : %s\n", instr.b.rs1, instr.b.rs2, imm, taken ? "taken" : "not taken");
        break;
    case 0b111: // BGEU
        taken = (pcpu->r_x[instr.b.rs1] >= pcpu->r_x[instr.b.rs2]);
        printf("  BGEU x%d, x%d, offset %d : %s\n", instr.b.rs1, instr.b.rs2, imm, taken ? "taken" : "not taken");
        break;
    default:
        printf("  Unknown B-format func3: %u\n", func3);
        break;
    }

    if (taken)
        pcpu->r_pc = pcpu->r_pc + imm;
    else
        pcpu->r_pc += sizeof(rz_instruction_t);

    return true;
}

// Обработка L-формата (загрузки)
bool rz_l_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_l_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    int32_t imm = sign_extend(instr.i.imm0_11, 12);
    rz_address_t addr = pcpu->r_x[instr.i.rs1] + imm;

    switch ((instr.whole & FUNC3_MASK) >> FUNC3_OFFS)
    {
    case 0b000:
        printf("  LB x%d, %d(x%d)\n", instr.i.rd, imm, instr.i.rs1);
        {
            int8_t val = *(int8_t *)mem_access(addr);
            pcpu->r_x[instr.i.rd] = (rz_register_t)val;
        }
        break;
    case 0b001:
        printf("  LH x%d, %d(x%d)\n", instr.i.rd, imm, instr.i.rs1);
        {
            int16_t val = *(int16_t *)mem_access(addr);
            pcpu->r_x[instr.i.rd] = (rz_register_t)val;
        }
        break;
    case 0b010:
        printf("  LW x%d, %d(x%d)\n", instr.i.rd, imm, instr.i.rs1);
        {
            int32_t val = *(int32_t *)mem_access(addr);
            pcpu->r_x[instr.i.rd] = (rz_register_t)val;
        }
        break;
    case 0b100:
        printf("  LBU x%d, %d(x%d)\n", instr.i.rd, imm, instr.i.rs1);
        {
            uint8_t val = *(uint8_t *)mem_access(addr);
            pcpu->r_x[instr.i.rd] = (rz_register_t)val;
        }
        break;
    case 0b101:
        printf("  LHU x%d, %d(x%d)\n", instr.i.rd, imm, instr.i.rs1);
        {
            uint16_t val = *(uint16_t *)mem_access(addr);
            pcpu->r_x[instr.i.rd] = (rz_register_t)val;
        }
        break;
    default:
        printf("  Unknown L-format func3\n");
        return false;
    }

    return true;
}

// Обработка S-формата (сохранения)
bool rz_s_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_s_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    int32_t imm = 0;
    imm |= instr.s.imm0_4;
    imm |= instr.s.imm5_11 << 5;

    if (imm & (1 << 11))
    {
        imm |= ~((1 << 12) - 1);
    }

    rz_address_t addr = pcpu->r_x[instr.s.rs1] + imm;

    switch ((instr.whole & FUNC3_MASK) >> FUNC3_OFFS)
    {
    case 0b000:
        printf("  SB x%d, %d(x%d)\n", instr.s.rs2, imm, instr.s.rs1);
        {
            uint8_t val = (uint8_t)(pcpu->r_x[instr.s.rs2] & 0xFF);
            *(uint8_t *)mem_access(addr) = val;
        }
        break;
    case 0b001:
        printf("  SH x%d, %d(x%d)\n", instr.s.rs2, imm, instr.s.rs1);
        {
            uint16_t val = (uint16_t)(pcpu->r_x[instr.s.rs2] & 0xFFFF);
            *(uint16_t *)mem_access(addr) = val;
        }
        break;
    case 0b010:
        printf("  SW x%d, %d(x%d)\n", instr.s.rs2, imm, instr.s.rs1);
        {
            uint32_t val = (uint32_t)(pcpu->r_x[instr.s.rs2]);
            *(uint32_t *)mem_access(addr) = val;
        }
        break;
    default:
        printf("  Unknown S-format func3\n");
        return false;
    }

    return true;
}

// Обработка MEM-формата (FENCE и FENCE.I)
bool rz_mem_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_mem_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    unsigned func3 = (instr.whole & FUNC3_MASK) >> FUNC3_OFFS;

    switch (func3)
    {
    case 0b000:
        printf("  FENCE\n");
        return true;
    case 0b001:
        printf("  FENCE.I\n");
        return true;
    default:
        printf("  Unknown MEM-format func3\n");
        return false;
    }
}

// Обработка SYS-формата (ECALL, EBREAK)
bool rz_sys_cycle(rz_cpu_p pcpu, rz_instruction_t instr)
{
    printf("[rz_sys_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);
    if ((instr.whole & (1u << 20)) == 0)
    {
        printf("  ECALL\n");
        return rz_ecall_handle(pcpu);
    }
    else
    {
        fprintf(stderr, "  EBREAK encountered at PC=0x%08X: stopping simulation.\n", pcpu->r_pc);
        return false;
    }
}

// Основной цикл обработки инструкции
bool rz_cycle(rz_cpu_p pcpu)
{
    pcpu->r_x[0] = 0u; // Регистры x0 всегда 0

    rz_instruction_t instr = *((rz_instruction_t *)mem_access(pcpu->r_pc));
    printf("[rz_cycle] PC=0x%08X instr=0x%08X\n", pcpu->r_pc, instr.whole);

    bool goon = true;

    switch (instr.whole & OPCODE_MASK)
    {
    case R_FORMAT:
        goon = rz_r_cycle(pcpu, instr);
        break;
    case I_FORMAT:
        goon = rz_i_cycle(pcpu, instr);
        break;
    case L_FORMAT:
        goon = rz_l_cycle(pcpu, instr);
        break;
    case S_FORMAT:
        goon = rz_s_cycle(pcpu, instr);
        break;
    case LUI_FORMAT:
        printf("[rz_cycle] LUI x%d = 0x%X\n", instr.u.rd, instr.u.imm12_31 << 12);
        pcpu->r_x[instr.u.rd] = instr.u.imm12_31 << 12;
        break;
    case AUIPC_FORMAT:
        printf("[rz_cycle] AUIPC x%d = PC + 0x%X\n", instr.u.rd, instr.u.imm12_31 << 12);
        pcpu->r_x[instr.u.rd] = (instr.u.imm12_31 << 12) + pcpu->r_pc;
        break;
    case J_FORMAT:
        goon = rz_j_cycle(pcpu, instr);
        break;
    case JALR_FORMAT:
        goon = rz_jalr_cycle(pcpu, instr);
        break;
    case MEM_FORMAT:
        goon = rz_mem_cycle(pcpu, instr);
        break;
    case SYS_FORMAT:
        goon = rz_sys_cycle(pcpu, instr);
        break;
    case B_FORMAT:
        goon = rz_b_cycle(pcpu, instr);
        break;
    default:
        fprintf(stderr, "Invalid instruction %08X format, opcode %02X\n", instr.whole, instr.whole & OPCODE_MASK);
        goon = false;
    }

    if ((instr.whole & OPCODE_MASK) != B_FORMAT &&
        (instr.whole & OPCODE_MASK) != J_FORMAT &&
        (instr.whole & OPCODE_MASK) != JALR_FORMAT)
    {
        pcpu->r_pc += sizeof(rz_instruction_t);
    }

    return goon;
}
