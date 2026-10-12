#ifndef UI_H
#define UI_H

#include "engine.h"

/* Immediate-mode render: rebuild the whole 128x64 frame from engine state.
 * The platform calls this only when something changed. */
void ui_render(Engine *engine);

#endif
