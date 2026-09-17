#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

typedef enum {
  I2C_OK = 0,      /* must be 0 so `if (status)` means failure */
  I2C_ERR_TIMEOUT, /* 1: a flag never arrived in time */
  I2C_ERR_NACK     /* 2: address byte was not acknowledged */
} I2cStatus;

void i2c1_init(void);
I2cStatus i2c1_byte_read(uint8_t saddr, uint8_t maddr, uint8_t *data);
I2cStatus i2c1_burst_read(uint8_t saddr, uint8_t maddr, int n, uint8_t *data);
I2cStatus i2c1_burst_write(uint8_t saddr, uint8_t maddr, int n,
                           const uint8_t *data);

I2cStatus i2c1_byte_write(uint8_t saddr, uint8_t maddr, uint8_t data);
#endif
