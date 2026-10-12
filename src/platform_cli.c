#include "engine.h"
#include "platform.h"
#include <stdio.h>

#define BLOCK 256
#define TEST_NOTES 5
#define HOLD_BLOCKS 90 /* ~0.48 s at 48 kHz/BLOCK=256 */
#define GAP_BLOCKS 15  /* ~80 ms release gap            */

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
  /* deliberately no engine_set_steps: proves MONO works with num_steps == 0 */

  while (1) {
    uint32_t phase = block % (HOLD_BLOCKS + GAP_BLOCKS);

    if (!on && phase == 0) { /* key down */
      engine_note_on(&e, test_notes[note_i]);
      on = 1;
    } else if (on && phase == HOLD_BLOCKS) { /* key up */
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
