#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

/* Filter with no platform dependencies: it only sees int16 samples and its
 * own state. The engine owns an instance but nothing outside filter.c needs
 * to know how it works. */
typedef enum {
  FILTER_NONE,
  FILTER_BAND_PASS,
  FILTER_LOW_PASS,
  FILTER_HIGH_PASS,
} FilterType;

typedef struct filter {
  float cutoff;
  float ic1eq;
  float ic2eq;
  float f; /* 2*pi*cutoff/SAMPLE_RATE, clamped to 1 */
  float q; /* damping, 1/Q */
  float resonance;
  FilterType type;
} Filter;

Filter filter_init(void);
void filter_set_cutoff(Filter *filter, float hz);
void filter_set_resonance(Filter *filter, float resonance);
int16_t filter_process(Filter *filter, int16_t sample);

#endif
