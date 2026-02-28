#include <stdint.h>
#include "regaddr.h"
#include "regutils.h"

volatile uint8_t exti0_flag = 0;                //MPU6050 DATA_RDY
volatile uint8_t exti1_flag = 0;                //Contactor INT
volatile uint8_t exti2_flag = 0;                //FCU INT
volatile uint8_t exti4_flag = 0;                //Safe mode input (PA4)
volatile uint8_t exti5_flag = 1;                //Hard safe mode input (PA5)

volatile uint8_t Sec_Timer = 0;                 //1 second timer flag set by TIM14 interrupt

void EXTI0_1_IRQHandler(void)
{
    uint32_t pr = EXTI_PR;

    if (pr & (1U << 0)) {
        exti0_flag = 1;                         //MPU DATA_RDY
        write_reg(&EXTI_PR, (1U << 0));         // W1C
    }

    if (pr & (1U << 1)) {
        exti1_flag = 1;                         //Contactor INT
        write_reg(&EXTI_PR, (1U << 1));         // W1C
    }
}

void TIM14_IRQHandler(void)
{
    write_reg(&TIM14_SR, 0);
    Sec_Timer = 1;
}

void EXTI2_3_IRQHandler(void)
{
    uint32_t pr = EXTI_PR;

    if (pr & (1U << 2)) {
        exti2_flag = 1;                         //FCU INT
        write_reg(&EXTI_PR, (1U << 2));         // W1C
    }
}

void EXTI4_15_IRQHandler(void)
{
    uint32_t pr = EXTI_PR;

    if (pr & (1U << 4)) {
        exti4_flag = 1;                         //Set safe mode flag to trigger check_safe_mode() in main loop
        write_reg(&EXTI_PR, (1U << 4));         // W1C
    }

    if (pr & (1U << 5)) {
        write_reg(&EXTI_PR, (1U << 5));         // W1C
        write_reg(&SCB_AIRCR, 0x05FA0004U);     // System reset via AIRCR (VECTKEY=0x5FA, SYSRESETREQ=1)
        while (1) {}
    }
}