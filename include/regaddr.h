#ifndef REGADDR_H
#define REGADDR_H

/**
 * Register address definitions for STM32F070F6 microcontroller.
 * vol() macro: cast an address to a volatile 32-bit register pointer for safe hardware access.
 */

#define vol(addr) (*(volatile uint32_t*)(addr))

/* Peripheral base addresses (AHB/APB memory map) */
#define RCC_BASE 0x40021000U          /* Reset and Clock Control */
#define GPIOA_BASE 0x48000000U        /* GPIO Port A */
#define GPIOB_BASE 0x48000400U        /* GPIO Port B */
#define GPIOF_BASE 0x48001400U        /* GPIO Port F */
#define TIM3_BASE 0x40000400U         /* Timer 3 */
#define TIM14_BASE 0x40002000U        /* Timer 14 */
#define I2C1_BASE 0x40005400U         /* I2C 1 */
#define USART1_BASE 0x40013800U       /* UART 1 */
#define SYSCFG_BASE 0x40010000U       /* SYSCFG */
#define EXTI_BASE 0x40010400U         /* EXTI */

/* SYSCFG + EXTI registers */
#define SYSCFG_EXTICR1 vol(SYSCFG_BASE + 0x08U)   /* EXTI Config for lines 0..3 */
/* SYSCFG_EXTICR2 covers EXTI lines 4..7 (offset 0x0C) */
#define SYSCFG_EXTICR2 vol(SYSCFG_BASE + 0x0CU)   /* EXTI Config for lines 4..7 */
#define EXTI_IMR vol(EXTI_BASE + 0x00U)            /* Interrupt Mask */
#define EXTI_EMR vol(EXTI_BASE + 0x04U)            /* Event Mask */
#define EXTI_RTSR vol(EXTI_BASE + 0x08U)           /* Rising Trigger */
#define EXTI_FTSR vol(EXTI_BASE + 0x0CU)           /* Falling Trigger */
#define EXTI_SWIER vol(EXTI_BASE + 0x10U)          /* Software Interrupt Event */
#define EXTI_PR vol(EXTI_BASE + 0x14U)             /* Pending Register */

/* RCC register offsets */
#define RCC_AHBENR vol(RCC_BASE + 0x14U)     /* AHB Enable (GPIO clocks) */
#define RCC_APB1ENR vol(RCC_BASE + 0x1CU)    /* APB1 Enable (TIM3, I2C1) */
#define RCC_APB2ENR vol(RCC_BASE + 0x18U)    /* APB2 Enable (USART1, SYSCFG) */

/* GPIOA registers */
#define GPIOA_MODER vol(GPIOA_BASE + 0x00U)    /* Mode register (00=input, 01=output, 10=AF, 11=analog) */
#define GPIOA_OTYPER vol(GPIOA_BASE + 0x04U)   /* Output type (0=push-pull, 1=open-drain) */
#define GPIOA_OSPEEDR vol(GPIOA_BASE + 0x08U)  /* Output speed */
#define GPIOA_PUPDR vol(GPIOA_BASE + 0x0CU)    /* Pull-up/pull-down (00=none, 01=pull-up, 10=pull-down) */
#define GPIOA_AFRL vol(GPIOA_BASE + 0x20U)     /* Alternate function low (pins 0..7) */
#define GPIOA_AFRH vol(GPIOA_BASE + 0x24U)     /* Alternate function high (pins 8..15) */
#define GPIOA_BSRR vol(GPIOA_BASE + 0x18U)     /* Bit set/reset register */
#define GPIOA_IDR vol(GPIOA_BASE + 0x10U)      /* Input data register */

/* GPIOB registers */
#define GPIOB_MODER vol(GPIOB_BASE + 0x00U)
#define GPIOB_OTYPER vol(GPIOB_BASE + 0x04U)
#define GPIOB_OSPEEDR vol(GPIOB_BASE + 0x08U)
#define GPIOB_PUPDR vol(GPIOB_BASE + 0x0CU)
#define GPIOB_AFRL vol(GPIOB_BASE + 0x20U)

/* GPIOF registers */
#define GPIOF_MODER vol(GPIOF_BASE + 0x00U)
#define GPIOF_OTYPER vol(GPIOF_BASE + 0x04U)
#define GPIOF_OSPEEDR vol(GPIOF_BASE + 0x08U)
#define GPIOF_PUPDR vol(GPIOF_BASE + 0x0CU)
#define GPIOF_AFRL vol(GPIOF_BASE + 0x20U)

/* TIM3 registers (PWM generation) */
#define TIM3_CR1 vol(TIM3_BASE + 0x00U)      /* Control register 1 (CEN=enable) */
#define TIM3_EGR vol(TIM3_BASE + 0x14U)      /* Event generation (UG=update) */
#define TIM3_CCMR1 vol(TIM3_BASE + 0x18U)    /* Capture/Compare Mode Register 1 (CH1, CH2 PWM config) */
#define TIM3_CCMR2 vol(TIM3_BASE + 0x1CU)    /* Capture/Compare Mode Register 2 (CH3, CH4 PWM config) */
#define TIM3_CCER vol(TIM3_BASE + 0x20U)     /* Capture/Compare Enable Register (CC1E, CC2E, CC3E, CC4E) */
#define TIM3_CNT vol(TIM3_BASE + 0x24U)      /* Counter value (read-only during run) */
#define TIM3_PSC vol(TIM3_BASE + 0x28U)      /* Prescaler (clock divider) */
#define TIM3_ARR vol(TIM3_BASE + 0x2CU)      /* Auto-reload register (period - 1) */
#define TIM3_CCR1 vol(TIM3_BASE + 0x34U)     /* Capture/Compare Register 1 (PWM duty for CH1) */
#define TIM3_CCR2 vol(TIM3_BASE + 0x38U)     /* Capture/Compare Register 2 (PWM duty for CH2) */
#define TIM3_CCR4 vol(TIM3_BASE + 0x40U)     /* Capture/Compare Register 4 (PWM duty for CH4) */

#define TIM14_CR1 vol(TIM14_BASE + 0x00U)      /* Control register 1 (CEN=enable, ARPE=auto-reload preload) */
#define TIM14_DIER vol(TIM14_BASE + 0x0CU)    /* DMA/Interrupt Enable Register (UIE=update interrupt enable) */
#define TIM14_SR vol(TIM14_BASE + 0x10U)      /* Status register (UIF=update interrupt flag) */
#define TIM14_EGR vol(TIM14_BASE + 0x14U)     /* Event generation register (UG=update generation) */
#define TIM14_CNT vol(TIM14_BASE + 0x24U)      /* Counter value (read-only during run) */
#define TIM14_PSC vol(TIM14_BASE + 0x28U)     /* Prescaler (clock divider) */
#define TIM14_ARR vol(TIM14_BASE + 0x2CU)     /* Auto-reload register (period - 1) */

/* I2C1 registers */
#define I2C1_CR1 vol(I2C1_BASE + 0x00U)       /* Control register 1 (PE=enable) */
#define I2C1_TIMINGR vol(I2C1_BASE + 0x10U)   /* Timing register (clock/rise/fall config) */
#define I2C1_CR2 vol(I2C1_BASE + 0x04U)       /* Control register 2 (AUTOEND, NBYTES, START, STOP, RD_WRN) */
#define I2C1_ISR vol(I2C1_BASE + 0x18U)       /* Interrupt and Status (TXIS, RXNE, TC, STOPF) */
#define I2C1_RXDR vol(I2C1_BASE + 0x24U)      /* Receive data register (read-only) */
#define I2C1_TXDR vol(I2C1_BASE + 0x28U)      /* Transmit data register (write-only) */
#define I2C1_ICR vol(I2C1_BASE + 0x1CU)       /* Interrupt clear register */

/* USART1 registers */
#define USART1_CR1 vol(USART1_BASE + 0x00U)   /* Control register 1 (UE=enable, TE, RE) */
#define USART1_CR2 vol(USART1_BASE + 0x04U)   /* Control register 2 (STOP bits) */
#define USART1_BRR vol(USART1_BASE + 0x0CU)   /* Baud rate register */
#define USART1_ISR vol(USART1_BASE + 0x1CU)   /* Interrupt and Status (TXE, RXNE, etc) */
#define USART1_TDR vol(USART1_BASE + 0x28U)   /* Transmit data register */

#endif