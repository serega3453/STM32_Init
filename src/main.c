#include <stdint.h>
#include <regutils.h>
#include <regaddr.h>
#include <module_configs.h>
#include <i2cutils.h>
#include <mpuutils.h>

#define MPU_ADDR 0x68
#define WHO_AM_I 0x75

unsigned char flag = 0b00000000;
uint32_t color = 0x08U;
uint8_t val = 0;

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
    RCC_EnableClock();

    GPIOA_Config();
    GPIOB_Config();
    GPIOF_Config();
    
    TIM3_Config();
    I2C1_Config();
    USART1_Config();

    /* Enable MPU WOM: threshold=0xF0, ODR=0x08 (62.5Hz), DLPF=0x06 (5Hz filtering) */
        /* Enable MPU WOM: threshold=0xF0, ODR=0x08 (62.5Hz), DLPF=0x06 (5Hz filtering), AFS_SEL=2 (±8g)
         * Use AFS_SEL=2 (±8g) to avoid saturation on high-impact events while keeping decent sensitivity.
         */
        mpu_wom_enable_pp_high(MPU_ADDR, 0xFF, 0x08, 0x03, 0x00);

    flag = 0b00000001;

    for(;;)
    {
        Color_Change();
        color = Next_Color(&color);
    }
}