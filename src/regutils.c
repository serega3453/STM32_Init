#include "regutils.h"
#include <regaddr.h>

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

void usart1_putc(char c) {
    while(!(read_bits(&USART1_ISR, (1U<<7))));           // TXE=1
    write_reg(&USART1_TDR, (uint32_t)(uint8_t)c);
}

void usart1_puts(const char* s) {
    while(*s) usart1_putc(*s++);
}

void usart1_put_i16(int16_t v) {                  // примитивный десятичный вывод
    char buf[8]; int i=0; if (v<0){ usart1_putc('-'); v=-v; }
    if (v==0){ usart1_putc('0'); return; }
    while(v && i<6){ buf[i++] = '0' + (v%10); v/=10; }
    while(i--) usart1_putc(buf[i]);
}