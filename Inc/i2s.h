#ifndef I2S_H_
#define I2S_H_

#include "engine.h"
#include "i2c.h"

extern volatile uint32_t i2s_ht_count;
extern volatile uint32_t i2s_tc_count;

I2cStatus i2s_clock_init(void);
I2cStatus i2s3_init(void);
void i2s_dma_config(sample_t *buf, uint32_t len);
void i2s_dma_start(void);
#endif
