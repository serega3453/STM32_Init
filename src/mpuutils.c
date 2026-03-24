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
    uint8_t err;

    uint8_t pwr;

pwr = 0xFF;
if (I2C1_ReadN(dev7, 0x6B, &pwr, 1)) {
    usart1_puts("read PWR_MGMT_1 failed\r\n");
    return 0;
}
usart1_puts("PWR_MGMT_1 before=0x");
usart1_put_hex8(pwr);
usart1_puts("\r\n");

    usart1_puts("MPU preconfig: start\r\n");
    raw_delay(150000);

    /* read WHO_AM_I */
    usart1_puts("MPU preconfig: read WHO_AM_I\r\n");
    who = 0x00;
if (I2C1_ReadN(dev7, 0x75, &who, 1)) {
    usart1_puts("MPU preconfig: WHO_AM_I read failed\r\n");
    return 0;
}
    usart1_puts("MPU preconfig: WHO_AM_I=0x");
    usart1_put_hex8(who);
    usart1_puts("\r\n");

    if (who == 0x00) {
        usart1_puts("MPU preconfig: WHO_AM_I invalid\r\n");
        return 0;
    }

    /* wake up */
    err = I2C1_WriteByte(dev7, 0x6B, 0x00);
    if (err) {
        usart1_puts("MPU preconfig: PWR_MGMT_1 write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    err = I2C1_WriteByte(dev7, 0x6C, 0x07);
    if (err) {
        usart1_puts("MPU preconfig: PWR_MGMT_2 write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    err = I2C1_WriteByte(dev7, 0x1C, (afs_sel << 3));
    if (err) {
        usart1_puts("MPU preconfig: ACCEL_CONFIG write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }
    g_afs_sel = afs_sel;

    err = I2C1_WriteByte(dev7, 0x1D, dlpf_cfg);
    if (err) {
        usart1_puts("MPU preconfig: ACCEL_CONFIG2 write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    err = I2C1_WriteByte(dev7, 0x1E, 0x00);
    if (err) {
        usart1_puts("MPU preconfig: LP_ACCEL_ODR write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    err = I2C1_WriteByte(dev7, 0x69, 0x00);
    if (err) {
        usart1_puts("MPU preconfig: MOT_DETECT_CTRL write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    err = I2C1_WriteByte(dev7, 0x1F, 0x00);
    if (err) {
        usart1_puts("MPU preconfig: WOM_THR write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    /* read back */
    tmp = 0xFF;
    I2C1_ReadN(dev7, 0x1C, &tmp, 1);
    usart1_puts("MPU preconfig: ACCEL_CONFIG=0x");
    usart1_put_hex8(tmp);
    usart1_puts("\r\n");

    tmp = 0xFF;
    I2C1_ReadN(dev7, 0x1D, &tmp, 1);
    usart1_puts("MPU preconfig: ACCEL_CONFIG2=0x");
    usart1_put_hex8(tmp);
    usart1_puts("\r\n");

    err = I2C1_WriteByte(dev7, 0x37, 0x10);
    if (err) {
        usart1_puts("MPU preconfig: INT_PIN_CFG write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    err = I2C1_WriteByte(dev7, 0x38, 0x01);
    if (err) {
        usart1_puts("MPU preconfig: INT_ENABLE write failed, err=0x");
        usart1_put_hex8(err);
        usart1_puts("\r\n");
        return 0;
    }

    raw_delay(5000);
    I2C1_ReadN(dev7, 0x3A, &tmp, 1);

    usart1_puts("MPU preconfig: done\r\n");
    return 1;
}

int mpu_detect_impact(uint8_t dev7, uint16_t sensitivity)
{
    static int16_t prev_ax = 0, prev_ay = 0, prev_az = 0;
    static uint8_t initialized = 0;
    uint8_t buf[6];
    int16_t ax, ay, az;

    /* Read raw accel registers */
    I2C1_ReadN(dev7, 0x3B, buf, 6);
    ax = (int16_t)((buf[0] << 8) | buf[1]);
    ay = (int16_t)((buf[2] << 8) | buf[3]);
    az = (int16_t)((buf[4] << 8) | buf[5]);

    if (!initialized) {
        prev_ax = ax; prev_ay = ay; prev_az = az;
        initialized = 1;
        return 0; /* No previous sample to compare with */
    }

    int32_t dx = (int32_t)ax - (int32_t)prev_ax;
    int32_t dy = (int32_t)ay - (int32_t)prev_ay;
    int32_t dz = (int32_t)az - (int32_t)prev_az;

    int64_t mag2 = (int64_t)dx * dx + (int64_t)dy * dy + (int64_t)dz * dz;
    int64_t thr = (int64_t)sensitivity * (int64_t)sensitivity;

    /* update previous sample for next call */
    prev_ax = ax; prev_ay = ay; prev_az = az;

    if (mag2 > thr) {
        return 1;
    }
    return 0;
}