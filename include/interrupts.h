#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

/* Flags set by ISR, read by main loop */
extern volatile uint8_t exti0_flag;  /* PA0 interrupt occurred */
extern volatile uint8_t exti1_flag;  /* PA1 interrupt occurred */

extern volatile uint8_t LED_Timer;  /* TIM14 update interrupt occurred */

/**
 * EXTI0_1 Interrupt Handler.
 * Sets exti0_flag or exti1_flag depending on which line triggered.
 */
void EXTI0_1_IRQHandler(void);

void TIM14_IRQHandler(void);

#endif