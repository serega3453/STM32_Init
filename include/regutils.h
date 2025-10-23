#ifndef REG_UTILS_H
#define REG_UTILS_H

#include <stdint.h>

/* --- Функции работы с регистрами --- */
void write_bits(volatile uint32_t* reg, uint32_t mask, uint32_t value);
void write_reg(volatile uint32_t* reg, uint32_t value);
uint32_t read_bits(volatile uint32_t* reg, uint32_t mask);
void raw_delay(volatile uint32_t d);

#endif // REG_UTILS_H
