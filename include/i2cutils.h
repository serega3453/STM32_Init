#include <stdint.h>

void I2C1_WriteByte(uint8_t dev7, uint8_t reg, uint8_t val);
void I2C1_ReadN(uint8_t dev7, uint8_t reg, uint8_t* buf, uint8_t n);