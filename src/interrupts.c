#include <stdint.h>
#include "regaddr.h"
#include "regutils.h"

/* Global interrupt flags */
volatile uint8_t exti0_flag = 0;    //MPU6050 DATA_RDY
volatile uint8_t exti1_flag = 0;    //Contactor INT
volatile uint8_t exti2_flag = 0;    //FCU INT
volatile uint8_t exti4_flag = 0;    //Safe mode input (PA4)
volatile uint8_t exti5_flag = 1;    //Hard safe mode input (PA5)

volatile uint8_t Sec_Timer = 0;

void EXTI0_1_IRQHandler(void)
{
    uint32_t pr = read_bits(&EXTI_PR, 0x03U);

    if (pr & (1U << 0)) {
        exti0_flag = 1;   //MPU DATA_RDY
        write_reg(&EXTI_PR, (1U << 0));   // W1C
    }

    if (pr & (1U << 1)) {
        exti1_flag = 1;   //Contactor INT
        write_reg(&EXTI_PR, (1U << 1));   // W1C
    }
}

void TIM14_IRQHandler(void)
{
    /* Clear UIF by writing to SR (write-1-to-clear) */
    write_reg(&TIM14_SR, 0);
    Sec_Timer = 1;

    //if (read_bits(&GPIOA_IDR, (1U << 5))) 
    //{
        //write_reg(&SCB_AIRCR, 0x05FA0004U); //reset
        //while(1);                           //wait for reset
    //}
    //else 
    //{
        //exti5_flag = 0;
    //}
}

/**
 * EXTI2_3 Interrupt Handler.
 * Handles both EXTI2 (PA2/FCU INT) and EXTI3.
 * In this project, only EXTI2 is configured.
 */
void EXTI2_3_IRQHandler(void)
{
    uint32_t pr = read_bits(&EXTI_PR, 0x0CU);  /* Check EXTI2 and EXTI3 pending bits */

    if (pr & (1U << 2)) {
        exti2_flag = 1;  //FCU INT
        write_bits(&EXTI_PR, (1U << 2), (1U << 2));
    }

    /* EXTI3 is unused in this project (safe-mode moved to PA4/EXTI4) */
}

/**
 * EXTI4_15_IRQHandler handles EXTI lines 4..15. We only care about EXTI4 (PA4 safe-mode).
 */
void EXTI4_15_IRQHandler(void)
{
    uint32_t pr = EXTI_PR;

    usart1_puts("EXTI4_15 IRQ PR=");
    for (int i = 7; i >= 0; --i)
        usart1_putc("0123456789ABCDEF"[(pr >> (i*4)) & 0x0F]);
    usart1_puts("\r\n");

    if (pr & (1U << 4)) {
        exti4_flag = 1;
        write_reg(&EXTI_PR, (1U << 4));   // W1C
    }

    if (pr & (1U << 5)) {
        write_reg(&EXTI_PR, (1U << 5));   // W1C
        write_reg(&SCB_AIRCR, 0x05FA0004U);
        while (1) {}
    }
}