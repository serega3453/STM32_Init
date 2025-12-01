void MPU_ReadAccelRaw(uint8_t dev7, uint8_t* buf);
void mpu_wom_enable_pp_high(uint8_t dev7, uint8_t thr, uint8_t lp_odr, uint8_t dlpf_cfg);
int16_t ax_g100(int16_t raw);
void print_accel_g100(int16_t ax, int16_t ay, int16_t az);