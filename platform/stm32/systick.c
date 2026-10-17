#include "systick.h"
#define CTRL_ENABLE (1U << 0)
#define CTRL_TICKINT (1U << 1)
#define CTRL_CLCKSRC (1U << 2)
#define CTRL_COUNTFLAG (1U << 16)
/*By default, the frequency of the MCU is 16Mhz*/
#define ONE_MSEC_LOAD 16000

void systick_init(void) {
  SysTick->LOAD = ONE_MSEC_LOAD - 1;

  SysTick->VAL = 0;

  SysTick->CTRL = CTRL_CLCKSRC | CTRL_ENABLE | CTRL_TICKINT;
}

static volatile uint32_t systick_ms = 0;

void SysTick_Handler(void) { systick_ms++; }

uint32_t systick_millis(void) { return systick_ms; }

void systick_msec_delay(uint32_t delay) {
  uint32_t start = systick_millis();
  while ((systick_millis() - start) < delay) {
  }
}
