#ifndef __ECALL_H__
#define __ECALL_H__

#include "cpu.h"

// Обработка инструкции ECALL и EBREAK
// Возвращает true, если обработка успешна и симулятор должен продолжить работу,
// false — если нужно остановить симулятор (например, при EBREAK)
bool rz_ecall_handle(rz_cpu_p pcpu);

#endif // ECALL_H__
