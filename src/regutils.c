#include "regutils.h"

/* запись битов по маске (RMW) */
void write_bits(volatile uint32_t* reg, uint32_t mask, uint32_t value)
{
    uint32_t tmp = *reg;
    tmp &= ~mask;
    tmp |= (value & mask);
    *reg = tmp;
}

/* прямое присвоение регистра */
void write_reg(volatile uint32_t* reg, uint32_t value)
{
    *reg = value;
}

/* чтение только нужных битов */
uint32_t read_bits(volatile uint32_t* reg, uint32_t mask)
{
    return *reg & mask;
}

void raw_delay(volatile uint32_t d)
{
    while(d--) __asm__("nop");
}