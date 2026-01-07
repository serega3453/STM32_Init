#include <stdint.h>
#include <regutils.h>
#include <regaddr.h>
#include <mpuutils.h>
#include <i2cutils.h>

uint8_t raw[6];

/* Small helper to print a single byte in hexadecimal over USART1 for diagnostics */
static void usart1_put_hex8(uint8_t v)
{
    const char *hex = "0123456789ABCDEF";
    usart1_putc(hex[(v >> 4) & 0x0F]);
    usart1_putc(hex[v & 0x0F]);
}

/* Current accelerometer full-scale selection (AFS_SEL):
 * 0 -> ±2g, 1 -> ±4g, 2 -> ±8g, 3 -> ±16g
 */
static uint8_t g_afs_sel = 0; /* default ±2g */

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
void mpu_wom_enable_pp_high(uint8_t dev7, uint8_t thr, uint8_t lp_odr, uint8_t dlpf_cfg, uint8_t afs_sel)
{
    uint8_t d;
    usart1_puts("MPU diag: start mpu_wom_enable_pp_high()\r\n");
    usart1_puts("MPU diag: step 0 - soft reset (write PWR_MGMT_1=0x80)\r\n");
    /* 0) Soft reset MPU6050 to clear any leftover state (INT flags, etc.) */
    I2C1_WriteByte(dev7, 0x6B, 0x80);   // PWR_MGMT_1: DEVICE_RESET=1
    usart1_puts("MPU diag: step 0 - write returned\r\n");
    raw_delay(100000);                  // Wait ~15 ms for reset to complete
    
    usart1_puts("MPU diag: step 1 - wake up device (clear sleep)\r\n");
    // 1) Wake up, keep only accelerometer enabled
    I2C1_WriteByte(dev7, 0x6B, 0x00);   // PWR_MGMT_1: SLEEP=0, CYCLE=0
    usart1_puts("MPU diag: step 1 - write PWR_MGMT_1=0x00 returned\r\n");
    usart1_puts("MPU diag: step 1.1 - disable gyros (PWR_MGMT_2=0x07)\r\n");
    I2C1_WriteByte(dev7, 0x6C, 0x07);   // PWR_MGMT_2: GyroXYZ off, Accel on
    usart1_puts("MPU diag: step 1.1 - write returned\r\n");

    usart1_puts("MPU diag: step 1.5 - set AFS_SEL (ACCEL_CONFIG)\r\n");
    /* 1.5) Set accelerometer full-scale range (AFS_SEL) */
    afs_sel &= 0x03;
    I2C1_WriteByte(dev7, 0x1C, (uint8_t)(afs_sel << 3)); /* ACCEL_CONFIG: AFS_SEL bits */
    usart1_puts("MPU diag: step 1.5 - write returned\r\n");
    g_afs_sel = afs_sel;

    usart1_puts("MPU diag: step 2 - wait for accel stabilization (~50ms)\r\n");
    // 2) Wait for accelerometer to stabilize (~40–50 ms at 8 MHz)
    raw_delay(300000);  // ~40–50 ms at 8 MHz
    usart1_puts("MPU diag: step 2 - wait done\r\n");

    usart1_puts("MPU diag: step 3 - set ACCEL_CONFIG2 (DLPF_CFG)\r\n");
    // 3) Set accelerometer DLPF (bandwidth)
    I2C1_WriteByte(dev7, 0x1D, dlpf_cfg);   // ACCEL_CONFIG2: DLPF_CFG
    usart1_puts("MPU diag: step 3 - write returned\r\n");

    usart1_puts("MPU diag: step 4 - enable MOT_DETECT_CTRL (WOM)\r\n");
    // 4) Enable WOM logic and per-axis comparison
    I2C1_WriteByte(dev7, 0x69, 0xC0);   // MOT_DETECT_CTRL
    usart1_puts("MPU diag: step 4 - write returned\r\n");

    usart1_puts("MPU diag: step 5 - set WOM_THR and LP_ACCEL_ODR\r\n");
    // 5) Set WOM threshold (delta accel) and low-power ODR
    I2C1_WriteByte(dev7, 0x1F, thr);    // WOM_THR
    usart1_puts("MPU diag: step 5 - WOM_THR write returned\r\n");
    I2C1_WriteByte(dev7, 0x1E, lp_odr); // LP_ACCEL_ODR
    usart1_puts("MPU diag: step 5 - LP_ACCEL_ODR write returned\r\n");

    /*
     * 6) Configure INT pin behaviour. To avoid a spurious latched INT while
     * configuring the device, temporarily configure INT without latch enabled
     * then clear any pending status. After the low-power reference period we
     * set the desired latch/polarity and enable the Motion INT.
     */
    usart1_puts("MPU diag: step 6 - INT_PIN_CFG temporary (no latch)\r\n");
    /* Temporary INT config: active-high, push-pull, no latch */
    I2C1_WriteByte(dev7, 0x37, 0x10);   // INT_PIN_CFG (temporary)
    usart1_puts("MPU diag: step 6 - write returned\r\n");

    usart1_puts("MPU diag: step 7 - read INT_STATUS to clear pending\r\n");
    /* 7) Clear any pending status immediately to avoid a spurious INT */
    I2C1_ReadN(dev7, 0x3A, &d, 1);      // INT_STATUS (clears pending)
    usart1_puts("MPU diag: step 7 - INT_STATUS="); usart1_put_hex8(d); usart1_puts("\r\n");

    usart1_puts("MPU diag: step 8 - enter low-power cycle mode (PWR_MGMT_1=CYCLE=1)\r\n");
    /* 8) Enter low-power cycle mode (enable WOM but keep INT disabled for now) */
    I2C1_WriteByte(dev7, 0x6B, 0x20);   // PWR_MGMT_1: CYCLE=1
    usart1_puts("MPU diag: step 8 - write returned\r\n");

    usart1_puts("MPU diag: step 9 - wait for WOM reference (~60ms)\r\n");
    /* 9) Pause while WOM establishes reference acceleration (~60 ms) */
    raw_delay(500000);  // ~60 ms, can be slightly longer
    usart1_puts("MPU diag: step 9 - wait done\r\n");

    usart1_puts("MPU diag: step 10 - read INT_STATUS after reference\r\n");
    /* 10) Clear status again after reference established */
    I2C1_ReadN(dev7, 0x3A, &d, 1);
    usart1_puts("MPU diag: step 10 - INT_STATUS="); usart1_put_hex8(d); usart1_puts("\r\n");

    usart1_puts("MPU diag: step 11 - set INT_PIN_CFG final (latch)\r\n");
    /* 11) Now program final INT_PIN behaviour (active-high, push-pull, latch) */
    I2C1_WriteByte(dev7, 0x37, 0x30);   // INT_PIN_CFG (final)
    usart1_puts("MPU diag: step 11 - write returned\r\n");

    usart1_puts("MPU diag: step 12 - enable Motion INT (INT_ENABLE bit6)\r\n");
    /* 12) Finally enable Motion INT (bit 6) */
    I2C1_WriteByte(dev7, 0x38, 0x40);   // INT_ENABLE (bit6)
    usart1_puts("MPU diag: step 12 - write returned\r\n");

    usart1_puts("MPU diag: finished mpu_wom_enable_pp_high()\r\n");
    usart1_puts("MPU6050 WOM enabled\r\n");
}

/**
 * Read accelerometer values in hundredths of g.
 * Assumes full-scale ±2g (LSB = 16384 per g).
 */

int16_t ax_g100(int16_t raw)
{
    /* Convert raw accelerometer value to hundredths of g (g*100).
     * Use current full-scale selection (g_afs_sel) to choose LSB per g:
     * LSB_per_g = 16384 >> g_afs_sel  (0:16384, 1:8192, 2:4096, 3:2048)
     */
    uint32_t lsb_per_g = 16384UL >> (g_afs_sel & 0x03);
    return (int32_t)raw * 100 / (int32_t)lsb_per_g;
}

void print_accel_g100(int16_t ax, int16_t ay, int16_t az)
{
    usart1_puts("AX="); usart1_put_i16(ax_g100(ax)); usart1_puts(" ");
    usart1_puts("AY="); usart1_put_i16(ax_g100(ay)); usart1_puts(" ");
    usart1_puts("AZ="); usart1_put_i16(ax_g100(az)); usart1_puts("\r\n");
}

void clear_mpu_int(void)
{
    uint8_t d;
    I2C1_ReadN(MPU_ADDR, 0x3A, &d, 1);      // INT_STATUS (clears pending)
}