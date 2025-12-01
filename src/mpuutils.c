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

/**
 * Enable MPU6050 WOM (Wake-On-Motion) with configurable DLPF.
 * dev7     - 7-bit I2C address
 * thr      - WOM threshold (0x00-0xFF, higher = less sensitive)
 * lp_odr   - Low-power ODR (0x00-0x0F, lower = more power saving)
 * dlpf_cfg - DLPF config (0x00-0x07, higher = more filtering)
 *            0x01: 184 Hz (original)
 *            0x06: 5 Hz (strong filtering)
 *            0x07: Hold (maximum filtering)
 */
void mpu_wom_enable_pp_high(uint8_t dev7, uint8_t thr, uint8_t lp_odr, uint8_t dlpf_cfg)
{
    uint8_t d;

    // 1) Wake up, keep only accelerometer enabled
    I2C1_WriteByte(dev7, 0x6B, 0x00);   // PWR_MGMT_1: SLEEP=0, CYCLE=0
    I2C1_WriteByte(dev7, 0x6C, 0x07);   // PWR_MGMT_2: GyroXYZ off, Accel on

    // 2) Wait for accelerometer to stabilize (~40–50 ms at 8 MHz)
    raw_delay(300000);  // ~40–50 ms at 8 MHz

    // 3) Set accelerometer DLPF (bandwidth)
    I2C1_WriteByte(dev7, 0x1D, dlpf_cfg);   // ACCEL_CONFIG2: DLPF_CFG

    // 4) Enable WOM logic and per-axis comparison
    I2C1_WriteByte(dev7, 0x69, 0xC0);   // MOT_DETECT_CTRL

    // 5) Set WOM threshold (delta accel) and low-power ODR
    I2C1_WriteByte(dev7, 0x1F, thr);    // WOM_THR
    I2C1_WriteByte(dev7, 0x1E, lp_odr); // LP_ACCEL_ODR

    // 6) Configure INT pin: active-high, push-pull
    I2C1_WriteByte(dev7, 0x37, 0x30);   // INT_PIN_CFG

    // 7) Clear any pending status to avoid a spurious INT
    I2C1_ReadN(dev7, 0x3A, &d, 1);      // INT_STATUS

    // 8) Enter low-power cycle mode (enable WOM but keep INT disabled for now)
    I2C1_WriteByte(dev7, 0x6B, 0x20);   // PWR_MGMT_1: CYCLE=1

    // 9) Pause while WOM establishes reference acceleration (~60 ms)
    raw_delay(500000);  // ~60 ms, can be slightly longer

    // 10) Clear status again
    I2C1_ReadN(dev7, 0x3A, &d, 1);

    // 11) Finally enable Motion INT (bit 6)
    I2C1_WriteByte(dev7, 0x38, 0x40);   // INT_ENABLE (bit6)
}

/**
 * Read accelerometer values in hundredths of g.
 * Assumes full-scale ±2g (LSB = 16384 per g).
 */

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