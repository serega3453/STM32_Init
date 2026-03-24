#include <regutils.h>
#include <regaddr.h>
#include <i2cutils.h>

/**
 * Write a single byte to a register on a 7-bit addressed I2C device.
 * Uses a write transaction: START, send device addr (write), send register addr, send value, STOP.
 * dev7 - 7-bit device address (will be shifted left by 1 internally)
 * reg  - register address to write
 * val  - byte value to write into the register
 */
uint8_t I2C1_WriteByte(uint8_t dev7, uint8_t reg, uint8_t val)
{
    while (read_bits(&I2C1_ISR, (1U << 15)));                         // BUSY

    write_bits(&I2C1_ICR, (1U << 4), (1U << 4));                     // clear NACKF
    write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                     // clear STOPF

    write_bits(&I2C1_CR2, (1U << 13), 0);                            // START=0
    write_bits(&I2C1_CR2, (1U << 14), 0);                            // STOP=0
    write_bits(&I2C1_CR2, (0x3FFU << 0), ((uint32_t)dev7 << 1));     // SADD
    write_bits(&I2C1_CR2, (1U << 10), 0);                            // RD_WRN=0
    write_bits(&I2C1_CR2, (0xFFU << 16), (2U << 16));                // NBYTES=2
    write_bits(&I2C1_CR2, (1U << 25), (1U << 25));                   // AUTOEND=1
    write_bits(&I2C1_CR2, (1U << 13), (1U << 13));                   // START=1

    while (!(read_bits(&I2C1_ISR, (1U << 1))) &&                     // TXIS
           !(read_bits(&I2C1_ISR, (1U << 4))));                      // NACKF
    if (read_bits(&I2C1_ISR, (1U << 4)))                             // NACKF
    {
        write_bits(&I2C1_ICR, (1U << 4), (1U << 4));
        while (!(read_bits(&I2C1_ISR, (1U << 5))));                  // STOPF
        write_bits(&I2C1_ICR, (1U << 5), (1U << 5));
        return 1;
    }

    write_reg(&I2C1_TXDR, reg);

    while (!(read_bits(&I2C1_ISR, (1U << 1))) &&                     // TXIS
           !(read_bits(&I2C1_ISR, (1U << 4))));                      // NACKF
    if (read_bits(&I2C1_ISR, (1U << 4)))                             // NACKF
    {
        write_bits(&I2C1_ICR, (1U << 4), (1U << 4));
        while (!(read_bits(&I2C1_ISR, (1U << 5))));                  // STOPF
        write_bits(&I2C1_ICR, (1U << 5), (1U << 5));
        return 2;
    }

    write_reg(&I2C1_TXDR, val);

    while (!(read_bits(&I2C1_ISR, (1U << 5))) &&                     // STOPF
           !(read_bits(&I2C1_ISR, (1U << 4))));                      // NACKF

    if (read_bits(&I2C1_ISR, (1U << 4)))                             // NACKF
    {
        write_bits(&I2C1_ICR, (1U << 4), (1U << 4));
        while (!(read_bits(&I2C1_ISR, (1U << 5))));                  // STOPF
        write_bits(&I2C1_ICR, (1U << 5), (1U << 5));
        return 3;
    }

    write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                     // clear STOPF
    while (read_bits(&I2C1_ISR, (1U << 15)));                        // BUSY

    return 0;
}

/**
 * Read n bytes from a register on a 7-bit addressed I2C device.
 * Uses repeated-start (ReStart) sequence: write register address, then read n bytes.
 * Transaction flow: START, send dev addr (write), send register addr, RESTART, 
 *                    send dev addr (read), read n bytes into buf, STOP.
 * dev7 - 7-bit device address
 * reg  - register address to read from
 * buf  - destination buffer (must hold at least n bytes)
 * n    - number of bytes to read
 */
uint8_t I2C1_ReadN(uint8_t dev7, uint8_t reg, uint8_t* buf, uint8_t n)
{
    uint8_t i;

    /* Ждём освобождения шины */
    while (read_bits(&I2C1_ISR, (1U << 15)));                         // BUSY

    /* Сбрасываем старые флаги */
    write_bits(&I2C1_ICR, (1U << 4), (1U << 4));                     // NACKCF
    write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                     // STOPCF

    /* Фаза записи: передаём адрес регистра */
    write_bits(&I2C1_CR2, (1U << 13), 0);                            // START=0
    write_bits(&I2C1_CR2, (1U << 14), 0);                            // STOP=0
    write_bits(&I2C1_CR2, (0x3FFU << 0), ((uint32_t)dev7 << 1));     // SADD
    write_bits(&I2C1_CR2, (1U << 10), 0);                            // RD_WRN=0
    write_bits(&I2C1_CR2, (0xFFU << 16), (1U << 16));                // NBYTES=1
    write_bits(&I2C1_CR2, (1U << 25), 0);                            // AUTOEND=0
    write_bits(&I2C1_CR2, (1U << 13), (1U << 13));                   // START=1

    while (!(read_bits(&I2C1_ISR, (1U << 1))) &&                     // TXIS
           !(read_bits(&I2C1_ISR, (1U << 4))));                      // NACKF

    if (read_bits(&I2C1_ISR, (1U << 4)))                             // NACKF
    {
        write_bits(&I2C1_ICR, (1U << 4), (1U << 4));                 // clear NACKF
        write_bits(&I2C1_CR2, (1U << 14), (1U << 14));               // STOP=1
        while (!(read_bits(&I2C1_ISR, (1U << 5))));                  // STOPF
        write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                 // clear STOPF
        return 1;
    }

    write_reg(&I2C1_TXDR, reg);

    while (!(read_bits(&I2C1_ISR, (1U << 6))) &&                     // TC
           !(read_bits(&I2C1_ISR, (1U << 4))));                      // NACKF

    if (read_bits(&I2C1_ISR, (1U << 4)))                             // NACKF
    {
        write_bits(&I2C1_ICR, (1U << 4), (1U << 4));                 // clear NACKF
        write_bits(&I2C1_CR2, (1U << 14), (1U << 14));               // STOP=1
        while (!(read_bits(&I2C1_ISR, (1U << 5))));                  // STOPF
        write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                 // clear STOPF
        return 2;
    }

    /* Фаза чтения */
    write_bits(&I2C1_CR2, (1U << 13), 0);                            // START=0
    write_bits(&I2C1_CR2, (1U << 14), 0);                            // STOP=0
    write_bits(&I2C1_CR2, (0x3FFU << 0), ((uint32_t)dev7 << 1));     // SADD
    write_bits(&I2C1_CR2, (1U << 10), (1U << 10));                   // RD_WRN=1
    write_bits(&I2C1_CR2, (0xFFU << 16), ((uint32_t)n << 16));       // NBYTES=n
    write_bits(&I2C1_CR2, (1U << 25), (1U << 25));                   // AUTOEND=1
    write_bits(&I2C1_CR2, (1U << 13), (1U << 13));                   // START=1

    for (i = 0; i < n; i++)
    {
        while (!(read_bits(&I2C1_ISR, (1U << 2))) &&                 // RXNE
               !(read_bits(&I2C1_ISR, (1U << 4))));                  // NACKF

        if (read_bits(&I2C1_ISR, (1U << 4)))                         // NACKF
        {
            write_bits(&I2C1_ICR, (1U << 4), (1U << 4));             // clear NACKF
            while (!(read_bits(&I2C1_ISR, (1U << 5))));              // STOPF
            write_bits(&I2C1_ICR, (1U << 5), (1U << 5));             // clear STOPF
            return 3;
        }

        buf[i] = (uint8_t)read_bits(&I2C1_RXDR, 0xFFU);
    }

    while (!(read_bits(&I2C1_ISR, (1U << 5))));                      // STOPF
    write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                     // clear STOPF

    /* Вот это важно: ждём, пока BUSY реально уйдёт */
    while (read_bits(&I2C1_ISR, (1U << 15)));                        // BUSY

    return 0;
}
