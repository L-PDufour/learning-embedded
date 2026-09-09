#include "engine.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define PI_F 3.14159265358979323846f

static Filter filter_init() {
  Filter filter;

  filter.filter_state = 0;
  filter.cutoff = 0;
  filter.filter_p = 0;
  filter.filter = FILTER_LOW_PASS;

  return filter;
}

void filter_set_cutoff(Filter *f, float cutoff) {
  f->cutoff = cutoff;
  f->filter_p = (1 - 2 * cutoff / SAMPLE_RATE) * (1 - 2 * cutoff / SAMPLE_RATE);
}

Engine engine_init() {
  Engine engine;
  engine.bpm = 120;
  engine.num_steps = 0;
  engine.current_step = 0;
  engine.step_sample_count = 0;
  engine.phase_acc = 0;
  engine.current_note = NOTE_REST;
  engine.wave = WAVE_SQUARE;
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
  int i;

  if (maxSteps > 32)
    maxSteps = 32;
  if (maxSteps < 1)
    maxSteps = 1;
  e->num_steps = maxSteps;

  for (i = 0; i < maxSteps; i++) {
    e->steps[i].note = n[i];
    e->steps[i].enabled = n[i] != NOTE_REST;
  }
}

void engine_set_bpm(Engine *e, int bpm) { e->bpm = bpm; }

static int16_t oscillator(WaveType wave, uint32_t phase) {
  float p = phase * (1.0f / 4294967296.0f); /* clock hand: 0..1 = one cycle */
  float sample;

  switch (wave) {
  case WAVE_SQUARE:
    sample = (p < 0.5f) ? AMPLITUDE : -AMPLITUDE;
    break;
  case WAVE_SAW:
    sample = (2.0f * p - 1.0f) * AMPLITUDE;
    break;
  case WAVE_TRIANGLE:
    sample = (p < 0.5f) ? (4.0f * p - 1.0f) * AMPLITUDE
                        : (3.0f - 4.0f * p) * AMPLITUDE;
    break;
  case WAVE_SINE:
    sample = sin(2.0f * PI_F * p) * AMPLITUDE;
    break;
  default:
    sample = 0;
    break;
  }
  return sample;
}

int16_t engine_next_sample(Engine *e) {

  int samples_per_step;
  int16_t sample;
  int freq;
  samples_per_step = (SAMPLE_RATE * 60) / (e->bpm * 4);

  e->step_sample_count++;
  if (e->step_sample_count >= samples_per_step) {
    e->step_sample_count = 0;
    e->current_step++;
    if (e->current_step >= e->num_steps)
      e->current_step = 0;
  }

  if (!e->steps[e->current_step].enabled) {
    e->current_note = NOTE_REST;
    return 0;
  }

  if (e->steps[e->current_step].note != e->current_note) {
    e->current_note = e->steps[e->current_step].note;
    freq = NOTE_FREQUENCIES[e->current_note];
    e->phase_acc = 0;
    e->phase_inc = (uint32_t)(freq * (4294967296.0 / SAMPLE_RATE));
  }

  sample = oscillator(e->wave, e->phase_acc);
  e->phase_acc += e->phase_inc;

  sample = filter_process(&e->filter, sample);
  return sample;
}
