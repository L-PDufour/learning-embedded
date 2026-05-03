#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
typedef struct engine {
  int num_steps;
  int steps[4];
  int bpm;
  int sample_idx;
} Engine;

#define SAMPLE_RATE 22050
#define AMPLITUDE 10000
Engine engine_init(void);
int16_t engine_next_sample(Engine *);
void engine_set_bpm(Engine *, int);

#endif
