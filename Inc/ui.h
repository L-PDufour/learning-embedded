#ifndef UI_H
#define UI_H

#include "engine.h"

typedef struct {
  uint8_t volume;
} UiStatus;

void ui_render(Engine *engine, const UiStatus *status);

#endif
