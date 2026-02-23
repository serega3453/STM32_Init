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
    // бит0 → R (CCR1)
    if (color & 0x01) write_reg(&TIM3_CCR1, 2399);
    else              write_reg(&TIM3_CCR1, 0);

    // бит1 → G (CCR2)
    if (color & 0x02) write_reg(&TIM3_CCR2, 2399);
    else              write_reg(&TIM3_CCR2, 0);

    // бит2 → B (CCR4)
    if (color & 0x04) write_reg(&TIM3_CCR4, 2399);
    else              write_reg(&TIM3_CCR4, 0);
}

void check_safe_mode(void)
{
    if (exti4_flag)
    {
        if (read_bits(&GPIOA_IDR, (1U << 4)))
        {
            flag |= (1 << 1);   // Set safe mode flag
            EXTI_Switch(0);   // Disable EXTI

            Color_Selector(0x04);   //Light solid BLUE LED
            usart1_puts("SM_S\r\n");
        }
        else 
        {
            flag &= ~(1 << 1);  // Clear safe mode flag
            EXTI_Switch(1);   // Enable EXTI

            Color_Selector(0x03);   //Light solid YELLOW LED
            usart1_puts("SM_R\r\n");
        }
        exti4_flag = 0;   // Clear safe mode interrupt flag
    }
}

int main(void)
{
    /*
    bit 0 - LED cycling control 
    bit 1 - safe mode control
    bit 2 - hard safe mode control
    */
    flag = 0b00000000;       //safe mode disabled, LED cycling off
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
    
    //mpu_preconfigure(MPU_ADDR, 0x00, 0x03);

    __asm("CPSIE i");   //Enable global interrupts

    exti5_flag = read_bits(&GPIOA_IDR, (1U << 5));

    Color_Selector(0x00);   //All LEDs off
    usart1_puts("HS_S\r\n");
    usart1_puts("INT_NOT\r\n");

    while(read_bits(&GPIOA_IDR, (1U << 5)))
    {
        __asm("WFI");   //Wait for interrupt (low power standby)
    }

    usart1_puts("HS_R\r\n");
    Color_Selector(0x02);   //Light solid GREEN LED

    while(1)
    {
        
    }

    for(;;)     /*Start loop*/
    {
        if (Sec_Timer)
        {
            Sec_Timer = 0;
            safe_timer_count++;

            uint8_t v = safe_timer_value - safe_timer_count;

            usart1_puts("TIM\r\n");
            usart1_puts("INT_NOT\r\n");

            usart1_putc("0123456789ABCDEF"[v >> 4]);
            usart1_putc("0123456789ABCDEF"[v & 0x0F]);
            usart1_putc('\r');
            usart1_putc('\n');

            if (safe_timer_count >= safe_timer_value)
            {
                break;
            }
        }
    }

    exti0_flag = 0;                                 //Clear any pending MPU INT flag
    exti1_flag = 0;                                 //Clear any pending Contactor INT flag
    exti2_flag = 0;                                 //Clear any pending FCU INT flag

    exti4_flag = 1;                                 //Set safe mode flag to trigger check

    for(;;)     /*Main loop*/
    {
        __asm("WFI");   //Wait for interrupt (low power standby)
        check_safe_mode();

        if ((flag >> 1) == 0)       //Normal operation
        {
            if (exti0_flag) 
            {
                exti0_flag = 0;

                if (mpu_detect_impact(MPU_ADDR, impact_sensitivity)) 
                {
                    write_reg(&GPIOA_BSRR, 0x01 << 3);  //Raise the pin (set PA3 or configured output)
                    flag &= ~(1 << 0);                  /* stop LED cycling */
                    Color_Selector(0x01);               //Light solid RED LED
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
                Color_Selector(0x01); /* solid red to indicate impact */
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
                Color_Selector(0x01); /* solid red to indicate impact */
                usart1_puts("INT_FCU\r\n");

                for (;;) 
                {
                    __asm("WFI");                   //Stop execution, INIT set
                }
            }
        }
    }
}