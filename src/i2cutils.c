#include <regutils.h>
#include <regaddr.h>
#include <i2cutils.h>

void I2C1_WriteByte(uint8_t dev7, uint8_t reg, uint8_t val)
{
    // WRITE, NBYTES=2, START
    write_bits(&I2C1_CR2, (1U <<1  ), 0);                          // START=0
    write_bits(&I2C1_CR2, (0x3FFU << 0), (uint32_t)(dev7 << 1));     // SADD = addr<<1
    write_bits(&I2C1_CR2, (1U << 10), 0);                          // RD_WRN=0 (write)
    write_bits(&I2C1_CR2, (0xFFU << 16), (2U << 16));                // NBYTES=2
    write_bits(&I2C1_CR2, (1U << 25), 0);                          // AUTOEND=0
    write_bits(&I2C1_CR2, (1U << 13), (1U << 13));                   // START=1

    while(!(read_bits(&I2C1_ISR, (1U << 1))));                     // TXIS
    write_reg(&I2C1_TXDR, reg);

    while(!(read_bits(&I2C1_ISR, (1U << 1))));                     // TXIS
    write_reg(&I2C1_TXDR, val);

    while(!(read_bits(&I2C1_ISR, (1U << 6))));                     // TC
    write_bits(&I2C1_CR2, (1U << 14), (1U << 14));                   // STOP=1
    while(!(read_bits(&I2C1_ISR, (1U << 5))));                     // STOPF
    write_bits(&I2C1_ICR, (1U << 5), (1U << 5));                     // clear STOPF
}

void I2C1_ReadN(uint8_t dev7, uint8_t reg, uint8_t* buf, uint8_t n)
{
    // write phase: отправить номер регистра
    write_bits(&I2C1_CR2, (1U << 13), 0);                          // START=0
    write_bits(&I2C1_CR2, (0x3FFU << 0), (uint32_t)(dev7 << 1));     // SADD
    write_bits(&I2C1_CR2, (1U << 10), 0);                          // RD_WRN=0
    write_bits(&I2C1_CR2, (0xFFU << 16), (1U << 16));                // NBYTES=1
    write_bits(&I2C1_CR2, (1U << 25), 0);                          // AUTOEND=0
    write_bits(&I2C1_CR2, (1U << 13), (1U << 13));                   // START=1

    while(!(read_bits(&I2C1_ISR, (1U << 1))));                     // TXIS
    write_reg(&I2C1_TXDR, reg);
    while(!(read_bits(&I2C1_ISR, (1U << 6))));                     // TC

    // read phase: RD_WRN=1, NBYTES=n, START, AUTOEND=1
    write_bits(&I2C1_CR2, (1U << 13), 0);                          // START=0
    write_bits(&I2C1_CR2, (0x3FFU << 0), (uint32_t)(dev7<<1));     // SADD
    write_bits(&I2C1_CR2, (1U << 10), (1U << 10));                   // RD_WRN=1
    write_bits(&I2C1_CR2, (0xFFU << 16), ((uint32_t)n << 16));       // NBYTES=n
    write_bits(&I2C1_CR2, (1U << 25), (1U << 25));                   // AUTOEND=1
    write_bits(&I2C1_CR2, (1U << 13), (1U << 13));                   // START=1

    for (uint8_t i=0; i<n; i++) {
        while(!(read_bits(&I2C1_ISR, (1U<<2))));                 // RXNE
        buf[i] = (uint8_t)read_bits(&I2C1_RXDR, 0xFFU);          // RXDR
    }

    while(!(read_bits(&I2C1_ISR, (1U<<5))));                     // STOPF
    write_bits(&I2C1_ICR, (1U<<5), (1U<<5));                     // clear STOPF
}
