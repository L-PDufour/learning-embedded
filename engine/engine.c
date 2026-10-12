#include "engine.h"
#include "sine_table.h"
#include "tb.h"

#include <stddef.h>
#include <stdint.h>

#define MAX_RAMP 256
#define MODE_COUNT 4

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
  engine.phase_inc = 0;
  engine.current_note = NOTE_REST;
  engine.mono_note = NOTE_REST;
  engine.current_mode = LOOP;
  engine.wave = WAVE_SQUARE;
  engine.filter = filter_init();
  engine.gate = 0;
  engine.ramp = 0;
  filter_set_cutoff(&engine.filter, DEFAULT_CUTOFF_HZ);
  return engine;
}

static int16_t filter_process(Filter *filter, int16_t sample) {
  float out = sample;

  switch (filter->filter) {
  case FILTER_LOW_PASS:
    filter->filter_state = (1 - filter->filter_p) * sample +
                           filter->filter_p * filter->filter_state;
    out = filter->filter_state;
    break;
  case FILTER_HIGH_PASS:
    filter->filter_state = (1 - filter->filter_p) * sample +
                           filter->filter_p * filter->filter_state;
    out = sample - filter->filter_state;
    break;
  case FILTER_NONE:
  default:
    out = sample;
    break;
  }
  return (int16_t)out;
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

static void calculate_step(Engine *engine) {
  uint32_t samples_per_step = 0;
  samples_per_step =
      (SAMPLE_RATE * SECONDS_PER_MINUTE) / (engine->bpm * STEPS_PER_BEAT);
  engine->step_sample_count++;

  if (engine->step_sample_count >= samples_per_step) {
    engine->step_sample_count = 0;
    engine->current_step++;
    if (engine->current_step >= engine->num_steps)
      engine->current_step = 0;
  }
}

static void apply_voice(Engine *engine, MusicNote note, float gate) {

  float ramp_step = 1.0f / MAX_RAMP;

  if (engine->ramp < gate) {
    engine->ramp += ramp_step;
    if (engine->ramp > gate)
      engine->ramp = gate;
  } else if (engine->ramp > gate) {
    engine->ramp -= ramp_step;
    if (engine->ramp < gate)
      engine->ramp = gate;
  }
  if (note != NOTE_REST && note != engine->current_note) {
    engine->current_note = note;
    engine->phase_inc =
        (uint32_t)(NOTE_FREQUENCIES[note] * (PHASE_RANGE / SAMPLE_RATE));
  }
}

static sample_t render_sample(Engine *engine) {
  MusicNote note = 0;
  float gate = 0;
  sample_t out;

  switch (engine->current_mode) {
  case LOOP:
    TB_ASSERT(engine->num_steps >= 1);
    TB_ASSERT(engine->current_step < engine->num_steps);
    calculate_step(engine);
    note = engine->steps[engine->current_step].note;
    gate = engine->steps[engine->current_step].enabled ? 1.0f : 0.0f;
    break;
  case EDITING:
    TB_ASSERT(engine->num_steps >= 1);
    TB_ASSERT(engine->current_step < engine->num_steps);
    note = engine->steps[engine->current_step].note;
    gate = engine->steps[engine->current_step].enabled ? 1.0f : 0.0f;
    break;
  case MONO:
    note = engine->mono_note;
    gate = engine->gate;
    break;
  case STOP:
    gate = 0;
    break;
  }

  apply_voice(engine, note, gate);

  out = filter_process(
      &engine->filter,
      (sample_t)(oscillator(engine->wave, engine->phase_acc) * engine->ramp));
  engine->phase_acc += engine->phase_inc;

  return out;
}

sample_t engine_next_sample(Engine *engine) { return render_sample(engine); }

void engine_note_on(Engine *engine, MusicNote new_note) {
  TB_ASSERT(engine != 0);
  TB_ASSERT(new_note < NOTE_COUNT);
  engine->mono_note = new_note;
  engine->gate = 1.0f;
}

void engine_note_off(Engine *engine) {
  TB_ASSERT(engine != 0);
  engine->gate = 0.0f;
}

MusicNote engine_get_note(Engine *engine) {
  TB_ASSERT(engine != 0);
  if (engine->current_mode == LOOP) {
    return engine->steps[engine->current_step].note;
  } else if (engine->current_mode == MONO) {
    return engine->mono_note;
  }
  return NOTE_REST;
}

void engine_set_mode(Engine *engine, Mode new_mode) {
  TB_ASSERT(engine != 0);
  TB_ASSERT(new_mode < MODE_COUNT);
  engine->current_mode = new_mode;
}

void engine_set_step_note(Engine *engine, uint32_t step, MusicNote note) {
  TB_ASSERT(engine != 0);
  TB_ASSERT(step < engine->num_steps);
  TB_ASSERT(note < NOTE_COUNT);

  engine->steps[step].note = note;
  engine->steps[step].enabled = (note != NOTE_REST);

  /* Postcondition: the write took. */
  TB_ASSERT(engine->steps[step].note == note);
}

void engine_set_cutoff(Engine *engine, float hz) {
  TB_ASSERT(engine != 0);
  filter_set_cutoff(&engine->filter, hz);
}

void engine_fill_buffer(Engine *engine, sample_t *buf, uint32_t n) {
  uint32_t i;

  TB_ASSERT(engine != 0);
  TB_ASSERT(buf != 0);

  for (i = 0; i < n; i++) {
    buf[i] = render_sample(engine);
  }
}
