#include <regutils.h>
#include <regaddr.h>

/**
 * Enable clock gates for all peripherals used in this application.
 * Enables: TIM3, I2C1, USART1 (APB1/APB2), SYSCFG, and GPIO ports A/B/F (AHB).
 * Includes small delays to allow clock stabilization before configuration.
 */
void RCC_EnableClock()
{
    write_bits(&RCC_APB1ENR, (0x01U << 1), (0x01U << 1));              /* TIM3_EN */
    write_bits(&RCC_APB1ENR, (0x01U << 8), (0x01U << 8));                    // TIM14_EN
    write_bits(&RCC_APB1ENR, (0x01U << 21), (0x01U << 21));            /* I2C1_EN */
    write_bits(&RCC_APB2ENR, (0x01U << 14), (0x01U << 14));            /* USART1_EN (on APB2) */
    write_bits(&RCC_APB2ENR, (0x01U << 0), (0x01U << 0));              /* SYSCFG_EN */
    raw_delay(10000);
    write_bits(&RCC_AHBENR, (0x03U << 17), 0x03U << 17);               /* GPIOA_EN && GPIOB_EN */
    write_bits(&RCC_AHBENR, (0x01U << 22), (0x01U << 22));             /* GPIOF_EN */

    raw_delay(10000);
}

/**
 * Configure GPIOA pins for project use:
 * - PA6, PA7: TIM3 PWM outputs (LED channels 0,1) via AF1
 * - PA9, PA10: USART1 TX/RX (AF1) for serial communication
 * - PA0: MPU6050 interrupt input (rising edge)
 * - PA1: Contactor interrupt input (rising edge)
 * - PA2: FCU interrupt input (rising edge)
 * - PA3: Digital output (MOSFET gate control, push-pull, high-speed)
 */
void GPIOA_Config()
{
    /* PA6, PA7 - TIM3 PWM outputs (LED channels) */
    write_bits(&GPIOA_MODER, (0x0FU << 12), (0x0AU << 12)); //PA6_AF && PA7_AF
    write_bits(&GPIOA_OTYPER, (0x03U << 6), (0x00U << 6)); //PA6_PP && PA7_PP
    write_bits(&GPIOA_OSPEEDR, (0x0FU << 12), (0x0FU << 12)); //PA6_HS && PA7_HS
    write_bits(&GPIOA_PUPDR, (0x0FU << 12), (0x00U << 12)); //PA6_PUNPD && PA7_PUNPD
    write_bits(&GPIOA_AFRL, (0xFFU << 24), (0x11U << 24)); //PA6_AF1 && PA7_AF1

    /* PA9, PA10 - USART1 TX/RX (AF1) */
    write_bits(&GPIOA_MODER, (0x0FU << 18), (0x0AU << 18)); // PA9_AF && PA10_AF
    write_bits(&GPIOA_OTYPER, (0x03U << 9), (0x00U << 9));  // push-pull
    write_bits(&GPIOA_OSPEEDR, (0x0FU << 18), (0x0FU << 18)); // high speed
    write_bits(&GPIOA_PUPDR, (0x0FU << 18), (0x04U << 18)); // PA9_NPUNPD, PA10_PU
    write_bits(&GPIOA_AFRH, (0xFFU << 4), (0x11U << 4)); // AF1 для обеих

    //GPIOA PA0 - Input (MPU INT)
    write_bits(&GPIOA_MODER, (0x03U << 0), (0x00U << 0)); //PA0_IN
    write_bits(&GPIOA_OTYPER, (0x01U << 0), (0x00U << 0)); //PA0_PP
    write_bits(&GPIOA_OSPEEDR, (0x03U << 0), (0x03U << 0)); //PA0_HS
    write_bits(&GPIOA_PUPDR, (0x03U << 0), (0x02U << 0)); //PA0_PD

    //GPIOA PA1 - Input (Contactor INT)
    write_bits(&GPIOA_MODER, (0x03U << 2), (0x00U << 2)); //PA1_IN
    write_bits(&GPIOA_OTYPER, (0x01U << 1), (0x00U << 1)); //PA1_PP
    write_bits(&GPIOA_OSPEEDR, (0x03U << 2), (0x03U << 2)); //PA1_HS
    write_bits(&GPIOA_PUPDR, (0x03U << 2), (0x02U << 2)); //PA1_PD

    /* PA2 - Input (FCU INT) */
    write_bits(&GPIOA_MODER,   (0x03U << 4), (0x00U << 4));           /* PA2_IN */
    write_bits(&GPIOA_OTYPER,  (0x01U << 2), (0x00U << 2));           /* PA2_PP */
    write_bits(&GPIOA_OSPEEDR, (0x03U << 4), (0x03U << 4));           /* PA2_HS */
    write_bits(&GPIOA_PUPDR,   (0x03U << 4), (0x02U << 4));           /* PA2_PD */

    /* PA3 - MOSFET gate output (digital push-pull, high-speed) */
    write_bits(&GPIOA_MODER,   (0x03U << 6), (0x01U << 6));           /* PA3_OUT */
    write_bits(&GPIOA_OTYPER,  (0x01U << 3), (0x00U << 3));           /* PA3_PP (push-pull) */
    write_bits(&GPIOA_OSPEEDR, (0x03U << 6), (0x03U << 6));           /* PA3_HS (high-speed) */
    write_bits(&GPIOA_PUPDR,   (0x03U << 6), (0x00U << 6));           /* PA3_NO_PULL */

    /* PA4 - Safe mode input from FCU */
    write_bits(&GPIOA_MODER,   (0x03U << 8), (0x00U << 8));           /* PA4_IN */
    write_bits(&GPIOA_OTYPER,  (0x01U << 4), (0x00U << 4));           /* PA4_PP */
    write_bits(&GPIOA_OSPEEDR, (0x03U << 8), (0x03U << 8));           /* PA4_HS */
    write_bits(&GPIOA_PUPDR,   (0x03U << 8), (0x02U << 8));           /* PA4_PD */
}

/**
 * Configure GPIOB pins for project use:
 * - PB1: TIM3 PWM output (LED channel 2) via AF1
 */
void GPIOB_Config()
{
    /* PB1 - TIM3 PWM output (LED channel 2) via AF1 */
    write_bits(&GPIOB_MODER, (0x03U << 2), (0x02U << 2));
    write_bits(&GPIOB_OTYPER, (0x01U << 1), (0x00U << 1));
    write_bits(&GPIOB_OSPEEDR, (0x03U << 2), (0x03U << 2));
    write_bits(&GPIOB_PUPDR, (0x03U << 2), (0x00U << 2));
    write_bits(&GPIOB_AFRL, (0x0FU << 4), (0x01U << 4));
}

/**
 * Configure GPIOF pins for project use:
 * - PF0, PF1: I2C1 SCL/SDA (open-drain, pull-ups built into I2C bus)
 */
void GPIOF_Config()
{
    /* PF0, PF1 - I2C1 SCL/SDA (open-drain, I2C bus pull-ups required externally) */
    write_bits(&GPIOF_MODER, (0x0FU << 0), (0x0AU << 0));
    write_bits(&GPIOF_OTYPER, (0x03U << 0), (0x03U << 0));
    write_bits(&GPIOF_OSPEEDR, (0x0FU << 0), (0x0FU << 0));
    write_bits(&GPIOF_PUPDR, (0x0FU << 0), (0x00U << 0));
    write_bits(&GPIOF_AFRL, (0xFFU << 0), (0x11U << 0));
}

/**
 * Configure TIM3 for PWM generation on channels 1, 2, 4.
 * Configuration:
 * - PSC = 0 (no prescale)
 * - ARR = 2399 (PWM period; 0..2399 gives ~16.7 kHz at 48 MHz core freq)
 * - CCMR1/CCMR2: PWM mode (mode 1 or 2, output on compare match)
 * - CCER: enables all three outputs (CC1E, CC2E, CC4E)
 * - CCR1/CCR2/CCR4: set to 0 initially (0% duty cycle, can be changed in main loop)
 */
void TIM3_Config()
{
    /* Disable timer during configuration */
    write_bits(&TIM3_CR1, (0x01U << 0), (0x00U << 0));
    write_bits(&TIM3_CR1, (0x01U << 7), (0x01U << 7));
    write_reg(&TIM3_PSC, 0x00000000);
    write_reg(&TIM3_ARR, 2399);
    write_reg(&TIM3_CNT, 0x00000000);
    write_reg(&TIM3_CCMR1, 0x00006868);
    write_reg(&TIM3_CCMR2, 0x00006800);
    write_reg(&TIM3_CCER, 0x00001011);
    write_reg(&TIM3_CCR1, 0x00000000);
    write_reg(&TIM3_CCR2, 0x00000000);
    write_reg(&TIM3_CCR4, 0x00000000);
    write_reg(&TIM3_EGR, 0x00000001);
    write_bits(&TIM3_CR1, (0x01U << 0), (0x01U << 0));
}

/**
 * Configure TIM14 for 1-second periodic interrupt.
 * Assumes APB1 clock = 8 MHz (default HSI after reset)
 * - PSC = 7999: divides 8MHz to 1 kHz
 * - ARR = 999: period = 1000 ticks at 1 kHz = 1 second
 * - Generates TIM14_IRQHandler interrupt every 1 second
 */
void TIM14_Config()
{
    /* Disable timer before configuration */
    write_bits(&TIM14_CR1, (1U<<0), 0);            /* CEN=0: stop timer */
    
    /* Configure prescaler and period */
    write_reg(&TIM14_PSC, 7999);                   /* PSC: 8MHz / 8000 = 1 kHz */
    write_reg(&TIM14_ARR, 999);                    /* ARR: 1000 ticks = 1 second */
    write_reg(&TIM14_CNT, 0);                      /* Reset counter */
    
    /* Enable update interrupt */
    write_bits(&TIM14_DIER, (1U<<0), (1U<<0));     /* UIE=1: enable update interrupt */
    
    /* Enable NVIC IRQ 19 BEFORE starting timer */
    volatile uint32_t* NVIC_ISER0 = (volatile uint32_t*)0xE000E100U;
    *NVIC_ISER0 = (1U << 19);                      /* Enable TIM14_IRQn (IRQ 19) */
    
    /* Start timer */
    write_bits(&TIM14_CR1, (1U<<0), (1U<<0));      /* CEN=1: start timer */
}

/**
 * Configure I2C1 for communication with MPU6050.
 * Timing: configured for ~400 kHz (I2C standard mode) at 48 MHz core frequency.
 * TIMINGR register is pre-calculated: 0x00201D2B.
 */
void I2C1_Config()
{
    /* Disable I2C before writing TIMINGR (register is locked when I2C is enabled) */
    write_bits(&I2C1_CR1, (0x01U << 0), (0x00U << 0));
    write_reg(&I2C1_TIMINGR, (0x00201D2B));
    write_bits(&I2C1_CR1, (0x01U << 0), (0x01U << 0));
}

/**
 * Configure USART1 for serial communication (debugging, data logging).
 * Baudrate: 115200 bps (BRR = 0x0045 for 48 MHz PCLK).
 * Data bits: 8, Stop bits: 1, Parity: none, Flow control: none.
 */
void USART1_Config()
{
    write_bits(&USART1_CR1, (1U<<0), 0);        /* Disable USART before configuration */
    write_reg(&USART1_BRR, 0x0045);
    write_bits(&USART1_CR2, (3U<<12), 0);
    write_bits(&USART1_CR1, (1U<<3), (1U<<3));
    write_bits(&USART1_CR1, (1U<<2), (1U<<2));
    write_bits(&USART1_CR1, (1U<<0), (1U<<0)); /* Enable USART */
}

/**
 * Configure EXTI (External Interrupt) for PA0, PA1, PA2.
 * - PA0 (EXTI0): MPU6050 motion detection (rising edge)
 * - PA1 (EXTI1): Contactor interrupt (rising edge)
 * - PA2 (EXTI2): FCU interrupt (rising edge)
 * EXTI0/EXTI1 share EXTI0_1_IRQHandler (IRQ #5)
 * EXTI2/EXTI3 share EXTI2_3_IRQHandler (IRQ #6)
 */
void EXTI_Config(void)
{
    /* Step 1: Connect PA0, PA1, PA2 to EXTI0, EXTI1, EXTI2 lines via SYSCFG */
    /* EXTI0 <- PA0, EXTI1 <- PA1, EXTI2 <- PA2 (port A = 0) */
    write_bits(&SYSCFG_EXTICR1, (0x0FU << 0), (0x00U << 0));   // EXTI0 = Port A
    write_bits(&SYSCFG_EXTICR1, (0x0FU << 4), (0x00U << 4));   // EXTI1 = Port A
    write_bits(&SYSCFG_EXTICR1, (0x0FU << 8), (0x00U << 8));   // EXTI2 = Port A

    /* Step 2: Configure trigger edges (all rising edge) */
    write_bits(&EXTI_RTSR, (1U << 0), (1U << 0));              // EXTI0: rising edge
    write_bits(&EXTI_RTSR, (1U << 1), (1U << 1));              // EXTI1: rising edge
    write_bits(&EXTI_RTSR, (1U << 2), (1U << 2));              // EXTI2: rising edge

    /* Step 3: Enable interrupt requests */
    write_bits(&EXTI_IMR, (1U << 0), (1U << 0));               // Enable EXTI0
    write_bits(&EXTI_IMR, (1U << 1), (1U << 1));               // Enable EXTI1
    write_bits(&EXTI_IMR, (1U << 2), (1U << 2));               // Enable EXTI2

    /* Step 4: Enable EXTI0_1_IRQn in NVIC (Nested Vectored Interrupt Controller) */
    /* IRQ #5 for EXTI0_1 */
    volatile uint32_t* NVIC_ISER0 = (volatile uint32_t*)0xE000E100U;
    *NVIC_ISER0 = (1U << 5);  // Enable IRQ 5 (EXTI0_1_IRQn)
    
    /* Step 5: Enable EXTI2_3_IRQn in NVIC */
    /* IRQ #6 for EXTI2_3 */
    *NVIC_ISER0 = (1U << 6);  // Enable IRQ 6 (EXTI2_3_IRQn)
}

void EXTI_Switch(uint8_t enable)
{
    if (enable & 1)
    {
        write_bits(&EXTI_IMR, (1U << 0), (1U << 0));               // Enable EXTI0
        write_bits(&EXTI_IMR, (1U << 1), (1U << 1));               // Enable EXTI1
        write_bits(&EXTI_IMR, (1U << 2), (1U << 2));               // Enable EXTI2
    }
    else
    {
        write_bits(&EXTI_IMR, (1U << 0), (0U << 0));               // Disable EXTI0
        write_bits(&EXTI_IMR, (1U << 1), (0U << 1));               // Disable EXTI1
        write_bits(&EXTI_IMR, (1U << 2), (0U << 2));               // Disable EXTI2
    }
}