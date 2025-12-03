#ifndef I2CUTILS_H
#define I2CUTILS_H

#include <stdint.h>

/**
 * I2C helper functions for STM32F070 I2C1 peripheral.
 * Minimal, blocking I2C operations for single-master configurations.
 * Implements write and repeated-start read transactions.
 */

/**
 * Write a single byte to an I2C slave register (blocking).
 * Transaction: START, send slave addr (write), send register addr, send value, STOP.
 * dev7 - 7-bit slave address (will be shifted left internally)
 * reg  - register address
 * val  - byte value to write
 */
void I2C1_WriteByte(uint8_t dev7, uint8_t reg, uint8_t val);

/**
 * Read n bytes from an I2C slave register (blocking).
 * Uses repeated-start sequence: write register address, RESTART, read n bytes.
 * dev7 - 7-bit slave address (will be shifted left internally)
 * reg  - register address to read from
 * buf  - destination buffer (must hold at least n bytes)
 * n    - number of bytes to read
 */
void I2C1_ReadN(uint8_t dev7, uint8_t reg, uint8_t* buf, uint8_t n);

#endif