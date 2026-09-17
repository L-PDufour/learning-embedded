#include "codec.h"

#define REG_POWER_CTL1 0x02
#define REG_POWER_CTL2 0x04
#define REG_CLOCKING_CTL 0x05
#define REG_INTERFACE_CTL1 0x06
#define REG_ANALOG_ZC_SETT 0x0A
#define REG_MISC_CTL 0x0E
#define REG_PLAYBACK_CTL2 0x0F
#define REG_PCMA_VOL 0x1A
#define REG_PCMB_VOL 0x1B
#define REG_TONE_CTL 0x1F
#define REG_MASTER_A_VOL 0x20
#define REG_MASTER_B_VOL 0x21
#define REG_SPEAKER_A_VOL 0x24
#define REG_SPEAKER_B_VOL 0x25
#define REG_LIMIT_CTL1 0x27

static uint8_t volume_to_reg(uint8_t percent) {
  uint32_t v = ((uint32_t)percent * 255U) / 100U;
  if (v > 0xE6U) {
    return (uint8_t)(v - 0xE7U);
  }
  return (uint8_t)(v + 0x19U);
}

uint8_t codec_volume_reg(void) { return volume_to_reg(CS43L22_VOLUME); }

static I2cStatus wr(uint8_t reg, uint8_t val) {
  return i2c1_byte_write(CS43L22_ADDR, reg, val);
}

I2cStatus codec_init(void) {
  uint8_t vol = volume_to_reg(CS43L22_VOLUME); /* pick e.g. 80 */
  I2cStatus s;

  s = wr(REG_POWER_CTL1, 0x01);
  if (s != I2C_OK)
    return s; /* stay off */
  s = wr(REG_POWER_CTL2, 0x05);
  if (s != I2C_OK)
    return s; /* auto-detect */
  s = wr(REG_CLOCKING_CTL, 0x81);
  if (s != I2C_OK)
    return s; /* auto clock */
  s = wr(REG_INTERFACE_CTL1, 0x04);
  if (s != I2C_OK)
    return s; /* Philips I2S */
  s = wr(REG_MASTER_A_VOL, vol);
  if (s != I2C_OK)
    return s;
  s = wr(REG_MASTER_B_VOL, vol);
  if (s != I2C_OK)
    return s;
  s = wr(REG_PLAYBACK_CTL2, 0x06);
  if (s != I2C_OK)
    return s; /* speaker mono (auto) */
  s = wr(REG_SPEAKER_A_VOL, 0x00);
  if (s != I2C_OK)
    return s;
  s = wr(REG_SPEAKER_B_VOL, 0x00);
  if (s != I2C_OK)
    return s;
  s = wr(REG_ANALOG_ZC_SETT, 0x00);
  if (s != I2C_OK)
    return s; /* no analog soft ramp */
  s = wr(REG_MISC_CTL, 0x04);
  if (s != I2C_OK)
    return s; /* no digital soft ramp */
  s = wr(REG_LIMIT_CTL1, 0x00);
  if (s != I2C_OK)
    return s; /* limiter off */
  s = wr(REG_TONE_CTL, 0x0F);
  if (s != I2C_OK)
    return s;
  s = wr(REG_PCMA_VOL, 0x0A);
  if (s != I2C_OK)
    return s;
  s = wr(REG_PCMB_VOL, 0x0A);
  if (s != I2C_OK)
    return s;

  return I2C_OK;
}
