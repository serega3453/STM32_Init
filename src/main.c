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
uint32_t color = 0x08U;               /* PWM compare value (0..2399, max is ARR=2399) */

/**
 * Cycle through LED channels with PWM intensity ramping.
 * Each call increments PWM compare value until it reaches 750,
 * then resets and switches to the next LED channel (TIM3_CCR1 -> CCR2 -> CCR4 -> CCR1).
 * flag bits determine which output is active (RMW pattern on GPIOA PA3 or TIM3).
 */
void Color_Change()
{
    /* Channel 0: control TIM3_CCR1 (LED1) when bit0 is set */
    if (flag >> 0 & 0x01)
    {
        if (color >= 750)
        {
            color = 0;
            flag = 0b00000010;  /* Rotate to channel 1 (CCR2) */
        }
        write_reg(&TIM3_CCR1, color);
    }

    /* Channel 1: control TIM3_CCR2 (LED2) when bit1 is set */
    if (flag >> 1 & 0x01)
    {
        if (color >= 750)
        {
            color = 0;
            flag = 0b00000100;  /* Rotate to channel 2 (CCR4) */
        }
        write_reg(&TIM3_CCR2, color);
    }

    /* Channel 2: control TIM3_CCR4 (LED3) when bit2 is set */
    if (flag >> 2 & 0x01)
    {
        if (color >= 750)
        {
            color = 0;
            flag = 0b00000001;  /* Rotate back to channel 0 (CCR1) */
        }
        write_reg(&TIM3_CCR4, color);
    }
}

/**
 * Increment color value at a controlled pace (simple pacing delay).
 * Returns the updated value for application use.
 * col - pointer to color value variable to increment
 */
uint32_t Next_Color(uint32_t* col)
{
    raw_delay(500);
    (*col)++;

    return (*col);
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
    mpu_wom_enable_pp_high(MPU_ADDR, 0xFF, 0x08, 0x03, 0x00);

    flag = 0b00000001;

    /* Infinite loop: cycle LED channels with PWM ramping */
    for(;;) {
        if (exti0_flag) {
            exti0_flag = 0;
            usart1_puts("PA0 event detected!\r\n");
        }

        if (exti1_flag) {
            exti1_flag = 0;
            usart1_puts("PA1 event detected!\r\n");
        }

        Color_Change();
        color = Next_Color(&color);
    }
}