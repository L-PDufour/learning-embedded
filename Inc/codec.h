#ifndef CODEC_H_
#define CODEC_H_

#include "i2c.h"

#define CS43L22_ADDR 0x4A /* 7-bit control-port address */
#define CS43L22_ID_REG 0x01
#define CS43L22_ID 0xE0
#define CS43L22_ID_MASK 0xF8

#define CS43L22_VOLUME 80
#define CS43L22_REG_MASTER_A_VOL 0x20

I2cStatus codec_init(void);
uint8_t codec_volume_reg(void); /* encoded value written to MASTER_x_VOL */

#endif
