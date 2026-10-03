#include "codec.h"
#include "engine.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "platform.h"
#include "systick.h"

#define BPM 120
#define STEPS 8
#define SAMPLES_PER_STEP (SAMPLE_RATE * 60 / (BPM * 4))
#define FRAMES_PER_HALF 256
#define TOTAL_FRAMES (FRAMES_PER_HALF * 2)
#define HALF_HALFWORDS (FRAMES_PER_HALF * 2)

static Engine e;
static const MusicNote song[STEPS] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_A4,
                                      NOTE_G4, NOTE_E4, NOTE_D4, NOTE_G4};
static sample_t stereo_buf[TOTAL_FRAMES * 2];
static sample_t mono_buf[FRAMES_PER_HALF];

#define DEMCR (*(volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t *)0xE0001004)
volatile uint32_t i2s_isr_last_cycles = 0;
volatile uint32_t i2s_isr_max_cycles = 0;

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

void platform_fill_half(int half) {
  int i;
  int base = half * HALF_HALFWORDS;
  uint32_t t0 = DWT_CYCCNT;
  uint32_t dt;

  engine_fill_buffer(&e, mono_buf, FRAMES_PER_HALF);
  for (i = 0; i < FRAMES_PER_HALF; i++) {
    stereo_buf[base + 2 * i] = mono_buf[i];
    stereo_buf[base + 2 * i + 1] = mono_buf[i];
  }

  dt = DWT_CYCCNT - t0;
  i2s_isr_last_cycles = dt;
  if (dt > i2s_isr_max_cycles) {
    i2s_isr_max_cycles = dt;
  }
}

void platform_init(void) {
  uint8_t id = 0;
  uint8_t vol = 0;

  I2cStatus status;
  I2cStatus clock_status;

  systick_init();
  DEMCR |= (1U << 24); /* TRCENA: enable the debug cycle counter */
  DWT_CYCCNT = 0;
  DWT_CTRL |= 1U; /* CYCCNTENA */
  led_init();
  board_init(); /* release codec reset; PD4 left high */
  clock_status = i2s_clock_init();
  i2s3_init();
  e = engine_init();
  engine_set_wave(&e, WAVE_SINE);
  engine_set_bpm(&e, BPM);
  engine_set_steps(&e, STEPS, (MusicNote *)song);
  platform_fill_half(0);
  platform_fill_half(1);
  i2s_dma_config(stereo_buf, TOTAL_FRAMES * 2);
  i2c1_init();
  /* Configure the codec, power it on, then prove the write path. */
  status = codec_init();
  if (status == I2C_OK) {
    status = codec_play();
  }
  i2s_isr_max_cycles = 0; /* measure only the live ISR, not the cold pre-fill */
  i2s_dma_start();
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
