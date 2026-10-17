#include "ui.h"
#include "ssd1306.h"

/* Pitch-class names. MusicNote is chromatic from NOTE_C0, so the octave is
 * index arithmetic; only the 12 pitch classes need naming. */
static const char *const pitch_class[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                            "F#", "G",  "G#", "A",  "A#", "B"};

static void note_name(MusicNote note, char *out) {
  const char *s;
  int k;
  int i = 0;

  if (note == NOTE_REST) {
    out[0] = '-';
    out[1] = '\0';
    return;
  }
  k = (int)note - (int)NOTE_C0;
  s = pitch_class[k % 12];
  while (s[i] != '\0') {
    out[i] = s[i];
    i++;
  }
  out[i] = (char)('0' + (k / 12));
  out[i + 1] = '\0';
}

void ui_render(Engine *engine) {
  char buf[4]; /* 2 chars name + 1 octave + NUL */

  ssd1306_clear(); /* else characters ghost between frames */
  note_name(engine_get_note(engine), buf);
  ssd1306_draw_string(0, 0, buf);
}
