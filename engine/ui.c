#include "ui.h"
#include "engine.h"
#include "ssd1306.h"
#include <stdint.h>

/* Pitch-class names. MusicNote is chromatic from NOTE_C0, so the octave is
 * index arithmetic; only the 12 pitch classes need naming. */
static const char *const pitch_class[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                            "F#", "G",  "G#", "A",  "A#", "B"};

static uint32_t fmt_u32(char *out, uint32_t n) {
  char tmp[10];
  uint32_t len = 0;
  uint32_t i = 0;

  if (n == 0) {
    out[0] = '0';
    out[1] = '\0';
    return 1;
  }

  while (n > 0) {
    tmp[len] = (char)('0' + (n % 10));
    n /= 10;
    len++;
  }

  for (i = 0; i < len; i++) {
    out[i] = tmp[len - 1 - i];
  }
  out[len] = '\0';

  return len;
}

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

const char *ui_mode_name(Mode mode) {
  switch (mode) {
  case LOOP:
    return "PLAY";
  case STOP:
    return "STOP";
  case EDITING:
    return "EDIT";
  case MONO:
    return "MONO";
  default:
    TB_UNREACHABLE();
    return "?";
  }
}

void ui_render(Engine *engine, const UiStatus *status) {
  char buf[4]; /* 2 chars name + 1 octave + NUL */
  const char *mode;

  char bpm[11];
  char vol[11];

  TB_ASSERT(engine != 0);
  TB_ASSERT(status != 0);

  mode = ui_mode_name(engine_get_mode(engine));

  fmt_u32(bpm, engine_get_bpm(engine));
  fmt_u32(vol, status->volume);

  ssd1306_clear(); /* else characters ghost between frames */

  note_name(engine_get_note(engine), buf);
  ssd1306_draw_string(0, 0, mode);
  ssd1306_draw_string(30, 0, bpm);
  ssd1306_draw_string(54, 0, "BPM");
  ssd1306_draw_string(84, 0, "VOL");
  ssd1306_draw_string(108, 0, vol);

  ssd1306_draw_string(0, 2, "Tempo:");
  ssd1306_draw_string(0, 2, "Filtre:");
  ssd1306_draw_string(0, 2, "Fréquence:");
}
