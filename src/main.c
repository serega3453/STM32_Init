#include <stdint.h>
#include <regutils.h>
#include <regaddr.h>
#include <module_configs.h>
#include <i2cutils.h>
#include <mpuutils.h>
#include <interrupts.h>

/* MPU6050 I2C slave address (7-bit, 0x68 = 0b1101000) */
#define MPU_ADDR 0x68
unsigned char flag;
uint32_t color;

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

void check_safe_mode(void)
{
    /* Check PA4 (safe mode pin) state and set/clear safe mode flag (bit 1) */
    if (read_bits(&GPIOA_IDR, (1U << 4)))
    {
        flag |= (1 << 1);   // Set safe mode flag
        EXTI_Switch(0);   // Enable EXTI
    } 

    else 
    {
        flag &= ~(1 << 1);  // Clear safe mode flag
        EXTI_Switch(1);   // Disable EXTI
    }
}

int main(void)
{
    /*
    bit 0 - LED cycling control 
    bit 1 - safe mode timer control
    bit 2 - safe mode FCU signal control
    */
    flag = 0b00000010;    //safe mode enabled, LED cycling off
    color = 0;               /* PWM compare value (0..2399, max is ARR=2399) */

    uint8_t safe_timer_value = 120;
    uint8_t safe_timer_count = 0;

    /* Impact detection sensitivity (raw LSB units). Larger = less sensitive. Tweak as needed. */
    uint16_t impact_sensitivity = 0xBFFF;

    RCC_EnableClock();

    GPIOA_Config();
    GPIOB_Config();
    GPIOF_Config();
    
    TIM3_Config();
    TIM14_Config();
    I2C1_Config();
    USART1_Config();
    EXTI_Config();
    
    mpu_preconfigure(MPU_ADDR, 0x00, 0x03);

    __asm("CPSIE i");   //Enable global interrupts

    Color_Selector(0x01);   //Light solid GREEN LED
    usart1_puts("SM\r\n");

    for(;(flag >> 1) & 1;)
    {
        if (Sec_Timer)
        {
            Sec_Timer = 0;
            safe_timer_count++;

            uint8_t v = safe_timer_value - safe_timer_count;

            usart1_putc("0123456789ABCDEF"[v >> 4]);
            usart1_putc("0123456789ABCDEF"[v & 0x0F]);
            usart1_putc('\r');
            usart1_putc('\n');

            if (safe_timer_count >= safe_timer_value)
            {
                flag &= ~(1<<1); //Reset safe mode flag after *safe_timer_value* seconds
            }
        }
    }

    Color_Selector(0x05);                           //Light solid YELLOW LED
    usart1_puts("SM\r\n");                          //Notice about end of safe mode
    exti0_flag = 0;                                 //Clear any pending MPU INT flag
    exti1_flag = 0;                                 //Clear any pending Contactor INT flag
    exti2_flag = 0;                                 //Clear any pending FCU INT flag

    for(;;)
    {
        check_safe_mode();

        if ((flag >> 1) & 1)        //Safe mode active
        {
            Color_Selector(0x02);   //Light solid BLUE LED
        }
        else
        {
            Color_Selector(0x05);   //Light solid YELLOW LED
        }

        if ((flag >> 1) == 0)       //Normal operation
        {
            if (exti0_flag) 
            {
                exti0_flag = 0;

                if (mpu_detect_impact(MPU_ADDR, impact_sensitivity)) 
                {
                    write_reg(&GPIOA_BSRR, 0x01 << 3);  //Raise the pin (set PA3 or configured output)
                    flag &= ~(1 << 0);                  /* stop LED cycling */
                    Color_Selector(0x04);               //Light solid RED LED
                    usart1_puts("INT_MPU\r\n");
                
                    for (;;) 
                    {
                        __asm("WFI");                   //Stop execution, INIT set
                    }
                } 
            }

            if (exti1_flag) 
            {
                exti1_flag = 0;
                flag &= ~(1 << 0); /* stop LED cycling */
                Color_Selector(0x04); /* solid red to indicate impact */
                usart1_puts("INT_CON\r\n");

                for (;;) 
                {
                    __asm("WFI");                   //Stop execution, INIT set
                }
            }

            if (exti2_flag) 
            {
                exti2_flag = 0;
                flag &= ~(1 << 0); /* stop LED cycling */
                Color_Selector(0x04); /* solid red to indicate impact */
                usart1_puts("INT_FCU\r\n");

                for (;;) 
                {
                    __asm("WFI");                   //Stop execution, INIT set
                }
            }
        }
    }
}