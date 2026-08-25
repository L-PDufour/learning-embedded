#include "engine.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static Filter filter_init() {
  Filter filter;

  filter.filter_state = 0;
  filter.cutoff = 0;
  filter.filter_p = 0;
  filter.filter = FILTER_LOW_PASS;

  return filter;
};

void filter_set_cutoff(Filter *f, float cutoff) {
  f->cutoff = cutoff;
  f->filter_p = (1 - 2 * cutoff / SAMPLE_RATE) * (1 - 2 * cutoff / SAMPLE_RATE);
}

Engine engine_init() {
  Engine engine;
  engine.bpm = 120;
  engine.num_steps = 0;
  engine.sample_idx = 0;
  engine.wave = WAVE_SINE;
  engine.filter = filter_init();
  filter_set_cutoff(&engine.filter, 4000);
  return engine;
}

static int16_t filter_process(Filter *f, int16_t sample) {

  switch (f->filter) {
  case FILTER_LOW_PASS:
    f->filter_state =
        (1 - f->filter_p) * sample + f->filter_p * f->filter_state;

    break;
  default:
    break;
  }
  return (int16_t)f->filter_state;
}

void engine_set_steps(Engine *e, int maxSteps, MusicNote *n) {
  if (maxSteps > 32)
    maxSteps = 32;
  if (maxSteps < 1)
    maxSteps = 1;
  e->num_steps = maxSteps;
  for (int i = 0; i < maxSteps; i++) {
    e->steps[i] = NOTE_FREQUENCIES[n[i]];
  }
}

void engine_set_bpm(Engine *e, int bpm) { e->bpm = bpm; }

static int16_t oscillator(WaveType wave, uint32_t phase) {
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
    sample = (sinf(2 * M_PI * current_cycle / period) * AMPLITUDE);
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

sample_t engine_next_sample(Engine *e) {

  int samples_per_step = (SAMPLE_RATE * 60) / e->bpm;

  e->step_sample_count++;
  if (e->step_sample_count > samples_per_step) {
    e->step_sample_count = 0;
    e->current_step++;
  }

  int16_t sample = oscillator(e->wave, );
  sample = filter_process(&e->filter, sample);
  return sample;
}
