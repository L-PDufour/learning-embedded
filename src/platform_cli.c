#include "engine.h"
#include "platform.h"
#include <stdio.h>

#define BLOCK 256

static Engine e;
static const MusicNote melody[5] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_G4,
                                    NOTE_A4};

void platform_init(void) {
  sample_t buf[BLOCK];
  int i;

  e = engine_init();
  engine_set_bpm(&e, 120);
  engine_set_steps(&e, 5, (MusicNote *)melody);

  while (1) {
    engine_fill_buffer(&e, buf, BLOCK);
    for (i = 0; i < BLOCK; i++) {
      platform_audio_write(buf[i]);
    }
  }
}

void platform_audio_write(sample_t sample) {
  fwrite(&sample, sizeof(int16_t), 1, stdout);
}
