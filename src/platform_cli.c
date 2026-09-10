#include "engine.h"
#include "platform.h"
#include <stdio.h>

static Engine e;
static const MusicNote melody[5] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_G4,
                                    NOTE_A4};

void platform_init(void) {
  e = engine_init();
  engine_set_bpm(&e, 120);
  engine_set_steps(&e, 5, (MusicNote *)melody);

  while (1) {
    sample_t sample = engine_next_sample(&e);
    platform_audio_write(sample);
  }
}

void platform_audio_write(sample_t sample) {
  fwrite(&sample, sizeof(int16_t), 1, stdout);
}
