#include <stdint.h>
#include "regaddr.h"
#include "regutils.h"

/* Global interrupt flags */
volatile uint8_t exti0_flag = 0;
volatile uint8_t exti1_flag = 0;

void EXTI0_1_IRQHandler(void)
{
    uint32_t pr = read_bits(&EXTI_PR, 0x03U);

    if (pr & (1U << 0)) {
        exti0_flag = 1;
        write_bits(&EXTI_PR, (1U << 0), (1U << 0));
    }

    if (pr & (1U << 1)) {
        exti1_flag = 1;
        write_bits(&EXTI_PR, (1U << 1), (1U << 1));
    }
}