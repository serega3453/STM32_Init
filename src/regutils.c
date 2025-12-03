#include "regutils.h"
#include <regaddr.h>

/**
 * Write bits to a register using read-modify-write (RMW) pattern.
 * Clears bits in the mask, then sets new bits (value & mask).
 * reg    - pointer to volatile register
 * mask   - which bits to modify
 * value  - new bit values (will be masked)
 */
void write_bits(volatile uint32_t* reg, uint32_t mask, uint32_t value)
{
    uint32_t tmp = *reg;
    tmp &= ~mask;
    tmp |= (value & mask);
    *reg = tmp;
}

/**
 * Direct write to a register (no masking).
 * reg    - pointer to volatile register
 * value  - full 32-bit value to write
 */
void write_reg(volatile uint32_t* reg, uint32_t value)
{
    *reg = value;
}

/**
 * Read masked bits from a register.
 * Returns register value ANDed with mask (zeroing all other bits).
 * reg    - pointer to volatile register
 * mask   - which bits to read
 * return - (register_value & mask)
 */
uint32_t read_bits(volatile uint32_t* reg, uint32_t mask)
{
    return *reg & mask;
}

/**
 * Simple busy-wait delay loop using NOP instructions.
 * Useful for short delays; actual time depends on core frequency.
 * d      - number of iterations (each iteration ~1 cycle on ARM Cortex-M0)
 */
void raw_delay(volatile uint32_t d)
{
    while(d--) __asm__("nop");
}

/**
 * Transmit a single character over USART1 (blocking).
 * Waits for transmit buffer empty (TXE) before sending.
 * c      - character to send
 */
void usart1_putc(char c) {
    while(!(read_bits(&USART1_ISR, (1U<<7))));           // TXE=1
    write_reg(&USART1_TDR, (uint32_t)(uint8_t)c);
}

/**
 * Transmit a NUL-terminated string over USART1 (blocking).
 * Calls usart1_putc() for each character until NUL.
 * s      - pointer to string (must be NUL-terminated)
 */
void usart1_puts(const char* s) {
    while(*s) usart1_putc(*s++);
}

/**
 * Transmit a signed 16-bit integer as decimal over USART1 (blocking).
 * Uses simple digit-by-digit conversion (no padding, max 6 digits).
 * Prints '-' for negative numbers.
 * v      - signed 16-bit value to print
 */
void usart1_put_i16(int16_t v) {
    char buf[8]; int i=0; if (v<0){ usart1_putc('-'); v=-v; }
    if (v==0){ usart1_putc('0'); return; }
    while(v && i<6){ buf[i++] = '0' + (v%10); v/=10; }
    while(i--) usart1_putc(buf[i]);
}