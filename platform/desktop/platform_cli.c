#include "engine.h"
#include "platform.h"
#include <stdio.h>

#define BLOCK 256
#define TEST_NOTES 5
#define HOLD_BLOCKS 90
#define GAP_BLOCKS 15
#define SWEEP_BLOCKS 400
#define CUTOFF_MIN_HZ 200.0f
#define CUTOFF_MAX_HZ 6000.0f

static Engine e;
static const MusicNote test_notes[TEST_NOTES] = {NOTE_C4, NOTE_E4, NOTE_G4,
                                                 NOTE_C5, NOTE_G4};

void platform_init(void) {
  sample_t buf[BLOCK];
  int i;
  uint32_t block = 0;
  uint32_t note_i = 0;
  int on = 0;

  e = engine_init();
  engine_set_wave(&e, WAVE_SINE);
  engine_set_mode(&e, MONO);
  engine_set_resonance(&e, 0.8f);

  while (1) {
    uint32_t phase = block % (HOLD_BLOCKS + GAP_BLOCKS);
    uint32_t sweep = block % SWEEP_BLOCKS;
    float t = (sweep < SWEEP_BLOCKS / 2)
                  ? (float)sweep / (float)(SWEEP_BLOCKS / 2)
                  : (float)(SWEEP_BLOCKS - sweep) / (float)(SWEEP_BLOCKS / 2);

    engine_set_cutoff(&e, CUTOFF_MIN_HZ + t * (CUTOFF_MAX_HZ - CUTOFF_MIN_HZ));
    if (!on && phase == 0) {
      engine_note_on(&e, test_notes[note_i]);
      on = 1;
    } else if (on && phase == HOLD_BLOCKS) {
      engine_note_off(&e);
      on = 0;
      note_i = (note_i + 1) % TEST_NOTES;
    }

    engine_fill_buffer(&e, buf, BLOCK);
    for (i = 0; i < BLOCK; i++) {
      platform_audio_write(buf[i]);
    }
    block++;
  }
}

void platform_audio_write(sample_t sample) {
  fwrite(&sample, sizeof(int16_t), 1, stdout);
}
