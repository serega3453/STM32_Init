#include <stdint.h>
#include "regaddr.h"
#include "regutils.h"
#include "mpuutils.h"

/* Global interrupt flags */
volatile uint8_t exti0_flag = 0;
volatile uint8_t exti1_flag = 0;
volatile uint8_t exti2_flag = 0;

volatile uint8_t LED_Timer = 0;

void EXTI0_1_IRQHandler(void)
{
    uint32_t pr = read_bits(&EXTI_PR, 0x03U);

    if (pr & (1U << 0)) {
        exti0_flag = 1;
        //clear_mpu_int();  // Clear MPU6050 interrupt by reading INT_STATUS
        write_bits(&EXTI_PR, (1U << 0), (1U << 0));
    }

    if (pr & (1U << 1)) {
        exti1_flag = 1;
        write_bits(&EXTI_PR, (1U << 1), (1U << 1));
    }
}

void TIM14_IRQHandler(void)
{
    /* Clear UIF by writing to SR (write-1-to-clear) */
    write_reg(&TIM14_SR, 0);
    LED_Timer = 1;
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
        exti2_flag = 1;
        write_bits(&EXTI_PR, (1U << 2), (1U << 2));
    }

    if (pr & (1U << 3)) {
        /* EXTI3 not used currently */
        write_bits(&EXTI_PR, (1U << 3), (1U << 3));
    }
}