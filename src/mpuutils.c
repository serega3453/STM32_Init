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
    /* Clean, robust WOM implementation that raises INT high on real impact.
     * Strategy:
     *  - Soft-reset and wake device
     *  - Configure accel range, DLPF and LP ODR
     *  - Enable MOT_DETECT_CTRL (per-axis compare)
     *  - Temporarily configure INT as non-latching and clear status
     *  - Enter cycle mode to let WOM form reference
     *  - Sample a small number of cycle-mode accel readings to estimate noise
     *  - Choose WOM_THR (use user thr if non-zero, otherwise derived from noise)
     *  - Program WOM_THR, enable latched INT and Motion INT
     *  - If INT_STATUS is immediately set, increase threshold and retry a few times
     */
    uint8_t d = 0;
    usart1_puts("WOM: start\r\n");

    /* soft reset */
    I2C1_WriteByte(dev7, 0x6B, 0x80);
    raw_delay(150000);

    /* wake */
    I2C1_WriteByte(dev7, 0x6B, 0x00);
    raw_delay(20000);
    I2C1_WriteByte(dev7, 0x6C, 0x07); /* disable gyros */

    /* set accel FS and DLPF */
    afs_sel &= 0x03;
    I2C1_WriteByte(dev7, 0x1C, (uint8_t)(afs_sel << 3));
    g_afs_sel = afs_sel;
    I2C1_WriteByte(dev7, 0x1D, dlpf_cfg);

    /* program LP_ACCEL_ODR (used in cycle mode) */
    I2C1_WriteByte(dev7, 0x1E, lp_odr);

    /* enable WOM per-axis compare */
    I2C1_WriteByte(dev7, 0x69, 0xC0);

    /* temporarily set INT non-latching so reads clear any spurious line */
    I2C1_WriteByte(dev7, 0x37, 0x10);
    raw_delay(5000);

    /* clear any pending INT */
    I2C1_ReadN(dev7, 0x3A, &d, 1);
    usart1_puts("WOM: pre-ref INT_STATUS=0x"); usart1_put_hex8(d); usart1_puts("\r\n");

    /* enter cycle mode so WOM reference can form */
    I2C1_WriteByte(dev7, 0x6B, 0x20); /* CYCLE=1 */
    raw_delay(50000);
    raw_delay(500000); /* ~60 ms for reference */

    /* sample small number of cycle-mode accel readings to estimate noise */
    {
        const uint8_t M = 12;
        int32_t sx = 0, sy = 0, sz = 0;
        int16_t sx_samples[M];
        int16_t sy_samples[M];
        int16_t sz_samples[M];
        uint8_t buf[6];
        for (uint8_t i = 0; i < M; i++) {
            I2C1_ReadN(dev7, 0x3B, buf, 6);
            int16_t rx = (int16_t)((buf[0] << 8) | buf[1]);
            int16_t ry = (int16_t)((buf[2] << 8) | buf[3]);
            int16_t rz = (int16_t)((buf[4] << 8) | buf[5]);
            sx_samples[i] = rx; sy_samples[i] = ry; sz_samples[i] = rz;
            sx += rx; sy += ry; sz += rz;
            raw_delay(20000);
        }
        int16_t mx = (int16_t)(sx / M);
        int16_t my = (int16_t)(sy / M);
        int16_t mz = (int16_t)(sz / M);

        /* mean absolute deviation per axis */
        int32_t axdev = 0, aydev = 0, azdev = 0;
        for (uint8_t i = 0; i < M; i++) {
            int32_t v;
            v = sx_samples[i] - mx; if (v < 0) v = -v; axdev += v;
            v = sy_samples[i] - my; if (v < 0) v = -v; aydev += v;
            v = sz_samples[i] - mz; if (v < 0) v = -v; azdev += v;
        }
        int32_t axr = axdev / M;
        int32_t ayr = aydev / M;
        int32_t azr = azdev / M;
        int32_t noise = axr; if (ayr > noise) noise = ayr; if (azr > noise) noise = azr;

        usart1_puts("WOM: cycle MAD AX="); usart1_put_i16((int16_t)axr); usart1_puts(" AY="); usart1_put_i16((int16_t)ayr); usart1_puts(" AZ="); usart1_put_i16((int16_t)azr); usart1_puts("\r\n");

        /* choose threshold: use user-supplied if non-zero, otherwise noise * factor */
        uint8_t chosen = 0;
        if (thr != 0) {
            chosen = thr;
        } else {
            int32_t calc = noise * 3; /* safety factor */
            if (calc < 8) calc = 8;
            if (calc > 0xFF) calc = 0xFF;
            chosen = (uint8_t)calc;
        }

        usart1_puts("WOM: chosen WOM_THR=0x"); usart1_put_hex8(chosen); usart1_puts("\r\n");

        /* program threshold and enable INT (with retries if INT_STATUS is already set) */
        uint8_t attempts = 0;
        for (attempts = 0; attempts < 4; attempts++) {
            I2C1_WriteByte(dev7, 0x1F, chosen); /* WOM_THR */
            raw_delay(5000);

            /* enable latched INT now that threshold is programmed */
            I2C1_WriteByte(dev7, 0x37, 0x30); /* latch, active-high */
            raw_delay(2000);

            I2C1_WriteByte(dev7, 0x38, 0x40); /* INT_ENABLE bit6 (Motion) */
            raw_delay(5000);

            I2C1_ReadN(dev7, 0x3A, &d, 1);
            usart1_puts("WOM: INT_STATUS after enable=0x"); usart1_put_hex8(d); usart1_puts("\r\n");
            if (d == 0) {
                usart1_puts("WOM: enabled OK\r\n");
                break;
            }

            /* if INT stuck, increase threshold and retry */
            usart1_puts("WOM: INT stuck, increasing threshold\r\n");
            uint16_t nt = chosen;
            nt = nt + (nt / 2); /* increase by 1.5x */
            if (nt > 0xFF) nt = 0xFF;
            if ((uint8_t)nt == chosen) break;
            chosen = (uint8_t)nt;
            /* temporarily disable latch so we can clear and retry cleanly */
            I2C1_WriteByte(dev7, 0x37, 0x10);
            raw_delay(2000);
            I2C1_ReadN(dev7, 0x3A, &d, 1);
            raw_delay(2000);
        }

        if (attempts >= 4) usart1_puts("WOM: warning - INT_STATUS remained set after retries\r\n");
    }

    usart1_puts("WOM: done\r\n");
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

int mpu_preconfigure(uint8_t dev7, uint8_t afs_sel, uint8_t dlpf_cfg)
{
    uint8_t who;
    uint8_t tmp;

    usart1_puts("MPU preconfig: start\r\n");

    /* soft reset */
    usart1_puts("MPU preconfig: soft reset (PWR_MGMT_1=0x80)\r\n");
    I2C1_WriteByte(dev7, 0x6B, 0x80);
    raw_delay(150000);

    /* read WHO_AM_I */
    usart1_puts("MPU preconfig: read WHO_AM_I\r\n");
    I2C1_ReadN(dev7, 0x75, &who, 1);
    usart1_puts("MPU preconfig: WHO_AM_I=0x"); usart1_put_hex8(who); usart1_puts("\r\n");
    if (who == 0x00) {
        usart1_puts("MPU preconfig: WHO_AM_I==0x00 -> device not responding\r\n");
        return 0;
    }

    /* wake up */
    usart1_puts("MPU preconfig: wake up (PWR_MGMT_1=0x00)\r\n");
    I2C1_WriteByte(dev7, 0x6B, 0x00);
    usart1_puts("MPU preconfig: set PWR_MGMT_2 (disable gyros)\r\n");
    I2C1_WriteByte(dev7, 0x6C, 0x07);

    /* Configure as requested by user:
     * - Enable accelerometers (AFS_SEL=0 -> ±2g, maximum resolution)
     * - Disable gyros (already set PWR_MGMT_2=0x07)
     * - Set ACCEL_CONFIG2 DLPF=0 for maximum sample rate / bandwidth
     * - Set LP_ACCEL_ODR=0 (max)
     * - Disable WOM/Motion detect here (we'll not configure WOM)
     * - Configure INT to be DATA_READY-driven (INT_ENABLE bit0)
     */
    usart1_puts("MPU preconfig: set ACCEL_CONFIG AFS_SEL=0 (±2g, max res)\r\n");
    I2C1_WriteByte(dev7, 0x1C, 0x00); /* AFS_SEL=0 */
    g_afs_sel = 0;

    usart1_puts("MPU preconfig: set ACCEL_CONFIG2 DLPF=0 (max rate)\r\n");
    I2C1_WriteByte(dev7, 0x1D, 0x00); /* DLPF_CFG=0 */

    usart1_puts("MPU preconfig: set LP_ACCEL_ODR=0 (max)\r\n");
    I2C1_WriteByte(dev7, 0x1E, 0x00);

    /* Ensure WOM/Motion detect disabled */
    I2C1_WriteByte(dev7, 0x69, 0x00); /* MOT_DETECT_CTRL = 0 */
    I2C1_WriteByte(dev7, 0x1F, 0x00); /* WOM_THR = 0 */

    /* read back registers for confirmation */
    I2C1_ReadN(dev7, 0x1C, &tmp, 1);
    usart1_puts("MPU preconfig: ACCEL_CONFIG=0x"); usart1_put_hex8(tmp); usart1_puts("\r\n");
    I2C1_ReadN(dev7, 0x1D, &tmp, 1);
    usart1_puts("MPU preconfig: ACCEL_CONFIG2=0x"); usart1_put_hex8(tmp); usart1_puts("\r\n");

    /* Configure INT pin and enable DATA_READY interrupt only */
    I2C1_WriteByte(dev7, 0x37, 0x10); /* INT_PIN_CFG: non-latch, active-high, push-pull */
    I2C1_WriteByte(dev7, 0x38, 0x01); /* INT_ENABLE: DATA_READY (bit0) */
    raw_delay(5000);
    I2C1_ReadN(dev7, 0x3A, &tmp, 1);  /* clear any pending INT_STATUS */
    usart1_puts("MPU preconfig: INT cleared, INT_ENABLE=0x01 (DATA_READY)\r\n");

    usart1_puts("MPU preconfig: done\r\n");
    return 1;
}