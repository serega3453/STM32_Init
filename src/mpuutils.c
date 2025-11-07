#include <stdint.h>
#include <regutils.h>
#include <regaddr.h>
#include <mpuutils.h>
#include <i2cutils.h>

#define MPU_ADDR 0x68

uint8_t raw[6];

void MPU_ReadAccelRaw(uint8_t dev7, uint8_t* buf)
{
        I2C1_ReadN(MPU_ADDR, 0x3B, raw, 6);

        int16_t ax = (int16_t)((raw[0]<<8) | raw[1]);
        int16_t ay = (int16_t)((raw[2]<<8) | raw[3]);
        int16_t az = (int16_t)((raw[4]<<8) | raw[5]);

        print_accel_g100(ax, ay, az);
}

void mpu_wom_enable_pp_high(uint8_t dev7, uint8_t thr, uint8_t lp_odr)
{
    uint8_t d;

    // 1) проснуться, оставить только аксель
    I2C1_WriteByte(dev7, 0x6B, 0x00);   // PWR_MGMT_1: SLEEP=0, CYCLE=0
    I2C1_WriteByte(dev7, 0x6C, 0x07);   // PWR_MGMT_2: GyroXYZ off, Accel on

    // 2) подождать, чтобы аксель успел стабилизироваться
    raw_delay(300000);  // ~40–50 мс на 8 МГц

    // 3) полоса акселя ≈184 Гц
    I2C1_WriteByte(dev7, 0x1D, 0x01);   // ACCEL_CONFIG2

    // 4) WOM: включить интеллект и сравнение по осям
    I2C1_WriteByte(dev7, 0x69, 0xC0);   // MOT_DETECT_CTRL

    // 5) задать порог (дельта ускорения) и частоту low-power режима
    I2C1_WriteByte(dev7, 0x1F, thr);    // WOM_THR
    I2C1_WriteByte(dev7, 0x1E, lp_odr); // LP_ACCEL_ODR

    // 6) настроить вывод INT — активный высокий, push-pull
    I2C1_WriteByte(dev7, 0x37, 0x30);   // INT_PIN_CFG

    // 7) сбросить старый статус, чтобы не словить мусорный INT
    I2C1_ReadN(dev7, 0x3A, &d, 1);      // INT_STATUS

    // 8) перевести в low-power (включить WOM, но пока без INT)
    I2C1_WriteByte(dev7, 0x6B, 0x20);   // PWR_MGMT_1: CYCLE=1

    // 9) пауза — WOM берёт эталон ускорения
    raw_delay(500000);  // ~60 мс, можно чуть больше

    // 10) ещё раз очистить статус
    I2C1_ReadN(dev7, 0x3A, &d, 1);

    // 11) только теперь разрешить Motion INT
    I2C1_WriteByte(dev7, 0x38, 0x40);   // INT_ENABLE (бит6)
}

int16_t ax_g100(int16_t raw) 
{ 
    return (int32_t)raw * 100 / 16384;
}

void print_accel_g100(int16_t ax, int16_t ay, int16_t az)
{
    usart1_puts("AX="); usart1_put_i16(ax_g100(ax)); usart1_puts(" ");
    usart1_puts("AY="); usart1_put_i16(ax_g100(ay)); usart1_puts(" ");
    usart1_puts("AZ="); usart1_put_i16(ax_g100(az)); usart1_puts("\r\n");
}