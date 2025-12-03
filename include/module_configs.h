#ifndef MODULE_CONFIGS_H
#define MODULE_CONFIGS_H

/**
 * Peripheral configuration functions for STM32F070.
 * These functions initialize clocks, GPIO, and peripherals (TIM3, I2C1, USART1).
 * Should be called during system startup in main() before using any hardware.
 */

/**
 * Enable peripheral clocks for TIM3, I2C1, USART1, SYSCFG, and GPIO ports A/B/F.
 * Includes stabilization delays.
 */
void RCC_EnableClock(void);

/**
 * Configure GPIOA pins:
 * - PA6, PA7: TIM3 PWM (LED channels 0, 1)
 * - PA9, PA10: USART1 TX/RX
 * - PA0: MPU6050 interrupt (input, low-active)
 * - PA1: External interrupt (input)
 * - PA2: Logic input (pull-up)
 * - PA3: Digital output (MOSFET gate control)
 */
void GPIOA_Config(void);

/**
 * Configure GPIOB pins:
 * - PB1: TIM3 PWM (LED channel 2)
 */
void GPIOB_Config(void);

/**
 * Configure GPIOF pins:
 * - PF0, PF1: I2C1 SCL/SDA (open-drain, external pull-ups required)
 */
void GPIOF_Config(void);

/**
 * Configure TIM3 for PWM generation on channels 1, 2, 4.
 * - Frequency: ~16.7 kHz (ARR=2399, PSC=0 at 48 MHz core)
 * - Outputs: TIM3_CH1 (PA6), TIM3_CH2 (PA7), TIM3_CH4 (PB1)
 * - Duty cycle controlled via CCR1, CCR2, CCR4 (0..2399 = 0..100%)
 */
void TIM3_Config(void);

/**
 * Configure I2C1 for MPU6050 communication.
 * - Speed: ~400 kHz (I2C standard mode)
 * - Pins: PF0 (SCL), PF1 (SDA)
 */
void I2C1_Config(void);

/**
 * Configure USART1 for serial communication.
 * - Baudrate: 115200 bps
 * - Format: 8 data bits, 1 stop bit, no parity
 * - Pins: PA9 (TX), PA10 (RX)
 */
void USART1_Config(void);

/**
 * Configure EXTI (External Interrupt) lines 0 and 1.
 * - Configures SYSCFG_EXTICR1 to route PA0/PA1 to EXTI0/EXTI1
 * - Sets rising-edge triggers for both lines
 * - Enables interrupt mask and NVIC IRQ#5 (EXTI0_1_IRQn)
 */
void EXTI_Config(void);

#endif