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

static uint32_t fmt_fixed2(char *out, float n) {
  int hundredths;
  int whole;
  int frac;

  if (n < 0.0f) {
    n = 0.0f;
  }
  if (n > 1.0f) {
    n = 1.0f;
  }

  hundredths = (int)(n * 100.0f + 0.5f);
  if (hundredths > 100) {
    hundredths = 100;
  }

  whole = hundredths / 100;
  frac = hundredths % 100;

  out[0] = (char)('0' + whole);
  out[1] = '.';
  out[2] = (char)('0' + frac / 10);
  out[3] = (char)('0' + frac % 10);
  out[4] = '\0';

  return 4;
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

static void pattern_token(char *out, MusicNote note) {
  char name[4];

  if (note == NOTE_REST) {
    out[0] = '.';
    out[1] = '.';
    return;
  }
  note_name(note, name);
  out[0] = name[0];
  out[1] = name[1];
}

static void pattern_row(char *out, const Engine *engine) {
  uint32_t pos = 0;
  uint32_t step;
  uint32_t current;
  uint32_t count;
  char token[2];

  current = engine_get_current_step(engine);
  count = engine_get_num_steps(engine);

  out[pos++] = 'S';
  out[pos++] = (char)('0' + (current + 1));
  out[pos++] = ' ';

  for (step = 0; step < count; step++) {
    pattern_token(token, engine_get_step(engine, step));
    if (step == current) {
      out[pos++] = '[';
      out[pos++] = token[0];
      out[pos++] = token[1];
      out[pos++] = ']';
    } else {
      out[pos++] = token[0];
      out[pos++] = token[1];
    }
  }
  out[pos] = '\0';
}

static const char *ui_wave_name(WaveType wave) {
  switch (wave) {
  case WAVE_SQUARE:
    return "SQR";
  case WAVE_SAW:
    return "SAW";
  case WAVE_TRIANGLE:
    return "TRI";
  case WAVE_SINE:
    return "SINE";
  default:
    TB_UNREACHABLE();
    return "?";
  }
}

static const char *ui_filter_name(FilterType filter) {
  switch (filter) {
  case FILTER_NONE:
    return "NONE";
  case FILTER_BAND_PASS:
    return "BP";
  case FILTER_LOW_PASS:
    return "LP";
  case FILTER_HIGH_PASS:
    return "HP";
  default:
    TB_UNREACHABLE();
    return "?";
  }
}

static const char *ui_mode_name(Mode mode) {
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
  char bpm[11];
  char vol[11];
  char cutoff[11];
  char resonance[11];
  char pattern[22];
  float hz;

  TB_ASSERT(engine != 0);
  TB_ASSERT(status != 0);

  hz = engine_get_cutoff(engine);
  if (hz < 0.0f) {
    hz = 0.0f;
  }

  fmt_u32(bpm, engine_get_bpm(engine));
  fmt_u32(vol, status->volume);
  fmt_u32(cutoff, (uint32_t)(hz + 0.5f));
  fmt_fixed2(resonance, engine_get_resonance(engine));
  pattern_row(pattern, engine);

  ssd1306_clear(); /* else characters ghost between frames */

  ssd1306_draw_string(0, 0, ui_mode_name(engine_get_mode(engine)));
  ssd1306_draw_string(30, 0, bpm);
  ssd1306_draw_string(54, 0, "BPM");
  ssd1306_draw_string(84, 0, "VOL");
  ssd1306_draw_string(108, 0, vol);
  ssd1306_draw_string(0, 1, "WAVE");
  ssd1306_draw_string(30, 1, ui_wave_name(engine_get_wave(engine)));
  ssd1306_draw_string(60, 1, "FILT");
  ssd1306_draw_string(90, 1, ui_filter_name(engine_get_filter(engine)));
  ssd1306_draw_string(0, 2, "CUT");
  ssd1306_draw_string(30, 2, cutoff);
  ssd1306_draw_string(66, 2, "Hz");
  ssd1306_draw_string(0, 3, "RES");
  ssd1306_draw_string(30, 3, resonance);
  ssd1306_draw_string(0, 5, pattern);
}
