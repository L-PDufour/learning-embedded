#include "codec.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "platform.h"
#include "systick.h"

#define BLOCK 256

static Engine e;
static const MusicNote melody[5] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_G4,
                                    NOTE_A4};
static sample_t buf[BLOCK];
/* Blink `count` times, then pause, forever. Used to report the codec
 * probe result with no terminal or debugger attached. */
static void blink_forever(int count, uint32_t on_ms, uint32_t off_ms,
                          uint32_t pause_ms) {
  int i;

  while (1) {
    for (i = 0; i < count; i++) {
      led_on();
      systick_msec_delay(on_ms);
      led_off();
      systick_msec_delay(off_ms);
    }
    systick_msec_delay(pause_ms);
  }
}

void platform_init(void) {
  uint8_t id = 0;
  uint8_t vol = 0;

  I2cStatus status;
  I2cStatus clock_status;

  systick_init();
  led_init();
  board_init(); /* release codec reset; PD4 left high */
  clock_status = i2s_clock_init();
  i2s3_init();
  i2c1_init();
  e = engine_init();
  engine_set_bpm(&e, 120);
  engine_set_steps(&e, 5, (MusicNote *)melody);
  engine_fill_buffer(&e, buf, BLOCK);
  /* Configure the codec, power it on, then prove the write path. */
  status = codec_init();
  if (status == I2C_OK) {
    status = codec_play();
  }
  if (status == I2C_OK) {
    status = i2c1_byte_read(CS43L22_ADDR, CS43L22_ID_REG, &id);
  }
  if (status == I2C_OK) {
    status = i2c1_byte_read(CS43L22_ADDR, CS43L22_REG_MASTER_A_VOL, &vol);
  }

  if (clock_status != I2C_OK) {
    blink_forever(1, 250, 250, 1000); /* clock timeout: HSE/PLLI2S not ready */
  } else if (status == I2C_ERR_NACK) {
    blink_forever(5, 80, 80, 1200); /* no device answered */
  } else if (status != I2C_OK) {
    blink_forever(10, 60, 60, 1200); /* timeout / bus stuck */
  } else if ((id & CS43L22_ID_MASK) != CS43L22_ID ||
             vol != codec_volume_reg()) {
    blink_forever(2, 250, 250, 1000); /* bus OK, but values wrong */
  } else {
    blink_forever(3, 250, 250, 1000); /* success: 3 slow blinks */
  }
}
