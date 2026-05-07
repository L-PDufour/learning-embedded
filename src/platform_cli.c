#include "engine.h"
#include "platform.h"
#include <stdio.h>

static Engine e;
void platform_init(void) {
  e = engine_init();
  engine_set_bpm(&e, 120);
  engine_set_steps(
      &e, 32, (MusicNote[]){NOTE_E5,   NOTE_DS5, NOTE_E5,  NOTE_DS5, NOTE_E5,
                            NOTE_B4,   NOTE_D5,  NOTE_C5,  NOTE_A4,  NOTE_REST,
                            NOTE_C4,   NOTE_E4,  NOTE_A4,  NOTE_B4,  NOTE_REST,
                            NOTE_E4,   NOTE_GS4, NOTE_B4,  NOTE_C5,  NOTE_REST,
                            NOTE_E4,   NOTE_E5,  NOTE_DS5, NOTE_E5,  NOTE_DS5,
                            NOTE_E5,   NOTE_B4,  NOTE_D5,  NOTE_C5,  NOTE_A4,
                            NOTE_REST, NOTE_REST});
  while (1) {
    sample_t sample = engine_next_sample(&e);
    platform_audio_write(sample);
  }
}

void platform_audio_write(sample_t sample) {
  fwrite(&sample, sizeof(int16_t), 1, stdout);
}
