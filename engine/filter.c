#include "filter.h"
#include "engine.h"
#include "tb.h"

#define PI 3.14159265358979323846f
#define DEFAULT_CUTOFF_HZ 4000.0f
#define DEFAULT_RESONANCE 0.3f
#define DAMP_MAX 2.0f
#define DAMP_MIN 0.05f

Filter filter_init(void) {
  Filter filter;

  filter.cutoff = 0;
  filter.ic1eq = 0;
  filter.ic2eq = 0;
  filter.f = 0;
  filter.type = FILTER_LOW_PASS; /* off until the cutoff control exists */
  filter_set_resonance(&filter, DEFAULT_RESONANCE);
  filter_set_cutoff(&filter, DEFAULT_CUTOFF_HZ);

  return filter;
}

void filter_set_resonance(Filter *filter, float resonance) {
  if (resonance < 0.0f)
    resonance = 0.0f;
  if (resonance > 1.0f)
    resonance = 1.0f;

  filter->resonance = resonance;
  filter->q = DAMP_MAX - resonance * (DAMP_MAX - DAMP_MIN);
}

void filter_set_cutoff(Filter *filter, float cutoff) {
  filter->cutoff = cutoff;
  filter->f = 2.0f * PI * cutoff / SAMPLE_RATE;
  if (filter->f > 1.0f)
    filter->f = 1.0f;
}

int16_t filter_process(Filter *filter, int16_t sample) {
  float x;
  float f;
  float q;
  float band;
  float low;
  float high;
  float out = 0.0f;

  if (filter->type == FILTER_NONE) {
    return sample;
  }

  x = (float)sample;
  f = filter->f;
  q = filter->q;
  band = filter->ic1eq;
  low = filter->ic2eq;

  low = low + f * band;
  high = x - low - q * band;
  band = band + f * high;

  /* State carries between samples; never clamped (resonance lives here). */
  filter->ic1eq = band;
  filter->ic2eq = low;

  switch (filter->type) {
  case FILTER_LOW_PASS:
    out = low;
    break;
  case FILTER_HIGH_PASS:
    out = high;
    break;
  case FILTER_BAND_PASS:
    out = band;
    break;
  default:
    TB_UNREACHABLE();
    break;
  }

  /* Clamp only the output before narrowing to int16. */
  if (out > 32767.0f)
    out = 32767.0f;
  if (out < -32768.0f)
    out = -32768.0f;

  return (int16_t)out;
}
