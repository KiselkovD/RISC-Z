#include "cpu.h"	 // для определения rz_cpu_p и rz_register_t
#include "misc.h"	 // если rz_register_t определён здесь (если не в cpu.h)
#include <stdio.h> // для printf, fprintf
#include "ecall.h" // для объявления rz_ecall_handle

bool rz_ecall_handle(rz_cpu_p pcpu)
{
	rz_register_t syscall_num = pcpu->r_x[17]; // a7 — номер системного вызова

	switch (syscall_num)
	{
	case 0: // Ввод целого числа с stdin в a0
	{
		int value;
		printf("Input integer: ");
		int ret = scanf("%d", &value);
		if (ret != 1)
		{
			fprintf(stderr, "Failed to read integer from input\n");
			return false; // Ошибка — остановить симулятор
		}
		pcpu->r_x[10] = (rz_register_t)value; // Записываем в a0
		break;
	}
	case 1: // Вывод целого числа из a0 на stdout
	{
		int value = (int)pcpu->r_x[10]; // Значение из a0
		printf("%d\n", value);
		break;
	}
	default:
		fprintf(stderr, "Unknown syscall number: %d\n", (int)syscall_num);
		return false; // Неизвестный системный вызов — остановка
	}

	return true; // Продолжаем выполнение
}
