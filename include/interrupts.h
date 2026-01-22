#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

/* Flags set by ISR, read by main loop */
extern volatile uint8_t exti0_flag;  /* PA0 (MPU INT) interrupt occurred */
extern volatile uint8_t exti1_flag;  /* PA1 (Contactor INT) interrupt occurred */
extern volatile uint8_t exti2_flag;  /* PA2 (FCU INT) interrupt occurred */
extern volatile uint8_t exti3_flag;  /* PA3 (Safe mode reset) interrupt occurred */

extern volatile uint8_t Sec_Timer;  /* TIM14 update interrupt occurred */

/**
 * EXTI0_1 Interrupt Handler.
 * Handles PA0 (MPU INT) and PA1 (Contactor INT).
 */
void EXTI0_1_IRQHandler(void);

/**
 * EXTI2_3 Interrupt Handler.
 * Handles PA2 (FCU INT) and PA3 (unused).
 */
void EXTI2_3_IRQHandler(void);

void TIM14_IRQHandler(void);

#endif