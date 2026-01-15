#include <stdint.h>
#include <regutils.h>
#include <regaddr.h>
#include <module_configs.h>
#include <i2cutils.h>
#include <mpuutils.h>
#include <interrupts.h>

/* MPU6050 I2C slave address (7-bit, 0x68 = 0b1101000) */
#define MPU_ADDR 0x68
/* MPU6050 WHO_AM_I register (should read back 0x68 or 0x71 depending on variant) */
#define WHO_AM_I 0x75

/* Global state variables for LED PWM control */
unsigned char flag = 0b00000000;      /* bit0/bit1/bit2 select which LED channel (CCR1/CCR2/CCR4) to illuminate */
uint32_t color = 0;               /* PWM compare value (0..2399, max is ARR=2399) */

/* Numbers from 0 to 7 select a desired LED color */
void Color_Selector(uint8_t color)
{
    // бит0 → G (CCR1)
    if (color & 0x01) write_reg(&TIM3_CCR1, 2399);
    else              write_reg(&TIM3_CCR1, 0);

    // бит1 → B (CCR2)
    if (color & 0x02) write_reg(&TIM3_CCR2, 2399);
    else              write_reg(&TIM3_CCR2, 0);

    // бит2 → R (CCR4)
    if (color & 0x04) write_reg(&TIM3_CCR4, 2399);
    else              write_reg(&TIM3_CCR4, 0);
}

/**
 * Main application entry point.
 * 1. Initializes clocks (RCC), GPIO ports, peripherals (TIM3, I2C1, USART1)
 * 2. Configures MPU6050 with WOM (Wake-On-Motion) interrupt capability
 * 3. Enters infinite loop cycling LED channels via PWM
 * 
 * MPU WOM configuration:
 * - WOM_THR=0xFF (can be tuned 0x00..0xFF; higher = less sensitive, used for impact detection)
 * - LP_ACCEL_ODR=0x08 (low-power acceleration output data rate, ~62.5 Hz)
 * - DLPF_CFG=0x03 (digital low-pass filter: ~41 Hz bandwidth)
 * - AFS_SEL=0x00 (±2g full-scale; 0=±2g, 1=±4g, 2=±8g, 3=±16g)
 * 
 * NOTE: Update these values directly in this function call if you need different WOM sensitivity.
 */
int main(void)
{
    RCC_EnableClock();

    GPIOA_Config();
    GPIOB_Config();
    GPIOF_Config();
    
    TIM3_Config();
    TIM14_Config();
    I2C1_Config();
    USART1_Config();
    EXTI_Config();

    /**
     * Enable MPU6050 WOM (Wake-On-Motion).
     * Parameters: dev7_addr, WOM_threshold, LP_ODR, DLPF_config, accel_full_scale
     * - 0xFF: WOM threshold (0x00=most sensitive, 0xFF=least sensitive)
     * - 0x08: Low-power ODR (~62.5 Hz)
     * - 0x03: DLPF_CFG (~41 Hz bandwidth)
     * - 0x00: AFS_SEL (0=±2g, 1=±4g, 2=±8g, 3=±16g)
     */
    mpu_preconfigure(MPU_ADDR, 0x00, 0x03);
    mpu_wom_enable_pp_high(MPU_ADDR, 0xA0, 0x08, 0x03, 0x00);

    flag = 0b00000001;

    /* Enable global interrupts */
    __asm("CPSIE i");  /* or use __enable_irq() if available */

    /* Infinite loop: cycle LED channels with PWM ramping */
    for(;;) {
        if (exti0_flag) {
            exti0_flag = 0;

            write_reg(&GPIOA_BSRR, 0x01 << 3);
            flag = 0b00000000;
            Color_Selector(0x04);

            usart1_puts("PA0 (MPU INT) detected!\r\n");
        }

        if (exti1_flag) {
            exti1_flag = 0;
            usart1_puts("PA1 (Contactor INT) detected!\r\n");
        }

        if (exti2_flag) {
            exti2_flag = 0;
            usart1_puts("PA2 (FCU INT) detected!\r\n");
        }

        if (LED_Timer && flag == 0b00000001) 
        {
            LED_Timer = 0;
            Color_Selector(color);
            /* Read and print accelerometer axes via UART using existing helper */
            MPU_ReadAccelRaw(MPU_ADDR, (uint8_t*)0);
            color = color + 1;
        }
    }
}