#define MPU_ADDR 0x68

/**
 * MPU6050 accelerometer helper functions.
 * Provides initialization, raw data reading, and basic motion detection (WOM) setup.
 */

/**
 * Read raw accelerometer values from MPU6050 (I2C address 0x68).
 * Fetches 6 bytes (AX_H, AX_L, AY_H, AY_L, AZ_H, AZ_L) and prints as g*100.
 * dev7 - 7-bit I2C address (for compatibility; always uses hardcoded MPU_ADDR=0x68)
 * buf  - unused (reserved for future API changes)
 */
void MPU_ReadAccelRaw(uint8_t dev7, uint8_t* buf);

/**
 * Initialize MPU6050 with Wake-On-Motion (WOM) capability.
 * Configures accelerometer, digital low-pass filter, low-power ODR, and motion INT.
 * Must be called once during startup before using motion detection.
 * 
 * dev7     - 7-bit I2C address
 * thr      - WOM threshold (0x00=most sensitive, 0xFF=least sensitive)
 * lp_odr   - Low-power acceleration output data rate (0x00..0x0F)
 * dlpf_cfg - Digital low-pass filter config (0x00..0x07; higher=more filtering)
 * afs_sel  - Accelerometer full-scale selection (0=±2g, 1=±4g, 2=±8g, 3=±16g)
 */
void mpu_wom_enable_pp_high(uint8_t dev7, uint8_t thr, uint8_t lp_odr, uint8_t dlpf_cfg, uint8_t afs_sel);

/* Preliminary MPU configuration: soft-reset, wake, set accel FS and DLPF.
 * Returns 1 on success (device responded and registers written), 0 on failure.
 */
int mpu_preconfigure(uint8_t dev7, uint8_t afs_sel, uint8_t dlpf_cfg);

/**
 * Convert raw accelerometer value to hundredths of g (g*100).
 * Automatically uses the current full-scale setting (set by mpu_wom_enable_pp_high).
 * raw - raw ADC value from accelerometer
 * return - acceleration in units of 0.01g (e.g., 100 = 1.00g)
 */
int16_t ax_g100(int16_t raw);

/**
 * Print accelerometer values over USART1 in human-readable format.
 * Output format: "AX=<val> AY=<val> AZ=<val>\r\n"
 * Values are in hundredths of g (g*100).
 * ax, ay, az - acceleration values in units of g*100 (typically from ax_g100 conversion)
 */
void print_accel_g100(int16_t ax, int16_t ay, int16_t az);

void clear_mpu_int(void);