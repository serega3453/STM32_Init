#ifndef REG_UTILS_H
#define REG_UTILS_H

#include <stdint.h>

/**
 * Register manipulation and USART utility functions.
 * Provides low-level read/write register access and serial I/O helpers.
 */

/**
 * Write bits to a register using read-modify-write (RMW) pattern.
 * reg   - pointer to volatile register
 * mask  - which bits to modify (1 = will be changed, 0 = untouched)
 * value - new bit values (masked and OR'd into register)
 */
void write_bits(volatile uint32_t* reg, uint32_t mask, uint32_t value);

/**
 * Direct write to a register (no masking).
 * reg   - pointer to volatile register
 * value - full 32-bit value to write
 */
void write_reg(volatile uint32_t* reg, uint32_t value);

/**
 * Read masked bits from a register.
 * reg   - pointer to volatile register
 * mask  - which bits to read (1 = include, 0 = zero out)
 * return - (register_value & mask)
 */
uint32_t read_bits(volatile uint32_t* reg, uint32_t mask);

/**
 * Simple busy-wait delay using NOP instructions.
 * d - number of NOP iterations (time = d / core_frequency_Hz)
 */
void raw_delay(volatile uint32_t d);

/**
 * Transmit a single character over USART1 (blocking).
 * c - character to send (0x00..0xFF)
 */
void usart1_putc(char c);

/**
 * Transmit a NUL-terminated string over USART1 (blocking).
 * s - pointer to string (must be NUL-terminated)
 */
void usart1_puts(const char* s);

/**
 * Transmit a signed 16-bit integer as decimal over USART1 (blocking).
 * Prints '-' for negative values; no padding, max 6 digits.
 * v - signed 16-bit value to print
 */
void usart1_put_i16(int16_t v);

#endif // REG_UTILS_H
