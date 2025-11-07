#ifndef REG_UTILS_H
#define REG_UTILS_H

#include <stdint.h>

/* --- Функции работы с регистрами --- */
void write_bits(volatile uint32_t* reg, uint32_t mask, uint32_t value);
void write_reg(volatile uint32_t* reg, uint32_t value);
uint32_t read_bits(volatile uint32_t* reg, uint32_t mask);
void raw_delay(volatile uint32_t d);
void usart1_putc(char c);
void usart1_puts(const char* s);
void usart1_put_i16(int16_t v);

#endif // REG_UTILS_H
