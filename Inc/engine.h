#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

typedef enum {
  WAVE_SQUARE,
  WAVE_SAW,
  WAVE_TRIANGLE,
  WAVE_SINE,
} WaveType;

typedef enum {
  FILTER_LOW_PASS,
} FilterType;

typedef struct filter {
  float filter_state;
  float cutoff;
  float filter_p;
  FilterType filter;
} Filter;

typedef struct engine {
  int num_steps;
  int steps[4];
  int bpm;
  int sample_idx;
  Filter filter;
  WaveType wave;

} Engine;

#define SAMPLE_RATE 22050
#define AMPLITUDE 10000
Engine engine_init(void);
int16_t engine_next_sample(Engine *);
void engine_set_bpm(Engine *, int);

#endif
