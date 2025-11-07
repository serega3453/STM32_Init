#include <regutils.h>
#include <regaddr.h>

void RCC_EnableClock()
{
    write_bits(&RCC_APB1ENR, (0x01U << 1), (0x01U << 1)); //TIM3_EN
    write_bits(&RCC_APB1ENR, (0x01U << 21), (0x01U << 21)); //I2C1_EN
    write_bits(&RCC_APB1ENR, (1U << 17), (1U << 17));  //USART2_EN
    write_bits(&RCC_APB2ENR, (0x01U << 0), (0x01U << 0)); //SYSCFG_EN
    raw_delay(10000);
    write_bits(&RCC_AHBENR, (0x03U << 17), 0x03U << 17); //GPIOA_EN && GPIOB_EN
    write_bits(&RCC_AHBENR, (0x01U << 22), (0x01U << 22)); //GPIOF_EN

    raw_delay(10000);
}

void GPIOA_Config()
{
    //GPIOA PA6, PA7 - LED PWM
    write_bits(&GPIOA_MODER, (0x0FU << 12), (0x0AU << 12)); //PA6_AF && PA7_AF
    write_bits(&GPIOA_OTYPER, (0x03U << 6), (0x00U << 6)); //PA6_PP && PA7_PP
    write_bits(&GPIOA_OSPEEDR, (0x0FU << 12), (0x0FU << 12)); //PA6_HS && PA7_HS
    write_bits(&GPIOA_PUPDR, (0x0FU << 12), (0x00U << 12)); //PA6_PUNPD && PA7_PUNPD
    write_bits(&GPIOA_AFRL, (0xFFU << 24), (0x11U << 24)); //PA6_AF1 && PA7_AF1

    //GPIOA PA2, PA3 - USART2
    write_bits(&GPIOA_MODER, (0x0FU << 4), (0x0AU << 4)); //PA2_AF && PA3_AF
    write_bits(&GPIOA_OTYPER, (0x03U << 2), (0x00U << 2)); //PA2_PP && PA3_PP
    write_bits(&GPIOA_OSPEEDR, (0x0FU << 4), (0x0FU << 4)); //PA2_HS && PA3_HS
    write_bits(&GPIOA_PUPDR, (0x0FU << 4), (0x04U << 4)); //PA2_NPUNPD && PA3_PU
    write_bits(&GPIOA_AFRL, (0xFFU << 8), (0x11U << 8)); //PA2_AF1 && PA3_AF1

    //GPIOA PA1 - Input (INT from MPU)
    write_bits(&GPIOA_MODER, (0x03U << 2), (0x00U << 2)); //PA1_IN
    write_bits(&GPIOA_OTYPER, (0x03U << 2), (0x00U << 2)); //PA1_PP
    write_bits(&GPIOA_OSPEEDR, (0x0FU << 4), (0x0FU << 4)); //PA1_HS
    write_bits(&GPIOA_PUPDR, (0x03U << 2), (0x02U << 2)); //PA1_PD
}

void GPIOB_Config()
{
    ///GPIOB PB1 - LED PWM
    write_bits(&GPIOB_MODER, (0x03U << 2), (0x02U << 2));
    write_bits(&GPIOB_OTYPER, (0x01U << 1), (0x00U << 1));
    write_bits(&GPIOB_OSPEEDR, (0x03U << 2), (0x03U << 2));
    write_bits(&GPIOB_PUPDR, (0x03U << 2), (0x00U << 2));
    write_bits(&GPIOB_AFRL, (0x0FU << 4), (0x01U << 4));
}

void GPIOF_Config()
{
    //GPIOF PF0, PF1 - I2C1
    write_bits(&GPIOF_MODER, (0x0FU << 0), (0x0AU << 0));
    write_bits(&GPIOF_OTYPER, (0x03U << 0), (0x03U << 0));
    write_bits(&GPIOF_OSPEEDR, (0x0FU << 0), (0x0FU << 0));
    write_bits(&GPIOF_PUPDR, (0x0FU << 0), (0x00U << 0));
    write_bits(&GPIOF_AFRL, (0xFFU << 0), (0x11U << 0));
}

void TIM3_Config()
{
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

void I2C1_Config()
{
    write_bits(&I2C1_CR1, (0x01U << 0), (0x00U << 0));
    write_reg(&I2C1_TIMINGR, (0x00201D2B));
    write_bits(&I2C1_CR1, (0x01U << 0), (0x01U << 0));
}

void USART2_Config()
{
    write_bits(&USART2_CR1, (1U<<0), 0);
    write_reg(&USART2_BRR, 0x0045);
    write_bits(&USART2_CR2, (3U<<12), 0);
    write_bits(&USART2_CR1, (1U<<3), (1U<<3));
    write_bits(&USART2_CR1, (1U<<2), (1U<<2));
    write_bits(&USART2_CR1, (1U<<0), (1U<<0));
}