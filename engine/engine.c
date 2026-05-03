#include <engine.h>
#include <math.h>
#include <stdint.h>

Engine engine_init() {
  Engine engine;
  engine.bpm = 120;
  engine.num_steps = 4;
  engine.sample_idx = 0;
  engine.wave = WAVE_SAW;
  int steps[4] = {
      392,
      294,
      262,
      330,
  };
  for (int i = 0; i < engine.num_steps; i++) {
    engine.steps[i] = steps[i];
  }
  return engine;
}

void engine_set_bpm(Engine *e, int bpm) { e->bpm = bpm; }

int16_t oscillator(WaveType wave, int current_cycle, int period) {
  int16_t sample;
  switch (wave) {
  case WAVE_SQUARE:
    if (current_cycle < period / 2) {
      sample = AMPLITUDE;
    } else {
      sample = -AMPLITUDE;
    }
    break;
  case WAVE_SINE:
    sample = (AMPLITUDE * sinf(2 * M_PI * current_cycle / period));
    break;
  case WAVE_TRIANGLE:
    if (current_cycle < period / 2) {
      sample = (current_cycle * 2 * 2 * AMPLITUDE / period) - AMPLITUDE;
    } else {
      sample = AMPLITUDE -
               ((current_cycle - period / 2) * 2 * 2 * AMPLITUDE / period);
    }
    break;
  case WAVE_SAW:
    sample = (current_cycle * 2 * AMPLITUDE / period) - AMPLITUDE;
    break;
  default:
    sample = 0;
    break;
  }
  return sample;
}

int16_t engine_next_sample(Engine *e) {

  int samples_per_step = (SAMPLE_RATE * 60) / e->bpm;
  int current_step = (e->sample_idx / samples_per_step) % 4;
  int period = SAMPLE_RATE / e->steps[current_step];
  int current_cycle = e->sample_idx % period;
  e->sample_idx++;
  int16_t sample = oscillator(e->wave, current_cycle, period);
  return sample;
}
