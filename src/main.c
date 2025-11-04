#include <stdint.h>
#include <regutils.h>
#include <regaddr.h>
#include <module_configs.h>

#define MPU_ADDR 0x68
#define WHO_AM_I 0x75

unsigned char flag = 0b00000000;
uint32_t color = 0x08U;
uint8_t val = 0;
uint8_t raw[6];

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

static int16_t ax_g100(int16_t raw) 
{ 
    return (int32_t)raw * 100 / 16384;
}

static void print_accel_g100(int16_t ax, int16_t ay, int16_t az) {
    usart2_puts("AX="); usart2_put_i16(ax_g100(ax)); usart2_puts(" ");
    usart2_puts("AY="); usart2_put_i16(ax_g100(ay)); usart2_puts(" ");
    usart2_puts("AZ="); usart2_put_i16(ax_g100(az)); usart2_puts("\r\n");
}

void Color_Change()
{
    if (flag >> 0 & 0x01)
    {
        if (color >= 750)
        {
            color = 0;
            flag = 0b00000010;
        }
        write_reg(&TIM3_CCR1, color);
    }

    if (flag >> 1 & 0x01)
    {
        if (color >= 750)
        {
            color = 0;
            flag = 0b00000100;
        }
        write_reg(&TIM3_CCR2, color);
    }

    if (flag >> 2 & 0x01)
    {
        if (color >= 750)
        {
            color = 0;
            flag = 0b00000001;
        }
        write_reg(&TIM3_CCR4, color);
    }
}

uint32_t Next_Color(uint32_t* col)
{
    raw_delay(500);
    (*col)++;

    return (*col);
}

int main(void)
{
    *(volatile uint32_t*)0x200017FC = (uint32_t)raw;

    RCC_EnableClock();

    GPIOA_Config();
    GPIOB_Config();
    GPIOF_Config();

    TIM3_Config();
    I2C1_Config();
    USART2_Config();

    flag = 0b00000001;

    for(;;)
    {
        Color_Change();
        color = Next_Color(&color);

        I2C1_ReadN(MPU_ADDR, 0x3B, raw, 6);

        int16_t ax = (int16_t)((raw[0]<<8) | raw[1]);
        int16_t ay = (int16_t)((raw[2]<<8) | raw[3]);
        int16_t az = (int16_t)((raw[4]<<8) | raw[5]);

        print_accel_g100(ax, ay, az);
    }
}