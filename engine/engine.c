#include "engine.h"
#include "sine_table.h"
#include "tb.h"

#include <stddef.h>
#include <stdint.h>

#define MAX_RAMP 256
#define DEFAULT_BPM 120U
#define DEFAULT_CUTOFF_HZ 4000.0f
#define BPM_MIN 40U
#define BPM_MAX 240U
#define SECONDS_PER_MINUTE 60U
#define STEPS_PER_BEAT 4U
#define PHASE_RANGE 4294967296.0f

static const float wave_gain[WAVE_COUNT] = {
    0.40f, /* WAVE_SQUARE:   many odd harmonics -> reads loudest */
    0.50f, /* WAVE_SAW:      full harmonic stack        */
    0.70f, /* WAVE_TRIANGLE: fewer harmonics            */
    1.25f  /* WAVE_SINE:     single tone, reference     */
};

static Filter filter_init(void) {
  Filter filter;

  filter.filter_state = 0;
  filter.cutoff = 0;
  filter.filter_p = 0;
  filter.filter = FILTER_LOW_PASS;

  return filter;
}

static void filter_set_cutoff(Filter *filter, float cutoff) {
  filter->cutoff = cutoff;
  filter->filter_p =
      (1 - 2 * cutoff / SAMPLE_RATE) * (1 - 2 * cutoff / SAMPLE_RATE);
}

Engine engine_init(void) {
  Engine engine;
  engine.bpm = DEFAULT_BPM;
  engine.num_steps = 0;
  engine.current_step = 0;
  engine.step_sample_count = 0;
  engine.phase_acc = 0;
  engine.current_note = NOTE_REST;
  engine.wave = WAVE_SQUARE;
  engine.filter = filter_init();
  engine.ramp = 0;
  filter_set_cutoff(&engine.filter, DEFAULT_CUTOFF_HZ);
  return engine;
}

static int16_t filter_process(Filter *filter, int16_t sample) {

  switch (filter->filter) {
  case FILTER_LOW_PASS:
    filter->filter_state = (1 - filter->filter_p) * sample +
                           filter->filter_p * filter->filter_state;

    break;
  default:
    break;
  }
  return (int16_t)filter->filter_state;
}

void engine_set_steps(Engine *engine, uint32_t max_steps,
                      const MusicNote *notes) {
  uint32_t i;
  TB_ASSERT(engine != 0);
  TB_ASSERT(max_steps >= 1);
  TB_ASSERT(max_steps <= ENGINE_STEPS_MAX);
  TB_ASSERT(notes != 0);

  engine->num_steps = max_steps;

  for (i = 0; i < max_steps; i++) {
    engine->steps[i].note = notes[i];
    engine->steps[i].enabled = notes[i] != NOTE_REST;
  }
}

MusicNote engine_get_step(const Engine *engine, uint32_t step) {
  MusicNote note;

  TB_ASSERT(engine != 0);
  TB_ASSERT(step < engine->num_steps);

  note = engine->steps[step].note;

  TB_ASSERT(note < NOTE_COUNT); /* positive-space check on the way out */

  return note;
}

void engine_set_bpm(Engine *engine, uint32_t bpm) {
  TB_ASSERT(engine != 0);
  TB_ASSERT(bpm >= BPM_MIN);
  TB_ASSERT(bpm <= BPM_MAX);

  engine->bpm = bpm;
}
void engine_set_wave(Engine *engine, WaveType wave) {
  TB_ASSERT(engine != 0);
  TB_ASSERT(wave < WAVE_COUNT);
  engine->wave = wave;
}

static sample_t oscillator(WaveType wave, uint32_t phase) {
  TB_ASSERT(wave < WAVE_COUNT);
  float p = phase * (1.0f / PHASE_RANGE); /* clock hand: 0..1 = one cycle */
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
    sample = sine_table[phase >> (32 - SINE_TABLE_BITS)];
    break;
  default:
    TB_UNREACHABLE();
    break;
  }
  return (sample_t)(sample * wave_gain[wave]);
}

static sample_t next_sample(Engine *engine) {
  TB_ASSERT(engine->num_steps >= 1);
  TB_ASSERT(engine->current_step < engine->num_steps);
  uint32_t samples_per_step = 0;
  int16_t sample = 0;
  uint16_t freq = 0;
  float target = 0;
  float step = 0;

  samples_per_step =
      (SAMPLE_RATE * SECONDS_PER_MINUTE) / (engine->bpm * STEPS_PER_BEAT);
  engine->step_sample_count++;
  if (engine->step_sample_count >= samples_per_step) {
    engine->step_sample_count = 0;
    engine->current_step++;
    if (engine->current_step >= engine->num_steps)
      engine->current_step = 0;
  }

  target = engine->steps[engine->current_step].enabled ? 1.0f : 0.0f;
  step = 1.0f / MAX_RAMP;
  if (engine->ramp < target) {
    engine->ramp += step;
    if (engine->ramp > target)
      engine->ramp = target;
  } else if (engine->ramp > target) {
    engine->ramp -= step;
    if (engine->ramp < target)
      engine->ramp = target;
  }

  if (engine->steps[engine->current_step].enabled &&
      engine->steps[engine->current_step].note != engine->current_note) {
    engine->current_note = engine->steps[engine->current_step].note;
    freq = NOTE_FREQUENCIES[engine->current_note];
    engine->phase_inc = (uint32_t)(freq * (PHASE_RANGE / SAMPLE_RATE));
  }

  sample = oscillator(engine->wave, engine->phase_acc);
  engine->phase_acc += engine->phase_inc;

  sample = filter_process(&engine->filter, (sample_t)(sample * engine->ramp));
  return sample;
}

sample_t engine_next_sample(Engine *engine) { return next_sample(engine); }

void engine_set_step_note(Engine *engine, uint32_t step, MusicNote note) {
  TB_ASSERT(engine != 0);
  TB_ASSERT(step < engine->num_steps);
  TB_ASSERT(note < NOTE_COUNT);

  engine->steps[step].note = note;
  engine->steps[step].enabled = (note != NOTE_REST);

  /* Postcondition: the write took. */
  TB_ASSERT(engine->steps[step].note == note);
}

void engine_fill_buffer(Engine *engine, sample_t *buf, uint32_t n) {
  uint32_t i;

  TB_ASSERT(engine != 0);
  TB_ASSERT(buf != 0);

  for (i = 0; i < n; i++) {
    buf[i] = next_sample(engine);
  }
}
