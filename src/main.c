#include <stdint.h>
#include <regutils.h>

#define vol(addr) (*(volatile uint32_t*)(addr))

#define RCC_BASE 0x40021000U
#define GPIOA_BASE 0x48000000U
#define GPIOB_BASE 0x48000400U
#define GPIOF_BASE 0x48001400U
#define TIM3_BASE 0x40000400U
#define I2C1_BASE 0x40005400U

#define RCC_AHBENR vol(RCC_BASE + 0x14U)
#define RCC_APB1ENR vol(RCC_BASE + 0x1CU)

//#define GPIOA_MODER ((volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_MODER vol(GPIOA_BASE + 0x00U)
#define GPIOA_OTYPER vol(GPIOA_BASE + 0x04U)
#define GPIOA_OSPEEDR vol(GPIOA_BASE + 0x08U)
#define GPIOA_PUPDR vol(GPIOA_BASE + 0x0CU)
#define GPIOA_AFRL vol(GPIOA_BASE + 0x20U)

#define GPIOB_MODER vol(GPIOB_BASE + 0x00U)
#define GPIOB_OTYPER vol(GPIOB_BASE + 0x04U)
#define GPIOB_OSPEEDR vol(GPIOB_BASE + 0x08U)
#define GPIOB_PUPDR vol(GPIOB_BASE + 0x0CU)
#define GPIOB_AFRL vol(GPIOB_BASE + 0x20U)

#define GPIOF_MODER vol(GPIOF_BASE + 0x00U)
#define GPIOF_OTYPER vol(GPIOF_BASE + 0x04U)
#define GPIOF_OSPEEDR vol(GPIOF_BASE + 0x08U)
#define GPIOF_PUPDR vol(GPIOF_BASE + 0x0CU)
#define GPIOF_AFRL vol(GPIOF_BASE + 0x20U)

#define TIM3_CR1 vol(TIM3_BASE + 0x00U)
#define TIM3_EGR vol(TIM3_BASE + 0x14U)
#define TIM3_CCMR1 vol(TIM3_BASE + 0x18U)
#define TIM3_CCMR2 vol(TIM3_BASE + 0x1CU)
#define TIM3_CCER vol(TIM3_BASE + 0x20U)
#define TIM3_CNT vol(TIM3_BASE + 0x24U)
#define TIM3_PSC vol(TIM3_BASE + 0x28U)
#define TIM3_ARR vol(TIM3_BASE + 0x2CU)
#define TIM3_CCR1 vol(TIM3_BASE + 0x34U)
#define TIM3_CCR2 vol(TIM3_BASE + 0x38U)
#define TIM3_CCR4 vol(TIM3_BASE + 0x40U)

#define I2C1_CR1 vol(I2C1_BASE + 0x00U)

unsigned char flag = 0b00000000;
uint32_t color = 0x08U;

void RCC_EnableClock()
{
    write_bits(&RCC_APB1ENR, (0x01U << 1), (0x01U << 1)); //TIM3_EN
    write_bits(&RCC_APB1ENR, (0x01U << 21), (0x01U << 21)); //I2C1_EN
    raw_delay(10000);
    write_bits(&RCC_AHBENR, (0x03U << 17), 0x03U << 17); //GPIOA_EN && GPIOB_EN
    write_bits(&RCC_AHBENR, (0x01U << 22), (0x01U << 22)); //GPIOF_EN

    raw_delay(10000);

}

void GPIOA_Config()
{
    write_bits(&GPIOA_MODER, (0x0FU << 12), (0x0AU << 12)); //PA6_AF && PA7_AF
    write_bits(&GPIOA_OTYPER, (0x03U << 6), (0x00U << 6)); //PA6_PP && PA7_PP
    write_bits(&GPIOA_OSPEEDR, (0x0FU << 12), (0x0FU << 12)); //PA6_HS && PA7_HS
    write_bits(&GPIOA_PUPDR, (0x0FU << 12), (0x00U << 12)); //PA6_PUNPD && PA7_PUNPD
    write_bits(&GPIOA_AFRL, (0xFFU << 24), (0x11U << 24)); //PA6_AF1 && PA7_AF1
}

void GPIOB_Config()
{
    write_bits(&GPIOB_MODER, (0x03U << 2), (0x02U << 2));
    write_bits(&GPIOB_OTYPER, (0x01U << 1), (0x00U << 1));
    write_bits(&GPIOB_OSPEEDR, (0x03U << 2), (0x03U << 2));
    write_bits(&GPIOB_PUPDR, (0x03U << 2), (0x00U << 2));
    write_bits(&GPIOB_AFRL, (0x0FU << 4), (0x01U << 4));
}

void GPIOF_Config()
{
    write_bits(&GPIOF_MODER, (0x0FU << 0), (0x0AU << 0));
    write_bits(&GPIOF_OTYPER, (0x03U << 0), (0x03U << 0));
    write_bits(&GPIOF_OSPEEDR, (0x0FU << 0), (0x0FU << 0));
    write_bits(&GPIOB_PUPDR, (0x0FU << 0), (0xA0U << 0));
    write_bits(&GPIOB_AFRL, (0xFFU << 0), (0x11U << 0));
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

I2C1_Config()
{
    write_bits(I2C1_CR1, (0x01U << 0), (0x00U << 0));
}

uint32_t Next_Color(uint32_t* col)
{
    raw_delay(500);
    (*col)++;

    return (*col);
}

int main(void)
{
    RCC_EnableClock();

    GPIOA_Config();
    GPIOB_Config();

    TIM3_Config();

    flag = 0b00000001;

    for(;;)
    {
        if (flag >> 0 & 0x01)
        {
            if (color >= 750)
            {
                color = 0;
                flag = 0b00000010;
            }
            write_reg(&TIM3_CCR1, color);
        }

        if (flag >> 1 & 0x01)
        {
            if (color >= 750)
            {
                color = 0;
                flag = 0b00000100;
            }
            write_reg(&TIM3_CCR2, color);
        }

        if (flag >> 2 & 0x01)
        {
            if (color >= 750)
            {
                color = 0;
                flag = 0b00000001;
            }
            write_reg(&TIM3_CCR4, color);
        }

        color = Next_Color(&color);
    }
}