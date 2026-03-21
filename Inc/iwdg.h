#ifndef IWDG_H__
#define IWDG_H__
#include "stm32f4xx.h"
#include <stdint.h>
void iwdg_init(void);
static uint8_t isIwdg_ready(void);
#endif
