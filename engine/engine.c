#include <engine.h>

Engine engine_init() {
  Engine engine;
  engine.bpm = 120;
  engine.num_steps = 4;
  engine.sample_idx = 0;
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

int16_t engine_next_sample(Engine *e) {
  int16_t sample;
  int samples_per_step = (SAMPLE_RATE * 60) / e->bpm;
  int current_step = (e->sample_idx / samples_per_step) % 4;
  int period = SAMPLE_RATE / e->steps[current_step];
  int current_cycle = e->sample_idx % period;
  e->sample_idx++;
  if (current_cycle < period / 2) {
    sample = AMPLITUDE;
  } else {
    sample = -AMPLITUDE;
  }
  return sample;
}
