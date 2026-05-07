#ifndef PLATFORM_H
#define PLATFORM_H

#include "engine.h"

void platform_init(void);
void platform_audio_write(sample_t sample);
void platform_on_button(int button);

#endif
