#include "adc.h"
#include "codec.h"
#include "engine.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "platform.h"
#include "ssd1306.h"
#include "systick.h"
#include "ui.h"
#include <stdint.h>

#define BPM 120
#define STEPS 8
#define DEBOUNCE 5
#define POT_READS 8
#define RESONANCE_MAX 0.9f        /* headroom below self-oscillation        */
#define RESONANCE_SLEW 0.02f      /* ~1 ms poll -> full swing in ~45 ms      */
#define RESONANCE_DEADBAND 0.005f /* ignores pot jitter below this delta  */
#define ADC_FULL_SCALE 4095.0f    /* 12-bit ADC */
#define SAMPLES_PER_STEP (SAMPLE_RATE * 60 / (BPM * 4))
#define FRAMES_PER_HALF 256
#define TOTAL_FRAMES (FRAMES_PER_HALF * 2)
#define HALF_HALFWORDS (FRAMES_PER_HALF * 2)
#define DEMCR (*(volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t *)0xE0001004)

static sample_t stereo_buf[TOTAL_FRAMES * 2];
static sample_t mono_buf[FRAMES_PER_HALF];
volatile uint32_t i2s_isr_last_cycles = 0;
volatile uint32_t i2s_isr_max_cycles = 0;
static Engine e;
static const MusicNote melody[STEPS] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4,
                                        NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
static const MusicNote keys[PADS_MAX] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4,
                                         NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};

static int highest_pad(uint8_t mask) {
  int i;
  for (i = PADS_MAX - 1; i >= 0; i--) {
    if (mask & (1U << i)) {
      return i;
    }
  }
  return -1;
}

static uint8_t poll_pads(void) {
  static uint8_t raw_prev = 0;
  static uint8_t stable = 0;
  static uint8_t count = 0;
  static int active = -1;
  uint8_t is_dirty = 0;
  uint8_t raw = pad_read();

  if (raw != raw_prev) {
    raw_prev = raw;
    count = 0;
  } else if (count < DEBOUNCE) {
    count++;
  }

  if (count >= DEBOUNCE && raw != stable) {
    uint8_t newly = (uint8_t)(raw & ~stable);
    stable = raw;

    if (newly != 0) { /* a key went down: it wins (last-pressed) */
      active = highest_pad(newly);
    } else if (active >= 0 && (stable & (1U << active)) == 0) {
      active = highest_pad(stable); /* active released: fall back to any held */
    }

    if (active >= 0) {
      engine_note_on(&e, keys[active]);
    } else {
      engine_note_off(&e);
    }
    is_dirty = 1; /* only a debounced note event redraws the screen */
  }
  return is_dirty;
}

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

/* PA1 pot -> filter resonance. Average POT_READS 12-bit samples, map to
 * 0..RESONANCE_MAX, then deadband + slew so ADC noise cannot flicker the
 * filter. Called from the main loop; a single 32-bit float store is atomic on
 * Cortex-M4, so the DMA ISR reads either the old or the new q, never a torn
 * value. */
static float read_resonance_pot(void) {
  static float last = -1.0f;
  uint32_t sum = 0;
  float target;
  int i;

  for (i = 0; i < POT_READS; i++) {
    sum += adc_read();
  }
  target = ((float)sum / (float)POT_READS) * (RESONANCE_MAX / ADC_FULL_SCALE);

  if (last < 0.0f) {
    last = target; /* first read: seed, do not slew up from a bogus value */
  } else if (target > last + RESONANCE_DEADBAND) {
    last += RESONANCE_SLEW;
  } else if (target < last - RESONANCE_DEADBAND) {
    last -= RESONANCE_SLEW;
  }

  if (last < 0.0f) {
    last = 0.0f;
  }
  if (last > RESONANCE_MAX) {
    last = RESONANCE_MAX;
  }

  return last;
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
  UiStatus ui;

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
  engine_set_steps(&e, STEPS, melody);
  pad_init();
  pa1_adc_init();
  start_conversion();
  engine_set_mode(&e, MONO);
  platform_fill_half(0);
  platform_fill_half(1);
  i2s_dma_config(stereo_buf, TOTAL_FRAMES * 2);
  i2c1_init();

  ssd1306_init();
  ssd1306_clear();
  ui.volume = codec_get_volume();
  ui_render(&e, &ui);
  ssd1306_update();

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
    while (1) {
      engine_set_resonance(&e, read_resonance_pot());
      if (poll_pads()) { /* redraw only when a debounced note event occurred */
        ui.volume = codec_get_volume();
        ui_render(&e, &ui);
        ssd1306_update();
      }
      systick_msec_delay(1);
    }
  }
}
